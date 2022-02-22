#pragma once
#include <thread>
#include <memory>
#include <chrono>
#include <atomic>
#include "sync_queue.hpp"

// SYNC_QUEUE v1.0.0 github.com:pmalhaire/multithread_cpp_pub_sub.git
// Subscriber subscribe to subscribe_queue and do work with input data
// methods must be implemented
// void work(std::shared_ptr<S>)
// void timeout()
template <typename Derived, typename T>
class Subscriber
{
public:
    Subscriber(SynchronizedQueue<T> &subscribe_queue,
               const std::chrono::duration<int64_t, std::milli> timeout)
        : m_subscribe_queue(subscribe_queue),
          m_timeout(timeout), m_thread([this]
                                       { this->loop(); }) {}
    ~Subscriber() { stop(); };

    // stop the thread
    void stop()
    {
        if (m_stop.exchange(true) == false)
        {
            m_thread.join();
        }
    }

private:
    Derived &derived() { return static_cast<Derived &>(*this); }

    // loop thread function
    void loop()
    {
        while (!this->m_stop)
        {
            std::shared_ptr<T> img = this->m_subscribe_queue.pop_for(m_timeout);
            if (img != nullptr)
            {
                if (!this->m_stop)
                {
                    derived().work(img);
                }
            }
            else
            {
                if (!this->m_stop)
                {
                    derived().timeout();
                }
            }
        }
    }

    std::thread m_thread;
    const std::chrono::duration<int64_t, std::milli> m_timeout;
    // input queue
    SynchronizedQueue<T> &m_subscribe_queue;
    std::atomic<bool> m_stop = false;
};
