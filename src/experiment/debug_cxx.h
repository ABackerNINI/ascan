#ifndef __DEBUG2_H__
#define __DEBUG2_H__

#ifdef DEBUG

#define cdbg __cdbg()
#define cdbgx __cdbg_ex()

inline static ostream &__cdbg() {
    cerr << "[DEBUG] ";
    return cerr;
}

inline static ostream &__cdbg_ex() {
    return cerr;
}

#else // DEBUG

#include <ostream>

class nul_ostream {
  public:
    template <typename T> inline nul_ostream &operator<<(const T &x) {
        (void)x;
        return *this;
    }
    inline nul_ostream &operator<<(std::ostream &(*f)(std::ostream &)) {
        (void)f;
        return *this;
    }
};

inline static nul_ostream &__cdbg() {
    static nul_ostream nul;
    return nul;
}

inline static nul_ostream &__cdbg_ex() {
    static nul_ostream nul;
    return nul;
}

#define cdbg __cdbg()
#define cdbgx __cdbg_ex()

#endif // DEBUG

#endif // __DEBUG2_H__
