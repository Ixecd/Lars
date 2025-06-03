#include <lars_loadbalance_agent/host_info.hpp>
#include <lars_loadbalance_agent/main_server.hpp>

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

// 时间窗口, 返回true表示需要改变状态
bool host_info::check_window() {
    if (rsucc + rerr == 0) return false;
    if ((rerr * 1.0) / (rsucc + rerr) >= lb_config.window_err_rate) return true;
    return false;
}

}  // namespace qc
