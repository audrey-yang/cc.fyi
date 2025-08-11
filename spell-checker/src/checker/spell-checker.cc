#include "spell-checker.h"

SpellChecker::SpellChecker() {
    std::string bf_dict = "/Users/aryang/Documents/cc.fyi/spell-checker/words.bf";
    std::ifstream bf_dict_file(bf_dict);
    if (bf_dict_file.is_open()) {
        bf_dict_file.close();
        bf = new BloomFilter(bf_dict);
    } else {
        std::string word_list = "/Users/aryang/Documents/cc.fyi/spell-checker/dict.txt";
        std::ifstream infile;
        infile.open(word_list, std::ios::in);
        int num_words = 0;
        std::string _unused;
        infile.seekg(0);
        while (std::getline(infile, _unused)) { 
            num_words++;
        }
        
        bf = new BloomFilter(num_words, 0.05);
        infile.clear();
        infile.seekg(0, std::ios::beg);
        std::string word;
        while (std::getline(infile, word)) { 
            bf->insert_item(word); 
        }
        infile.close();
        bf->write_to_file(bf_dict);
    }
}

bool SpellChecker::check_word(std::string word) {
    return bf->item_exists(word);
}

void SpellChecker::check_words(int argc, const char* argv[]) {
    std::vector<std::string> words(argv + 1, argv + argc);

    std::cout << "These words are spelled wrong:" << std::endl;
    for (std::string word : words) {
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        if (!check_word(word)) {
            std::cout << "\t" << word << std::endl;
        }
    }
}
