#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <memory>
#include <map>
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>

#define LINGERSTD	600

typedef unsigned int uint_t;

class CThreadPool
{
public:
    CThreadPool()
    {
        nidle = 0;
        minthreads = 0;
        maxthreads = 0;
        curthreads = 0;
        thrcounter = 0;
        stop = false;
    }

    void open(int minthreads, int maxthreads, int linger = LINGERSTD)
    {
        this->minthreads = minthreads;
        this->maxthreads = maxthreads;
        this->linger = linger;

        for (int i = 0; i < minthreads; ++i) {
            addthread();
        }

        // ................................
        bInitialized = true;
    }

    void close()
    {
        if (!bInitialized) {
            return;
        }

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }

        condition.notify_all();

        for (auto it = workers.begin(); it != workers.end(); ++it) {
            it->second.join();
        }

        {
            std::unique_lock<std::mutex> lock(this->map_mutex);
            workers.clear();
        }

        // ................................
        bInitialized = false;
    }

    bool Initialized()
    {
        return bInitialized;
    }

    int get_pool_linger()
    {
        return this->linger;
    }
    void put_pool_linger(int pl_lng)
    {
        this->linger = pl_lng;
    }

    template<class F, class... Args>
    int queue(F&& f, Args&&... args)
    {
        delthread();

        if (nidle == 0 && curthreads < maxthreads)
            addthread();

        auto task = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop)
                return 1;
            tasks.emplace([task = std::move(task)]() { task(); });
        }
        condition.notify_one();

        return 0;
    }

    template<class F, class... Args>
    auto queuewres(F&& f, Args&&... args)
    {
        delthread();

        if (nidle == 0 && curthreads < maxthreads)
            addthread();

        using return_type = typename std::result_of<F(Args...)>::type;

        std::packaged_task<return_type()> task(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        auto res = task->get_future();

        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // don't allow enqueueing after stopping the pool
            if (stop)
                return NULL;
            //throw std::runtime_error("enqueue on stopped ThreadPool");

            m_tasks.emplace([task = std::move(task)](){ task(); });
        }
        condition.notify_one();

        return res;
    }

    ~CThreadPool()
    {
        if (bInitialized) {
            close();
        }
    }

private:
    void addthread()
    {
        ++nidle;
        ++curthreads;

        int key = ++thrcounter;

        std::unique_lock<std::mutex> lock2(this->map_mutex);

        workers.emplace(key, std::thread(
            [this, key] {

            for (;;) {
                std::function<void()> task;

                {
                    std::unique_lock<std::mutex> lock(this->queue_mutex);
                    bool bres = this->condition.wait_for(lock, std::chrono::seconds(linger), [this] { return this->stop || !this->tasks.empty(); });
                    if (this->stop || (!bres && curthreads > minthreads && nidle)) {
                        if (!this->stop) {
                            --nidle;
                            --curthreads;

                            {
                                std::unique_lock<std::mutex> lock(this->mustdel_mutex);
                                this->mustdel.emplace_back(key);
                            }
                        }
                        return;
                    }
                    if (!bres) {
                        continue;
                    }

                    task = std::move(this->tasks.front());
                    this->tasks.pop();
                }

                --nidle;
                task();
                ++nidle;
            }
        }));
    }

    void delthread()
    {
        std::unique_lock<std::mutex> lock(this->mustdel_mutex);
        std::unique_lock<std::mutex> lock2(this->map_mutex);

        if (mustdel.empty())
            return;

        for (auto it = mustdel.begin(); it != mustdel.end(); ++it) {
            auto it2 = workers.find(*it);
            if (it2 != workers.end()) {
                it2->second.join();
                workers.erase(*it);
            }
        }
        mustdel.clear();
    }

    // .......... data ..........
    std::atomic<int> nidle;
    std::atomic<int> minthreads;
    std::atomic<int> curthreads;
    std::atomic<int> linger;

    std::atomic<int> thrcounter;

    int maxthreads;

    std::map< int, std::thread > workers;
    std::vector< int > mustdel;
    std::queue< std::function<void()> > tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    std::atomic<bool> stop;

    std::mutex map_mutex;
    std::mutex mustdel_mutex;

    bool bInitialized = false;
};

#endif