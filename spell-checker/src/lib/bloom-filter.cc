#include "bloom-filter.h"

BloomFilter::BloomFilter(int n, double eps) {
    // From https://en.wikipedia.org/wiki/Bloom_filter#Optimal_number_of_hash_functions
    m = int(-n * std::log(eps) / (std::log(2) * std::log(2)));
    k = int(-std::log(eps) / std::log(2));
    arr.reserve(m);
}

BloomFilter::BloomFilter(std::string filename) {    
    std::ifstream infile;
    infile.open(filename, std::ios::binary | std::ios::in);
    char hdr[12];

    infile.seekg(0, std::ios::beg);
    infile.read(hdr, 12);
    k = hdr[6] << 8 | hdr[7];
    m = (hdr[8] & 0xff) << 24 | (hdr[9] & 0xff) << 16 | (hdr[10] & 0xff) << 8 | (hdr[11] & 0xff);

    arr.reserve(m);
    for (int i = 0; i < m; i++) {
        char bit;
        infile.read(&bit, 1);
        arr[i] = bit;
    }

    infile.close();
}

void BloomFilter::insert_item(std::string data) {
    for (int i = 0; i < k; i++) {
        uint64_t pos = fnv1a(i, data.c_str()) % m;
        arr[pos] = 1;
    }
}

bool BloomFilter::item_exists(std::string data) {
    for (int i = 0; i < k; i++) {
        uint64_t pos = fnv1a(i, data.c_str()) % m;
        if (arr[pos] == 0) {
            return false;
        }
    }
    return true;
}

void BloomFilter::write_to_file(std::string filename) {
    std::ofstream outfile;
    outfile.open(filename, std::ios::binary | std::ios::out);

    char hdr[12] = {'C', 'C', 'B', 'F'};
    // version number
    hdr[4] = 0;
    hdr[5] = 1;
    // k
    hdr[6] = k >> 8 & 0xff;
    hdr[7] = k & 0xff;
    // m
    hdr[8] = m >> 24 & 0xff;
    hdr[9] = m >> 16 & 0xff;
    hdr[10] = m >> 8 & 0xff;
    hdr[11] = m & 0xff;

    outfile.write(hdr, 12);
    outfile.write(&arr[0], m);
    outfile.close();
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