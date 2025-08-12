#include <iostream>
#include "src/lib/bloom-filter.h"
#include "src/checker/spell-checker.h"

int main(int argc, const char* argv[])
{
    if (strcmp(argv[1], "-build") == 0) {
        SpellChecker::load_dict(std::string(argv[2]));
    } else {
        SpellChecker checker;
        checker.check_words(argc, argv);
    }
}