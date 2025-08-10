#include "bloom-filter.h"

BloomFilter::BloomFilter(int n, double eps) {
    // From https://en.wikipedia.org/wiki/Bloom_filter#Optimal_number_of_hash_functions
    m = int(-n * std::log(eps) / (std::log(2) * std::log(2)));
    k = int(-std::log(eps) / std::log(2));
    arr.reserve(m);
}

void BloomFilter::insert_item(std::string data) {
    std::cerr << data << std::endl;
    for (int i = 0; i < k; i++) {
        uint64_t pos = fnv1a(i, data.c_str()) % m;
        arr[pos] = true;
    }
}

bool BloomFilter::item_exists(std::string data) {
    for (int i = 0; i < k; i++) {
        uint64_t pos = fnv1a(i, data.c_str()) % m;
        if (!arr[pos]) {
            return false;
        }
    }
    return true;
}

uint64_t fnv1a(int FNV_offset_basis, const char* data) {
    uint64_t FNV_64_PRIME = 0x100000001b3;
    
    // from https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
    int hash = FNV_offset_basis;
    for (int i = 0; i < strlen(data); i++) {
        hash ^= int(data[i]);
        hash *= FNV_64_PRIME;
    }
    return hash;
}