#include <gtest/gtest.h>
#include "src/lib/bloom-filter.h"


TEST(BloomFilterTest, BasicChecks)
{
    BloomFilter bf(3, 0.1);
    EXPECT_EQ(bf.k, 3); 
    EXPECT_EQ(bf.m, 14);
    bf.insert_item("testcase1");
    bf.insert_item("test2");
    bf.insert_item("Test3");
    EXPECT_EQ(bf.item_exists("testcase1"), true);
    EXPECT_EQ(bf.item_exists("test2"), true);
    EXPECT_EQ(bf.item_exists("Test3"), true);
    EXPECT_EQ(bf.item_exists("another!"), false);
}

TEST(BloomFilterTest, FileStoreLoad)
{
    BloomFilter bf(3, 0.1);
    bf.insert_item("testcase1");
    bf.insert_item("test2");
    bf.insert_item("Test3");
    EXPECT_EQ(bf.item_exists("testcase1"), true);
    EXPECT_EQ(bf.item_exists("test2"), true);
    EXPECT_EQ(bf.item_exists("Test3"), true);
    EXPECT_EQ(bf.item_exists("another!"), false);
    bf.write_to_file("./test.bf");

    BloomFilter bf2("./test.bf");
    EXPECT_EQ(bf.item_exists("testcase1"), true);
    EXPECT_EQ(bf.item_exists("test2"), true);
    EXPECT_EQ(bf.item_exists("Test3"), true);
    EXPECT_EQ(bf.item_exists("another!"), false);
}
