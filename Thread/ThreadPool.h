#ifndef COMPILERFRONT_THREADPOOL_H
#define COMPILERFRONT_THREADPOOL_H

#include <deque>
#include <vector>
#include <functional>

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace xthread {

    typedef std::function<void()> task_t;

    template <typename T, typename F>
    struct IsFunctor
    {
        typedef char small;
        struct big { char d[2]; };

        static T GetDummyT();
        static big  test(...);
        static small test(F&&);

        enum { value = sizeof(test(GetDummyT())) == sizeof(small) };
    };

    class TaskQueue
    {
    public:

        void Clear()
        {
            std::unique_lock<std::mutex> lock(m_);
            tasks_.clear();
        }

        size_t GetTaskNum() const
        {
            std::unique_lock<std::mutex> lock(m_);
            return tasks_.size();
        }

        bool Pop(task_t& f)
        {
            std::unique_lock<std::mutex> lock(m_);
            while (!exit_ && tasks_.empty()) cv_.wait(lock);

            if (exit_) return false;

            f = std::move(tasks_.front());
            tasks_.pop_front();

            return true;
        }

        bool TryPop(task_t &f)
        {
            std::unique_lock<std::mutex> lock{m_, std::try_to_lock};
            if (!lock || tasks_.empty()) return false;

            f = std::move(tasks_.front());
            tasks_.pop_front();
            return true;
        }

        template<typename T>
        bool Push(T&& f)
        {
            {
                std::unique_lock<std::mutex> lock{m_};
                tasks_.emplace_back(std::forward<T>(f));
            }

            cv_.notify_one();
            return true;
        }

        template<typename T>
        bool PushExitTask(T&& f, bool linger)
        {
            {
                std::unique_lock<std::mutex> lock{m_};
                if (!linger) tasks_.clear();

                exit_ = true;
                tasks_.emplace_back(std::move(f));
            }

            cv_.notify_one();
            return true;
        }

        template<typename T>
        bool TryPush(T&& f)
        {
            {
                std::unique_lock<std::mutex> lock{m_, std::try_to_lock};
                if (!lock) return false;

                tasks_.emplace_back(std::forward<T>(f));
            }

            cv_.notify_one();
            return true;
        }

    private:
        bool exit_ = false;
        mutable std::mutex m_;
        std::condition_variable cv_;

        std::deque<task_t> tasks_;
    };

    class ThreadPool
    {
    public:
        ThreadPool()
            : ThreadPool(std::thread::hardware_concurrency())
        {
        }

        explicit ThreadPool(int threadnum)
            : thread_num_(threadnum), run_(threadnum), queue_(threadnum)
        {
            threads_.reserve(threadnum);
        }

        ~ThreadPool()
        {
            Shutdown();
        }

        template <typename T>
        bool PushTask(T&& f)
        {
            static_assert(IsFunctor<T, task_t>::value, "invalid function type for the thread pool.");

            const int sel = sel_++;
            for (auto i = 0; i < thread_num_; ++i)
            {
                auto& q = queue_[(i + sel) % thread_num_];
                if (q.TryPush(std::forward<T>(f))) return true;
            }

            return queue_[sel].Push(std::forward<T>(f));
        }

        bool CloseThread(bool gracefully)
        {
            if (done_) return false;

            done_ = true;
            for (auto i = 0; i < thread_num_; ++i)
            {
                queue_[i].PushExitTask([&]() { run_[i]  = false; }, gracefully);
            }

            return true;
        }

        void Shutdown()
        {
            CloseThread(false);

            for (auto i = 0u; i < threads_.size(); ++i)
            {
                threads_[i].join();
            }

            for (auto i = 0u; i < queue_.size(); ++i)
            {
                queue_[i].Clear();
            }

            threads_.clear();
        }

        void StartWorking()
        {
            sel_ = -1; done_ = false;
            for (auto i = 0; i < thread_num_; ++i)
            {
                run_[i] = true;
                threads_.emplace_back([&, i]() { Entry(i); });
            }
        }

        std::vector<size_t> GetTaskNum() const
        {
            std::vector<size_t> num;

            num.reserve(thread_num_);
            for (const auto& q: queue_)
            {
                num.push_back(q.GetTaskNum());
            }

            return num;
        }

    private:
        void Entry(int id)
        {
            // the i-th thread.
            while (run_[id])
            {
                task_t fun;

                // steal work if necessary.
                for (auto i = 0; i < thread_num_; ++i)
                {
                    if (queue_[(id + i)%thread_num_].TryPop(fun)) break;
                }

                if (!fun && !queue_[id].Pop(fun)) break;

                fun();
            }

            // log the exit??
            // std::cout << "thread " << id << " exiting\n";
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

    private:
        const int thread_num_;
        std::atomic<int> sel_{-1};
        std::atomic<bool> done_{false};
        std::vector<std::atomic<bool>> run_;
        std::vector<TaskQueue> queue_;
        std::vector<std::thread> threads_;
    };

} // end xthread namespace

#endif //COMPILERFRONT_THREADPOOL_H
