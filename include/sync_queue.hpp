#pragma once
// SYNC_QUEUE v1.0.0 github.com:pmalhaire/multithread_cpp_pub_sub.git
#include <queue>
#include <chrono>
#include <memory>
#include <condition_variable>

template <typename T>
class SynchronizedQueue
{
    std::queue<std::shared_ptr<T>> queue_;
    std::mutex mutex_;
    std::condition_variable condvar_;

    using lock = std::lock_guard<std::mutex>;
    using ulock = std::unique_lock<std::mutex>;

public:
    void push(std::shared_ptr<T> const &val)
    {
        bool wake;
        {
            lock l(mutex_);             // prevents multiple pushes corrupting queue_
            bool wake = queue_.empty(); // we may need to wake consumer
            queue_.push(val);
        }
        if (wake)
        {
            // note one only subscriber if several it should be notify all
            condvar_.notify_one();
        }
    }

    int size()
    {
        return queue_.size();
    }

    // pop_for try to pop for timeout returns nullptr if timeout fails
    std::shared_ptr<T> pop_for(const std::chrono::duration<int64_t, std::milli> timeout)
    {
        ulock u(mutex_);
        // if queue is empty wait for an object to be pushed
        if (queue_.empty())
        {
            if (condvar_.wait_for(u, timeout) != std::cv_status::no_timeout)
            {
                // if timeout is reached return nullptr
                return nullptr;
            }
        }
        std::shared_ptr<T> retval = queue_.front();
        queue_.pop();
        return retval;
    }
};
