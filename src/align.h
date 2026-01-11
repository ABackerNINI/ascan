#ifndef __ALIGN_H__
#define __ALIGN_H__

#include <iostream>
#include <string>

class Align {
  public:
    class AlignWrapper {
      public:
        AlignWrapper(const Align &align, const std::string &word) : align(align), word(word) {}

        std::string to_string() const {
            std::string str = word;
            if (align.get_max_length() > word.length()) {
                size_t n = align.get_max_length() - word.length();
                for (size_t i = 0; i < n; i++) {
                    str += " ";
                }
            }
            return str;
        }

        friend std::ostream &operator<<(std::ostream &os, const AlignWrapper &wrapper) {
            os << wrapper.to_string();
            return os;
        }

      private:
        const Align &align;
        const std::string &word;
    };

    void add(const std::string &word) {
        // print_debug("word: %s\n", word.c_str());
        if (word.length() > max_length) {
            max_length = word.length();
        }
    }

    size_t get_max_length() const { return max_length; }

    AlignWrapper operator()(const std::string &word) const { return AlignWrapper(*this, word); }

  private:
    size_t max_length = 0;
};

#endif // __ALIGN_H__
