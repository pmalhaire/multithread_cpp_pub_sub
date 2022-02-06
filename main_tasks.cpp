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

template <typename T>
int work(T x)
{
    std::cout << "work:" << x << std::endl;
    return 0;
}

template <typename T>
void task_lambda(T x)
{
    std::packaged_task<int(T)> task([](T x) {
        return work(x);
    });
    std::future<int> result = task.get_future();

    task(x);

    std::cout << "task_lambda:\t" << result.get() << '\n';
}

template <typename T>
void task_bind(T x)
{
    std::packaged_task<int()> task(std::bind(work<T>, x));
    std::future<int> result = task.get_future();

    task();

    std::cout << "task_bind:\t" << result.get() << '\n';
}

template <typename T>
void task_thread(T x)
{
    std::packaged_task<int(T)> task(work<T>);
    std::future<int> result = task.get_future();

    std::thread task_td(std::move(task), x);
    task_td.join();

    std::cout << "task_thread:\t" << result.get() << '\n';
}

int main()
{
    auto c = std::make_shared<MyClass>();
    task_lambda(c);
    task_bind(c);
    task_thread(c);
}