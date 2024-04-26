#ifndef __QC_HPP__
#define __QC_HPP__

#include <assert.h>
#include <stdio.h>

namespace qc {


#define qc_assert(expr)                                                        \
    do {                                                                       \
        if (!(expr)) {                                                         \
            fprintf(stderr, "Assertion failed: %s, file %s, line %d\n", #expr, \
                    __FILE__, __LINE__);                                       \
            assert(expr);                                                      \
        }                                                                      \
    } while (0);

    
}  // namespace qc

#endif