#ifndef __QC_HPP__
#define __QC_HPP__

#include <assert.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

namespace qc {

typedef unsigned long long ull;

#define qc_assert(expr)                                                      \
    do {                                                                     \
        if (!(expr)) {                                                       \
            std::cerr << "Assertion failed: " << #expr << " at " << __FILE__ \
                      << ":" << __LINE__ << std::endl;                       \
            std::abort();                                                    \
        }                                                                    \
    } while (0)
 
}  // namespace qc

#endif