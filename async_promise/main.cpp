#include <iostream>
#include <cmath>
#include <thread>
#include <future>
#include <functional>
#include "sync_queue.hpp"

class MyClass
{

public:
    MyClass()
    {
        std::cout << "Ctor:" << this << std::endl;
    };
    ~MyClass()
    {
        std::cout << "Dtor:" << this << std::endl;
    };
};

SynchronizedQueue<MyClass> queue;

template <typename T>
int work_and_push(T x)
{
    std::cout << "work_and_push:" << x << std::endl;
    queue.push(std::move(x));
    std::cout << "work_and_push pushed" << std::endl;
    return 0;
}

template <typename T>
void pub_thread(T x)
{
    std::packaged_task<int(T)> task(work_and_push<T>);
    std::future<int> result = task.get_future();

    std::thread task_td(std::move(task), x);
    task_td.join();

    std::cout << "pub_thread:\t" << result.get() << std::endl;
}

template <typename T>
int pop_and_work()
{
    std::cout << "pop_and_work" << std::endl;
    T x = queue.pop();
    std::cout << "pop_and_work poped:" << x << std::endl;
    return 0;
}

template <typename T>
void sub_thread()
{
    std::packaged_task<int()> task(pop_and_work<T>);
    std::future<int> result = task.get_future();

    std::thread task_td(std::move(task));
    task_td.join();

    std::cout << "sub_thread:\t" << result.get() << std::endl;
}

int main()
{
    auto c = std::make_shared<MyClass>();
    pub_thread(c);
    sub_thread<std::shared_ptr<MyClass>>();
}