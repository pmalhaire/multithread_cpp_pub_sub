#include <thread>
#include <memory>
#include <chrono>
#include "sync_queue.hpp"

// PubSub subscribe to subscribe_queue and publish to publish_queue
// methods must be implemented
// std::shared_ptr<P> work(std::shared_ptr<S>)
// void timeout()
template <typename Derived, typename S, typename P>
class PubSub
{
public:
    PubSub(SynchronizedQueue<S> &subscribe_queue,
           const std::chrono::duration<int64_t, std::milli> sub_timeout,
           SynchronizedQueue<P> &publish_queue)
        : m_subscribe_queue(subscribe_queue), m_publish_queue(publish_queue),
          m_sub_timeout(sub_timeout),
          m_thread([this]
                   { this->loop(); }){};
    ~PubSub()
    {
        stop();
    };

    void stop()
    {
        if (m_stop == false)
        {
            m_stop = true;
            m_thread.join();
        }
    }

private:
    Derived &derived() { return static_cast<Derived &>(*this); }

    // loop thread function
    void loop()
    {
        std::cout << "PubSub started" << std::endl;
        while (!this->m_stop)
        {
            std::shared_ptr<S> src = this->m_subscribe_queue.pop_for(m_sub_timeout);
            // make sure virtual func is not called after destroy
            if (src != nullptr && !this->m_stop)
            {
                std::shared_ptr<P> dst = derived().work(src);
                if (dst != nullptr)
                {
                    this->m_publish_queue.push(dst);
                }
                else
                {
                    exit(156);
                    // unexpected case should never happend
                }
            }
            else if (!this->m_stop)
            {
                derived().timeout();
            }
        }
        return;
    }

    std::thread m_thread;
    const std::chrono::duration<int64_t, std::milli> m_sub_timeout;
    // subscribe_queue as input queue
    SynchronizedQueue<S> &m_subscribe_queue;
    // publish_queue as output queue
    SynchronizedQueue<P> &m_publish_queue;
    std::atomic<bool> m_stop = false;
};
