#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
//for 1.000.000 print
#include <locale>
using namespace std::chrono_literals;
#include "sync_queue.hpp"

class MyClass
{

public:
    MyClass() : m_ts(std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count()){};
    ~MyClass(){};
    const int64_t m_ts;
};

std::ostream &operator<<(std::ostream &os, MyClass const &m)
{
    return os << "MyClass" << m.m_ts;
}
using myclass_shared = std::shared_ptr<MyClass>;

// Graber subscribe to grab_queue and publish to process queue
class Graber
{
public:
    Graber(SynchronizedQueue<myclass_shared> &grab_queue,
           SynchronizedQueue<myclass_shared> &process_queue,
           const std::chrono::duration<int64_t, std::milli> timeout)
        : m_grab_queue(grab_queue), m_process_queue(process_queue),
          m_pop_timeout(timeout), m_thread([this] { this->grab(); }){};
    ~Graber()
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
    u_int64_t grab_count()
    {
        return m_grab_count;
    }

private:
    std::thread m_thread;
    const std::chrono::duration<int64_t, std::milli> m_pop_timeout;
    u_int64_t m_grab_count = 0;
    // grab queue as input queue
    SynchronizedQueue<myclass_shared> &m_grab_queue;
    // process queue as output queue
    SynchronizedQueue<myclass_shared> &m_process_queue;
    bool m_stop = false;
    // do what is needed with data
    void work() {}
    // handle the timeout case
    void timeout() {}
    // grab from m_grab_queue to process queue
    void grab()
    {
        while (!this->m_stop)
        {
            myclass_shared img = this->m_grab_queue.pop_for(m_pop_timeout);
            if (img != nullptr)
            {
                work();
                this->m_process_queue.push(img);
                ++m_grab_count;
                //std::cout << "grab done" << std::endl;
            }
            else
            {
                timeout();
                //std::cout << "grab timeout" << std::endl;
            }
        }
        return;
    }
};

// Processor subscribe to to process queue and work with data
class Processor
{
public:
    Processor(SynchronizedQueue<myclass_shared> &process_queue,
              const std::chrono::duration<int64_t, std::milli> timeout)
        : m_process_queue(process_queue), m_pop_timeout(timeout),
          m_thread([this] { this->process(); }){};
    ~Processor()
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
    u_int64_t process_count()
    {
        return m_process_count;
    }

private:
    std::thread m_thread;
    const std::chrono::duration<int64_t, std::milli> m_pop_timeout;
    u_int64_t m_process_count = 0;
    SynchronizedQueue<myclass_shared> &m_process_queue;
    bool m_stop = false;
    // do what is needed with data
    void work() {}
    // handle the timeout case
    void timeout() {}
    // process data from process queue
    void process()
    {
        while (!this->m_stop)
        {
            myclass_shared img = this->m_process_queue.pop_for(m_pop_timeout);
            if (img != nullptr)
            {
                work();
                ++m_process_count;
                //std::cout << "proess done" << std::endl;
            }
            else
            {
                timeout();
                //std::cout << "process timeout" << std::endl;
            }
        }
        return;
    }
};

int main()
{
    // test inputs
    // number of grab data to be produced by run
    constexpr int grab_per_run = 1000;
    // number of run to do
    constexpr int run_count = 10000;
    // interval beetween runs
    constexpr auto wait_interval_between_runs = 0ms;

    // initialize queues
    SynchronizedQueue<myclass_shared> grab_queue;
    SynchronizedQueue<myclass_shared> process_queue;

    // initialize and lauch worker threads
    Graber grabber(grab_queue, process_queue, 10ms);
    Processor processor(process_queue, 10ms);

    // time the tests
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // write 1000000 as 1 000 000 for readability
    std::cout.imbue(std::locale(""));
    std::cout << "provide grab data "
              << run_count * grab_per_run << "msg in "
              << run_count << "runs "
              << "wait " << wait_interval_between_runs.count()
              << "ms beetween runs" << std::endl;
    for (auto i = 0; i < run_count; i++)
    {
        for (auto j = 0; j < grab_per_run; j++)
        {
            auto c = std::make_shared<MyClass>();
            grab_queue.push(c);
        }
        if (wait_interval_between_runs > 0ns)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval_between_runs));
        }
    }

    std::cout << "wait for results" << std::endl;

    // interval beetween counts
    constexpr auto wait_interval = 100ms;
    while (true)
    {
        if (processor.process_count() > run_count * grab_per_run)
        {
            std::cerr << "something is wrong too many images processed" << std::endl;
            exit(1);
        }
        else if (processor.process_count() == (run_count * grab_per_run))
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval));
        std::cout << "processed_images:" << +processor.process_count() << "/" << run_count * grab_per_run << std::endl;
    }
    auto stop = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    std::cout << "Stop threads" << std::endl;
    grabber.stop();
    processor.stop();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    auto elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(stop - start).count();
    std::cout << "finished processed_images:" << +processor.process_count() << " in "
              << elapsed_ms
              << "ms rate:" << +processor.process_count() / elapsed_s << "msg/sec" << std::endl;
}
