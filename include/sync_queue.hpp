#pragma once
// SYNC_QUEUE v1.0.0 github.com:pmalhaire/multithread_cpp_pub_sub.git
#include <queue>
#include <chrono>
#include <memory>
#include <condition_variable>

template <typename T>
class SynchronizedQueue
{
    std::queue<std::shared_ptr<T>> m_queue;
    std::mutex m_mutex;
    std::condition_variable m_condvar;

    using lock = std::lock_guard<std::mutex>;
    using ulock = std::unique_lock<std::mutex>;
    const int m_max_size;

public:
    SynchronizedQueue(int max_size = 0) : m_max_size(max_size)
    {
    }
    // push adds a val to the queue returns false if the queue is full
    bool push(std::shared_ptr<T> const &val)
    {
        bool wake;
        {
            lock l(m_mutex); // prevents multiple pushes corrupting m_queue
            // returns false if queue is full
            if (m_max_size > 0 && m_queue.size() == m_max_size)
            {
                return false;
            }
            bool wake = m_queue.empty(); // we may need to wake consumer
            m_queue.push(val);
        }
        if (wake)
        {
            // note one only subscriber if several it should be notify all
            m_condvar.notify_one();
        }
        return true;
    }

    int size()
    {
        return m_queue.size();
    }

    // pop_for try to pop for timeout returns nullptr if timeout fails
    std::shared_ptr<T> pop_for(const std::chrono::duration<int64_t, std::milli> timeout)
    {
        ulock u(m_mutex);
        // if queue is empty wait for an object to be pushed
        if (m_queue.empty())
        {
            if (m_condvar.wait_for(u, timeout) != std::cv_status::no_timeout)
            {
                // if timeout is reached return nullptr
                return nullptr;
            }
        }
        std::shared_ptr<T> retval = m_queue.front();
        m_queue.pop();
        return retval;
    }
};
