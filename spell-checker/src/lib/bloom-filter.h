#include <cmath>
#include <cstdbool>
#include <vector>
#include <iostream>
#include <fstream>

class BloomFilter {
    uint32_t m, k;
    std::vector<char> arr;

  public:
    BloomFilter(int n, double eps);
    BloomFilter(std::string filename);
    void insert_item(std::string data);
    bool item_exists(std::string data);
    void write_to_file(std::string filename);
};

uint64_t fnv1a(int FNV_offset_basis, const char* data);
