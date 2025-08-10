#include <cmath>
#include <cstdbool>
#include <vector>
#include <iostream>

class BloomFilter {
    std::vector<bool> arr;

  public:
    int m, k;
    BloomFilter(int n, double eps);
    void insert_item(std::string data);
    bool item_exists(std::string data);
};

uint64_t fnv1a(int FNV_offset_basis, const char* data);
