#pragma once
#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include "sync_queue.hpp"

// Publisher creates data to be published
template <typename T>
class Publisher
{
public:
    Publisher(SynchronizedQueue<T> &publish_queue)
        : m_publish_queue(publish_queue){};
    ~Publisher(){};

    // publish is blocking
    void publish(std::shared_ptr<T> img)
    {
        this->m_publish_queue.push(img);
    }

private:
    SynchronizedQueue<T> &m_publish_queue;
};
