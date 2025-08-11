#include <iostream>
#include "src/lib/bloom-filter.h"

int main(int argc, char **argv)
{
    std::cerr << "Hello, world" << std::endl;
    BloomFilter bf(2000, 0.01);
    bf.insert_item("hello");
    bf.insert_item("Hello, world");
    bf.insert_item("another test string");
    std::cerr << (bf.item_exists("hello") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("Hello") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("Hello, world") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("another test string") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bf.item_exists("another test string!!") ? "FOUND" : "NOT FOUND") << std::endl;
    bf.write_to_file("/Users/aryang/Documents/cc.fyi/spell-checker/test.bf");

    BloomFilter bfcopy("/Users/aryang/Documents/cc.fyi/spell-checker/test.bf");
    std::cerr << (bfcopy.item_exists("hello") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bfcopy.item_exists("Hello") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bfcopy.item_exists("Hello, world") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bfcopy.item_exists("another test string") ? "FOUND" : "NOT FOUND") << std::endl;
    std::cerr << (bfcopy.item_exists("another test string!!") ? "FOUND" : "NOT FOUND") << std::endl;
}