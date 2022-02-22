#include "sync_queue.hpp"
#include <gtest/gtest.h>

void testFull()
{
    SynchronizedQueue<int> two_val_queue(2);

    auto one = std::make_shared<int>(1);
    auto two = std::make_shared<int>(2);
    auto three = std::make_shared<int>(3);
    ASSERT_TRUE(two_val_queue.push(one));
    ASSERT_TRUE(two_val_queue.push(two));
    ASSERT_FALSE(two_val_queue.push(three));
}
