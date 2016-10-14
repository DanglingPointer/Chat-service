#pragma once
#include <memory>
#include <array>
#include <queue>
#include <future>
#include <algorithm>
#include <atomic>
#include <functional>
#include <mutex>
#include <condition_variable>

template<std::size_t TCOUNT> class ThreadPool
{
public:
    typedef ThreadPool<TCOUNT> My_t;
    explicit ThreadPool(int linger) :m_linger(linger)
    {
        for (std::atomic_bool& b : m_statusFree)
            b = true;
    }
    ~ThreadPool()
    {
        m_cv.notify_all();
        for (auto& pthread : m_workers) {
            if (pthread)
                pthread->join();
        }
    }
    template<class TFunc, class... TArgs>
    void AddTask(TFunc&& f, TArgs&&... args)
    {
        TryAddWorker();

        using Res_t = std::result_of_t<TFunc(TArgs...)>;
        auto btask = std::bind(std::forward<TFunc>(f), std::forward<TArgs>(args)...);

        {
            std::lock_guard<std::mutex> lk(m_queueLock);
            m_tasks.emplace([btask = std::move(btask)](){ btask(); });
        }
        m_cv.notify_one();
    }
    template<class TFunc, class... TArgs>
    auto AddFunc(TFunc&& f, TArgs&&... args)
    {
        TryAddWorker();

        using Res_t = std::result_of_t<TFunc(TArgs...)>;
        auto task = std::make_shared<std::packaged_task<Res_t()>>(std::bind(std::forward<TFunc>(f), std::forward<TArgs>(args)...));

        {
            std::lock_guard<std::mutex> lk(m_queueLock);
            m_tasks.emplace([task](){ (*task)(); });
        }
        m_cv.notify_one();
        return task->get_future();
    }

private:
    void TryAddWorker()
    {
        for (int i = 0; i < TCOUNT; ++i) {
            if (m_statusFree[i]) {
                if (m_workers[i]) m_workers[i]->join();
                m_workers[i] = std::make_unique<std::thread>(&My_t::DoWork, this, i);
                return;
            }
        }
    }
    void DoWork(int workerId)
    {
        m_statusFree[workerId] = false;
        std::function<void()> task;
        for (;;) {
            {
                std::unique_lock<std::mutex> lk(m_queueLock);

                if (m_cv.wait_for(lk, std::chrono::seconds(m_linger), [this] { return !m_tasks.empty(); }) ) {
                    task = std::move(m_tasks.front());
                    m_tasks.pop();
                }
                else {
                    m_statusFree[workerId] = true;
                    return;
                }

            }
            task();
        }
    }
    std::array<std::unique_ptr<std::thread>, TCOUNT> m_workers;
    std::array<std::atomic_bool, TCOUNT> m_statusFree;

    std::queue<std::function<void()>> m_tasks;

    std::mutex m_queueLock;
    std::condition_variable m_cv;
    std::atomic<int> m_linger;
};