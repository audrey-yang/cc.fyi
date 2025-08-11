#ifndef SPELL_CHECKER_H
#define SPELL_CHECKER_H
#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>

#include "src/lib/bloom-filter.h"

class SpellChecker {
    BloomFilter* bf;
    bool check_word(std::string);

  public:
    SpellChecker();
    void check_words(int argc, const char* argv[]);
};

uint64_t fnv1a(int FNV_offset_basis, const char* data);
#endif
