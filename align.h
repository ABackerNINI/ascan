#ifndef __ALIGN_H__
#define __ALIGN_H__

#include <iostream>
#include <string>

class Align {
  public:
    class AlignWrapper {
      public:
        AlignWrapper(Align &align, const std::string &word) : align(align), word(word) {}

        friend std::ostream &operator<<(std::ostream &os, const AlignWrapper &wrapper);

      private:
        Align &align;
        const std::string &word;
    };

    void add(const std::string &word);

    size_t get_max_length() const;

    AlignWrapper operator()(const std::string &word);

  private:
    size_t max_length = 0;
};

#endif // __ALIGN_H__
