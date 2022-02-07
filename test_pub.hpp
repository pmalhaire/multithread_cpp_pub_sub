#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
// for 1.000.000 print
#include <locale>
#include "pub.hpp"
#include "sync_queue.hpp"

using namespace std::chrono_literals;


class MyOutClass
{
public:
    MyOutClass() : m_ts(std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count()){};
    ~MyOutClass(){};
    const int64_t m_ts;
};

class MyPublisher:public Publisher<MyOutClass> {
public:
    using Publisher::Publisher;
    // this is empty but can be used in a class
};

void testPub(u_int64_t input_per_run, 
u_int64_t run_count,
const std::chrono::duration<int64_t, std::milli> wait_interval_between_runs
) {
    // initialize queues
    SynchronizedQueue<MyOutClass> pub_queue;

    // initialize and launch worker threads
    MyPublisher pub(pub_queue);

    // time the tests
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // write 1000000 as 1 000 000 for readability
    std::cout.imbue(std::locale(""));

    std::cout << "provide input data "
              << run_count * input_per_run << "msg in "
              << run_count << "runs"
              << " wait " << wait_interval_between_runs.count()
              << "ms beetween runs" << std::endl;
    for (auto i = 0; i < run_count; i++)
    {
        for (auto j = 0; j < input_per_run; j++)
        {
            auto c = std::make_shared<MyOutClass>();
            pub.publish(c);
        }
        if (wait_interval_between_runs > 0ns)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval_between_runs));
        }
    }

    std::cout << "wait for results" << std::endl;

    // interval beetween counts
    const auto timeout = 10000ms;
    constexpr auto wait_interval = 100ms;
    u_int64_t poped_msg = 0;
    while (true)
    {
        auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());
        if ( now - start > timeout) {
            std::cerr << "something is wrong test is too long >" << timeout.count() << "ms" << std::endl;
            exit(1); 
        }
        if (poped_msg > run_count * input_per_run)
        {
            std::cerr << "something is wrong too many images processed" << std::endl;
            exit(1);
        }
        else if (poped_msg == (run_count * input_per_run))
        {
            break;
        }
        // pop until timeout
        while (pub_queue.pop_for(10ms) != nullptr){
            ++poped_msg;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval));
        std::cout << "published messages received:" << +poped_msg << "/" << run_count * input_per_run << " queue size:" << pub_queue.size()  << std::endl;
    }
    auto stop = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    std::cout << "published messages:" << +poped_msg;
    if ( elapsed_ms > 0 ){
        // do not divide by 0
        std::cout <<" in "
              << elapsed_ms
              << "ms rate:" << +poped_msg / elapsed_ms * 1000 << "msg/sec";
    }
    std::cout << std::endl;

}