#include <thread>
#include <memory>
#include <chrono>
#include "sync_queue.hpp"

// Subscriber creates data to be published
template <typename T>
class Subscriber
{
public:
    Subscriber(SynchronizedQueue<T> &subscribe_queue,
           const std::chrono::duration<int64_t, std::milli> timeout)
        : m_subscribe_queue(subscribe_queue),
          m_timeout(timeout), m_thread([this] { this->loop(); }){};
    ~Subscriber()
    {
        stop();
    };
    // do what is needed with data must be defined by heritage
    virtual void work(std::shared_ptr<T> img) = 0;
    // what to do if subscribe timeout occurs must be defined by heritage
    virtual void timeout() = 0;
    // stop the thread
    void stop()
    {
        if (m_stop == false)
        {
            m_stop = true;
            m_thread.join();
        }
    }

private:
    std::thread m_thread;
    const std::chrono::duration<int64_t, std::milli> m_timeout;

    // input queue
    SynchronizedQueue<T> &m_subscribe_queue;
    bool m_stop = false;
    // loop thread function
    void loop()
    {
        while (!this->m_stop)
        {
            std::shared_ptr<T> img = this->m_subscribe_queue.pop_for(m_timeout);
            if (img != nullptr)
            {   
                // make sure virtual func is not called after destroy
                if  (!this->m_stop){
                    work(img);
                }
            }
            else
            {
                // make sure virtual func is not called after destroy
                if  (!this->m_stop){
                    timeout();
                }
            }
        }
        return;
    }
};

