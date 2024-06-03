/**
 * @file host_info.cc
 * @author qc
 * @brief host_info信息
 * @version 0.1
 * @date 2024-06-01
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "host_info.hpp"

#include "main_server.hpp"

extern struct load_balance_config lb_config;

namespace qc {

void host_info::set_overload() {
    vsucc           = 0;
    verr            = lb_config.init_err_cnt;
    vsucc           = 0;
    rsucc           = 0;
    rerr            = 0;
    contin_succ     = 0;
    contin_err      = 0;
    overload        = true;
}

void host_info::set_idle() {
    vsucc           = lb_config.init_succ_cnt;
    verr            = 0;
    vsucc           = 0;
    rsucc           = 0;
    rerr            = 0;
    contin_succ     = 0;
    contin_err      = 0;
    overload        = false;
}

}  // namespace qc