#include <iostream>
#include "src/lib/bloom-filter.h"
#include "src/checker/spell-checker.h"

int main(int argc, const char* argv[])
{
    SpellChecker checker;
    checker.check_words(argc, argv);
}