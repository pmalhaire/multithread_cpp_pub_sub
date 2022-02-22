#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include "test_sub.hpp"
#include "test_pub.hpp"
#include "test_pub_sub.hpp"

using namespace std::chrono_literals;

TEST(async_queue, subscriber)
{
    testSub(1, 1, 0ms);
    testSub(1, 1, 10ms);
    testSub(10, 10, 10ms);
    testSub(1000, 1000, 0ms);
}

TEST(async_queue, publisher)
{
    testPub(1, 1, 0ms);
    testPub(1, 1, 10ms);
    testPub(10, 10, 10ms);
    testPub(1000, 1000, 0ms);
}

TEST(async_queue, publisher_subscriber)
{
    testPubSub(1, 1, 1ms, 10ms);
    testPubSub(1, 1, 10ms, 10ms);
    testPubSub(10, 10, 1ms, 10ms);
    testPubSub(1000, 1000, 0ms, 10ms);
    testPubSub(20, 1000, 0ms, 10ms, 2000000);
}
