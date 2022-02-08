#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
// for 1.000.000 print
#include <locale>
#include "pub_sub.hpp"
#include "sync_queue.hpp"

using namespace std::chrono_literals;

class MyInClassPS
{
public:
    MyInClassPS(ssize_t buf_size) : m_ts(std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count()),
                         buffer(std::make_unique<std::vector<u_int8_t>>(buf_size))
                         {};
    ~MyInClassPS(){};
    const int64_t m_ts;
private:
    std::unique_ptr<std::vector<u_int8_t>> buffer;
};

class MyOutClassPS
{
public:
    MyOutClassPS(ssize_t buf_size=0) : m_ts(std::chrono::duration_cast<std::chrono::nanoseconds>(
                         std::chrono::system_clock::now().time_since_epoch())
                         .count()),
                         buffer(std::make_unique<std::vector<u_int8_t>>(buf_size))
                         {};
    ~MyOutClassPS(){};
    const int64_t m_ts;
private:
    std::unique_ptr<std::vector<u_int8_t>> buffer;
};


class MyPubSub:public PubSub<MyInClassPS, MyOutClassPS> {
public:
    using PubSub::PubSub;
    std::shared_ptr<MyOutClassPS> work(std::shared_ptr<MyInClassPS> img) override {
        m_message_count++;
        return std::make_shared<MyOutClassPS>();
    }

    void timeout() override {
        //std::cerr << "MyPubSub timeout" << std::endl;
    }
    u_int64_t message_count(){
        return m_message_count;
    }
private:
    u_int64_t m_message_count=0;
};

void testPubSub(u_int64_t input_per_run, 
u_int64_t run_count,
const std::chrono::duration<int64_t, std::milli> wait_interval_between_runs,
const std::chrono::duration<int64_t, std::milli> sub_timeout,
ssize_t buffer_size = 0
) {
    // initialize queues
    SynchronizedQueue<MyInClassPS> sub_queue;
    SynchronizedQueue<MyOutClassPS> pub_queue;

    // initialize and lauch worker threads
    MyPubSub pubsub(sub_queue, sub_timeout, pub_queue);
    // time the tests
    auto start = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());

    // write 1000000 as 1 000 000 for readability
    std::cout.imbue(std::locale(""));

    std::cout << "provide input data "
              << run_count * input_per_run << "msg in "
              << run_count << "runs "
              << "wait " << wait_interval_between_runs.count()
              << "ms beetween runs " 
              << "sub_timeout:" << sub_timeout.count() << "ms" << std::endl;
    
    u_int64_t pushed_msg = 0;
    u_int64_t poped_msg = 0;

    const auto timeout = 60000ms;
    auto print_interval = 1000ms;
    auto pop_timeout = 1ms;
    auto last_print_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    for (auto i = 0; i < run_count; i++)
    {
        // push messages
        for (auto j = 0; j < input_per_run; j++)
        {
            auto c = std::make_shared<MyInClassPS>(buffer_size);
            sub_queue.push(c);
            pushed_msg++;
        }

        // pop messages
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
        if (now - last_print_time > print_interval){
            std::cout << "run :" << i
                << " published messages:" 
                << +poped_msg << "/" << run_count * input_per_run 
                << " sub queue size:" << sub_queue.size()
                << " pub queue size:" << pub_queue.size()  << std::endl;
            last_print_time = now;
        }
        // pop until timeout
        while (pub_queue.pop_for(pop_timeout) != nullptr && poped_msg < pushed_msg){
            ++poped_msg;
        }
        // do no wait on last run
        if (wait_interval_between_runs > 0ns && i != run_count-1 )
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(wait_interval_between_runs));
        }
    }
    std::cout << "publish ended:" 
    << +poped_msg << "/" << run_count * input_per_run 
    << " sub queue size:" << sub_queue.size()
    << " pub queue size:" << pub_queue.size()  << std::endl;
    while (poped_msg < pushed_msg){
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
        if (pub_queue.pop_for(sub_timeout) != nullptr){
            ++poped_msg;
        }  
        if (now - last_print_time > print_interval){
            std::cout << "waiting end"
                << " published messages:" 
                << +poped_msg << "/" << run_count * input_per_run 
                << " sub queue size:" << sub_queue.size()
                << " pub queue size:" << pub_queue.size()  << std::endl;
            last_print_time = now;
        }    
    }
    std::cout << "run end published messages:" 
    << +poped_msg << "/" << run_count * input_per_run 
    << " sub queue size:" << sub_queue.size()
    << " pub queue size:" << pub_queue.size()  << std::endl;  
    auto stop = std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    if (poped_msg != run_count * input_per_run)
    {
        std::cerr << "something is wrong not all messages processed" << std::endl;
        exit(1);
    }
    auto elapsed_ms = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start).count();
    //pubsub.stop();
    std::cout << "published messages:" << +poped_msg;
    if ( elapsed_ms > 0 ){
        // do not divide by 0
        std::cout <<" in "
              << elapsed_ms
              << "ms rate:" << +poped_msg / elapsed_ms * 1000 << "msg/sec";
    }
    std::cout << std::endl;

}
