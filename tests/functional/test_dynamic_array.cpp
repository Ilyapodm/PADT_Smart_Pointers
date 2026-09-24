#include <gtest/gtest.h>

#include "dynamic_array.hpp"

TEST(DynamicArrayTest, DefaultConstructor) {
    DynamicArray<int> array;

    EXPECT_EQ(array.get_size(), 0);
    EXPECT_EQ(array.get_capacity(), 0);
}

TEST(DynamicArrayTest, CountConstructor) {
    DynamicArray<int> array(44);

    EXPECT_EQ(array.get_size(), 44);
    EXPECT_EQ(array.get_capacity(), 44);

    
}