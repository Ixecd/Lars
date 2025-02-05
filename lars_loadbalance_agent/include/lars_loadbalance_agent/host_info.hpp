/**
 * @file host_info.hpp
 * @author qc
 * @brief 主机信息封装
 * @version 0.1
 * @date 2024-05-24
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once
#include <lars_loadbalance_agent/main_server.hpp>

namespace qc {
/// @brief host_info中除了必要的ip +
/// port,剩下的是一些关于负载均衡算法的判定属性。
class host_info {
public:
    host_info(uint32_t ip, uint32_t port, uint32_t init_vsucc)
        : ip(ip), port(port), vsucc(init_vsucc) {}

    void set_overload();

    void set_idle();

    bool check_window();

public:
    uint32_t ip;    // host被代理主机ip
    uint32_t port;  // host被代理主机port
    uint32_t vsucc;  // 虚拟成功次数(API反馈),用于过载(overload)和空闲(idle)状态的判定
    uint32_t verr = 0;   // 虚拟失败次数(API反馈)
    uint32_t rsucc = 0;  // 真实成功次数,给Reporter上报用户观察
    uint32_t rerr = 0;   // 真实失败次数,给Reporter上报用户观察
    uint32_t contin_succ = 0;  // 连接成功次数
    uint32_t contin_err = 0;   // 连接失败次数
    // ------ 用来判断超时,如果超时就改变状态 ------ 
    long idle_ts = 0;       // 开始空闲的时间点
    long overload_ts = 0;   // 开始过载的时间点

    bool overload;  // 是否过载
};
}  // namespace qc
