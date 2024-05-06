#ifndef __QC_HPP__
#define __QC_HPP__

#include <assert.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>

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

/**
 * @details 编译器优化  qc_likely(x) -> x 大概率成立(1),优化; 
 *                    qc_unlikely(x)-> x大概率不成立(0),优化;
 *   __builtin_expect 是 GCC 和 LLVM 的内建函数，用于提供分支预测的信息
 */
#if         defined __GNUC__ || defined __llvm__
#define     qc_likely(x) __builtin_expect(!!(x), 1)
#define     qc_unlikely(x) __builtin_expect(!!(x), 0)
#else
#define     qc_likely(x) (x)
#define     qc_unlikely(x) (x)
#endif

}  // namespace qc

#endif