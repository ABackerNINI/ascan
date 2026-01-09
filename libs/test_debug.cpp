#include <cassert>
#include <climits>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <type_traits>
#include <unistd.h>
#include <vector>

using namespace std;

#define DEBUG

#if __cplusplus >= 201103L

#ifdef DEBUG

#define cdbg __cdbg()
#define cdbgx __cdbgx()

#include <ostream>

class cerr_ostream {
  public:
    template <typename T> inline cerr_ostream &operator<<(const T &x) {
        if (isatty(STDERR_FILENO)) {
            // std::cerr << CC(CC_FG_MAGENTA, "[DEBUG] ") << x;
        } else {
            std::cerr << "[DEBUG] " << x;
        }
        return *this;
    }
    inline cerr_ostream &operator<<(std::ostream &(*func)(std::ostream &os)) {
        cerr << func;
        return *this;
    }
};

inline cerr_ostream &__cdbg() {
    static cerr_ostream cerr_os;
    return cerr_os;
}

inline cerr_ostream &__cdbgx() {
    static cerr_ostream cerr_os;
    return cerr_os;
}

#else /* DEBUG */

#include <ostream>

class null_ostream {
  public:
    template <typename T> inline null_ostream &operator<<(const T &x) {
        (void)x;
        return *this;
    }
    inline null_ostream &operator<<(std::ostream &(*func)(std::ostream &os)) {
        (void)func;
        return *this;
    }
};

inline null_ostream &__cdbg() {
    do {
        static null_ostream nul;
        return nul;
    } while (0);
}

inline null_ostream &__cdbgx() {
    do {
        static null_ostream nul;
        return nul;
    } while (0);
}

#define cdbg __cdbg()
#define cdbgx __cdbgx()

#endif /* DEBUG */

#endif /* __cplusplus */

int main() {
    int a = 10;
    // cdbg << "what" << a << endl;
    // cdbg_ex << "what" << a << endl;

    return 0;
}
