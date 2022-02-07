#include <iostream>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;
#include "sync_queue.hpp"

// to be able to follow object creation
//#define SHOW_CTOR
//#deine SHOW_DTOR
class MyClass
{

public:
    MyClass() : m_ts(std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count())
    {
#if SHOW_CTOR
        std::cout << "MyClass Ctor:" << this->m_ts << std::endl;
#endif
    };
    ~MyClass()
    {
#if SHOW_DTOR
        std::cout << "MyClass Dtor:" << this->m_ts << std::endl;
#endif
    };
    const int64_t m_ts;
};

std::ostream &operator<<(std::ostream &os, MyClass const &m)
{
    return os << "MyClass" << m.m_ts;
}

SynchronizedQueue<MyClass> grab_queue;
SynchronizedQueue<MyClass> process_queue;
constexpr auto timeout = 10ms;

bool stop_grab = false;
bool stop_process = false;
auto processed_images = 0;

template <typename T>
int grab()
{
    while (!stop_grab)
    {
        T img = grab_queue.pop_for(timeout);
        if (img != nullptr)
        {
            // std::cout << "grab:" << img->m_ts << std::endl;
            process_queue.push(img);
            // std::cout << "grab done" << std::endl;
        }
        else
        {
            // std::cout << "grab timeout" << std::endl;
        }
    }
    return 0;
}

template <typename T>
int process()
{
    while (!stop_process)
    {
        T img = process_queue.pop_for(timeout);
        if (img != nullptr)
        {
            // std::cout << "process:" << img->m_ts << std::endl;
            // std::cout << "process done" << std::endl;
            ++processed_images;
        }
        else
        {
            // std::cout << "process timeout" << std::endl;
        }
    }
    return 0;
}

int main()
{

    auto grab_thread = std::thread(grab<std::shared_ptr<MyClass>>);
    auto process_thread = std::thread(process<std::shared_ptr<MyClass>>);
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    constexpr int run_count = 10000;
    constexpr int grab_per_run = 1000;
    // used to test the empty queue case
    constexpr auto wait_interval_between_runs = 0ms;
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

    constexpr auto wait_interval = 100ms;
    while (true)
    {
        if (processed_images > run_count * grab_per_run)
        {
            std::cerr << "something is wrong too many images processed" << std::endl;
            exit(1);
        }
        else if (processed_images == (run_count * grab_per_run))
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval));
        std::cout << "processed_images:" << +processed_images << "/" << run_count * grab_per_run << std::endl;
    }
    auto stop = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    stop_grab = true;
    stop_process = true;
    std::cout << "wait for stop" << std::endl;
    grab_thread.join();
    process_thread.join();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    auto elapsed_s = std::chrono::duration_cast<std::chrono::seconds>(stop - start).count();
    std::cout << "finished processed_images:" << +processed_images << " in "
              << elapsed_ms
              << "ms rate:" << processed_images / elapsed_s << "msg/sec" << std::endl;
}