#ifndef COMPILERFRONT_THREADPOOL_H
#define COMPILERFRONT_THREADPOOL_H

#include "Basic/NonCopyable.h"

#include <deque>
#include <vector>
#include <functional>

#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace xthread {

    typedef std::function<void()> task_t;

    class TaskQueue
    {
    public:

        void Clear()
        {
            std::unique_lock<std::mutex> lock(m_);
            tasks_.clear();
        }

        bool Pop(task_t& f)
        {
            std::unique_lock<std::mutex> lock(m_);
            while (tasks_.empty()) cv_.wait(lock);

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

        // TODO: type check
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
        bool TryPush(T&& f)
        {
            {
                std::unique_lock lock{m_, std::try_to_lock};
                if (!lock) return false;

                tasks_.emplace_back(std::forward<T>(f));
            }

            cv_.notify_one();
            return true;
        }

    private:
        std::mutex m_;
        std::condition_variable cv_;

        std::deque<task_t> tasks_;
    };

    class ThreadPool : NonCopyable
    {
    public:
        ThreadPool()
            : ThreadPool(std::thread::hardware_concurrency())
        {

        }

        explicit ThreadPool(int threadnum)
            : queue_(threadnum), thread_num_(threadnum)
        {
            threads_.reserve(threadnum);
        }

        template <typename T>
        bool PushTask(T&& f)
        {
            for (auto& q: queue_)
            {
               if (q.TryPush(std::forward<T>(f))) return true;
            }

            return queue_[0].Push(std::forward<T>(f));
        }

        void Shutdown()
        {
            if (threads_.empty()) return;

            // TODO: need to impl shutdown of thread.
            // current impl will block on empty queue.
            done_ = true;
            for (auto i = 0; i < threadnum; ++i)
            {
                threads_[i].join();
            }

            threads_.clear();
        }

        void StartWorking()
        {
            for (auto i = 0; i < threadnum; ++i)
            {
                threads_.emplace_back([&]() { Entry(i); });
            }
        }

    private:
        void Entry(int id)
        {
            // the i-th thread.
            while (true)
            {
                task_t fun;

                // steal work if necessary.
                for (auto i = 0; i < thread_num_; ++i)
                {
                    if (queue_[(id + i)%thread_num_].TryPop(fun)) break;
                }

                if (!fun && !queue_[id].Pop(fun)) return;

                fun();
            }
        }

    private:
        int thread_num_;
        std::atomic<bool> done_ = false;
        std::vector<TaskQueue> queue_;
        std::vector<std::thread> threads_;
    };

} // end xthread namespace

#endif //COMPILERFRONT_THREADPOOL_H
