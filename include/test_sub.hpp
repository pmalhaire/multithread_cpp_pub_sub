#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
// for 1.000.000 print
#include <locale>
#include "sub.hpp"
#include "sync_queue.hpp"

using namespace std::chrono_literals;

class MyInClass
{
public:
    MyInClass() : m_ts(std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count()){};
    ~MyInClass(){};
    const int64_t m_ts;
};

class MySubscriber:public Subscriber<MySubscriber, MyInClass> {
public:
    using Subscriber::Subscriber;
    void work(std::shared_ptr<MyInClass> img){
        m_message_count++;
    }

    void timeout(){
        std::cerr << "MySubscriber timeout" << std::endl;
    }
    u_int64_t message_count(){
        return m_message_count;
    }
private:
    u_int64_t m_message_count=0;
};

void testSub(u_int64_t input_per_run, 
u_int64_t run_count,
const std::chrono::duration<int64_t, std::milli> wait_interval_between_runs
) {
    // initialize queues
    SynchronizedQueue<MyInClass> sub_queue;

    // initialize and launch worker threads
    MySubscriber sub(sub_queue, 100ms);

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
            auto c = std::make_shared<MyInClass>();
            sub_queue.push(c);
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
    while (true)
    {
        auto now = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());
        if ( now - start > timeout) {
            std::cerr << "something is wrong test is too long >" << timeout.count() << "ms" << std::endl;
            exit(1); 
        }
        if (sub.message_count() > run_count * input_per_run)
        {
            std::cerr << "something is wrong too many images processed" << std::endl;
            exit(1);
        }
        else if (sub.message_count() == (run_count * input_per_run))
        {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval));
        std::cout << "processed messages:" << +sub.message_count() << "/" << run_count * input_per_run << std::endl;
    }
    auto stop = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    std::cout << "Stop threads" << std::endl;
    //sub.stop();
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();

    std::cout << "finished processed messages:" << +sub.message_count();
    if ( elapsed_ms > 0 ){
        // do not divide by 0
        std::cout <<" in "
              << elapsed_ms
              << "ms rate:" << +sub.message_count() / elapsed_ms * 1000 << "msg/sec";
    }
    std::cout << std::endl;

}
