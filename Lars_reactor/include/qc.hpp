#ifndef __QC_HPP__
#define __QC_HPP__

#include <assert.h>
#include <iostream>

namespace qc {

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