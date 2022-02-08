#ifndef __SYNC_QUEUE
#define __SYNC_QUEUE

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
            bool wake = queue_.empty();      // we may need to wake consumer
            queue_.push(val);
        }
        if (wake) {
            condvar_.notify_all();
        }
    }

    int size() {
        return queue_.size();
    }

    std::shared_ptr<T> pop()
    {
        ulock u(mutex_);
        while (queue_.empty())
            condvar_.wait(u);
        // now queue_ is non-empty and we still have the lock
        std::shared_ptr<T> retval = queue_.front();
        queue_.pop();
        return retval;
    }

    std::shared_ptr<T> pop_for(const std::chrono::duration<int64_t, std::milli> timeout)
    {
        ulock u(mutex_);
        // if queue is empty wait for an object to be pushed
        if (queue_.empty())
        {
            if (condvar_.wait_for(u, timeout) != std::cv_status::no_timeout){
                // if timeout is reached return nullptr
                return nullptr;
            }
        }
        std::shared_ptr<T> retval = queue_.front();
        queue_.pop();
        return retval;
    }
};
#endif
