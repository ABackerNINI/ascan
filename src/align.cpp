#include "align.h"

std::ostream &operator<<(std::ostream &os, const Align::AlignWrapper &wrapper) {
    os << wrapper.to_string();
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

Align::AlignWrapper Align::operator()(const std::string &word) const {
    return AlignWrapper(*this, word);
}

std::string Align::AlignWrapper::to_string() const {
    std::string str = word;
    if (align.get_max_length() > word.length()) {
        size_t n = align.get_max_length() - word.length();
        for (size_t i = 0; i < n; i++) {
            str += " ";
        }
    }
    return str;
}
