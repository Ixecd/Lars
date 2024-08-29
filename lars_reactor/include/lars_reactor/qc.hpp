#ifndef __QC_HPP__
#define __QC_HPP__

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <iostream>

namespace qc {

// #define TICK(x) auto bench_##x = std::chrono::steady_lock::now();
// #define TOCK(x) std::cout << #x " : " << std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(std::chrono::steady_lock::now() - bench_##x).count() << " ms" << std::endl;

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