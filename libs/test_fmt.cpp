#include <iostream>
#include <string>

#include "fmt/color.h"

using namespace std;

int main(int argc, char **argv) {
    fmt::print(fmt::bg(fmt::color::red), "WARNING: Template file is missing some placeholders");
    fmt::print("\nrcc expects the following placeholders:\n");
    fmt::print("something else\n");

    return 0;
}
