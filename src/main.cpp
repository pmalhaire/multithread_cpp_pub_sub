#include <iostream>
#include <thread>
#include <memory>
#include <chrono>
#include "test_sub.hpp"
#include "test_pub.hpp"
#include "test_pub_sub.hpp"

using namespace std::chrono_literals;

int main()
{
    // std::cout << "# Test subscriber" << std::endl;
    // testSub(1,1,0ms);
    // testSub(1,1,10ms);
    // testSub(10,10,10ms);
    // testSub(1000,1000,0ms);

    // std::cout << std::endl << "# Test publisher" << std::endl;
    // testPub(1,1,0ms);
    // testPub(1,1,10ms);
    // testPub(10,10,10ms);
    // testPub(1000,1000,0ms);

    std::cout << std::endl << "# Test publisher subscriber" << std::endl;
    testPubSub(1,1,1ms,100ms);
    testPubSub(1,1,10ms,100ms);
    testPubSub(10,10,1ms,100ms);
    testPubSub(1000,1000,0ms,1000ms);
    testPubSub(10,1000,0ms,1000ms,1000000);
}