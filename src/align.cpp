#include "align.h"
#include "debug.h"

std::ostream &operator<<(std::ostream &os, const Align::AlignWrapper &wrapper) {
    os << wrapper.word;
    if (wrapper.align.get_max_length() > wrapper.word.length()) {
        size_t n = wrapper.align.get_max_length() - wrapper.word.length();
        for (size_t i = 0; i < n; i++) {
            os << " ";
        }
    }
    return os;
}

void Align::add(const std::string &word) {
    // print_debug("word: %s\n", word.c_str());
    if (word.length() > max_length) {
        max_length = word.length();
    }
}

size_t Align::get_max_length() const {
    return max_length;
}

Align::AlignWrapper Align::operator()(const std::string &word) {
    return AlignWrapper(*this, word);
}
