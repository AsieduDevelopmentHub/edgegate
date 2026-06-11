#include <unity.h>
#include "../src/core/circular_buffer.h"

void test_buffer_push_pop() {
    CircularBuffer<int, 4> buf;
    TEST_ASSERT_TRUE(buf.push(1));
    TEST_ASSERT_TRUE(buf.push(2));
    TEST_ASSERT_EQUAL(2, buf.size());

    int val = 0;
    TEST_ASSERT_TRUE(buf.pop(val));
    TEST_ASSERT_EQUAL(1, val);
    TEST_ASSERT_EQUAL(1, buf.size());
}

void test_buffer_full() {
    CircularBuffer<int, 2> buf;
    TEST_ASSERT_TRUE(buf.push(1));
    TEST_ASSERT_TRUE(buf.push(2));
    TEST_ASSERT_TRUE(buf.full());
    TEST_ASSERT_FALSE(buf.push(3));
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_buffer_push_pop);
    RUN_TEST(test_buffer_full);
    UNITY_END();
}

void loop() {}
