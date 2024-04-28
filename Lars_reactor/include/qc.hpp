#ifndef __QC_HPP__
#define __QC_HPP__

#include <assert.h>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

namespace qc {

#define qc_assert(expr)                                                      \
    do {                                                                     \
        if (!(expr)) {                                                       \
            std::cerr << "Assertion failed: " << #expr << " at " << __FILE__ \
                      << ":" << __LINE__ << std::endl;                       \
            std::abort();                                                    \
        }                                                                    \
    } while (0)
 
/// @return 1 success
int SetNonblocking(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    qc_assert(fcntl(fd, F_SETFL, O_NONBLOCK | flag) != -1);
    return 1;
}


}  // namespace qc

#endif