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
    explicit ThreadPool(int linger) :m_linger(linger)
    {
        for (std::atomic_bool& b : m_free)
            b = true;
    }
    ~ThreadPool()
    {
        for (auto& pthread : m_workers) {
            pthread->join();
        }
    }
    template<class TFunc, class... TArgs>
    void AddTask(TFunc&& f, TArgs&&... args)
    {
        auto btask = std::bind(std::forward<TFunc>(f), std::forward<TArgs>(args)...);
        {
            std::lock_guard<std::mutex> lk(m_queueLock);
            m_tasks.emplace([btask = std::move(btask)](){ btask(); });

            TryAddWorker();
        }
        m_cv.notify_one();
    }
    template<class TFunc, class... TArgs>
    auto AddFunc(TFunc&& f, TArgs&&... args)
    {
        using Res_t = std::result_of_t<TFunc(TArgs...)>;
        std::packaged_task<Res_t()> task(std::bind(std::forward<TFunc>(f), std::forward<TArgs>(args)...));
        auto fut = task.get_future();
        {
            std::lock_guard<std::mutex> lk(m_queueLock);
            m_tasks.emplace([task = std::move(task)](){ task(); });

            TryAddWorker();
        }
        m_cv.notify_one();
        return fut;
    }

private:
    void TryAddWorker()
    {
        for (int i = 0; i < TCOUNT; ++i) {
            if (m_free[i]) {
                m_workers[i]->join();
                m_workers[i] = std::make_unique<std::thread>(DoWork, i);
                return;
            }
        }
    }
    void DoWork(int workerId)
    {
        m_free[workerId] = false;
        std::function<void()> task;
        for (;;) {
            {
                std::unique_lock<std::mutex> lk(m_queueLock);
                this->m_cv.wait_for(lk, std::chrono::seconds(m_linger));
                if (m_tasks.empty()) {
                    m_free[workerId] = true;
                    return;
                }

                task = std::move(m_tasks.front());
                this->m_tasks.pop();
            }
            task();
        }
    }
    std::array<std::unique_ptr<std::thread>, TCOUNT> m_workers;
    std::queue<std::function<void()>> m_tasks;
    std::array<std::atomic_bool, TCOUNT> m_free;
    std::mutex m_queueLock;
    std::condition_variable m_cv;
    std::atomic<int> m_linger;
};