#include <iostream>
#include "src/lib/bloom-filter.h"

int main(int argc, char **argv)
{
    std::cerr << "Hello, world" << std::endl;
    BloomFilter bf(5, 0.5);
    std::cerr << bf.k << ", " << bf.m << std::endl;
    bf.insert_item("hello");
    bf.insert_item("Hello, world");
    bf.insert_item("another test string");
    std::cerr << (bf.item_exists("hello") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("Hello") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("Hello, world") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("another test string") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("another test string!!") ? "FOUND" : "NOT FOUND") << std::endl;
}