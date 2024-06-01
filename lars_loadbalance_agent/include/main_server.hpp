/**
 * @file main_server.hpp
 * @author qc
 * @brief 主服务端头文件
 * @version 0.1
 * @date 2024-05-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <netdb.h>
#include "lars_reactor.hpp"
#include "lars.pb.h"

struct load_balance_config {

    int probe_num;

    int init_succ_cnt;

    int init_err_cnt;

    float err_rate;

    float succ_rate;

    int contin_err_limits;

    int contin_succ_limits;
    // 当前agent本地ip地址(上报 填充caller字段)
    uint32_t local_ip;
};
// 在头文件中声明一个变量,之后都用这个
struct load_balance_config lb_config;

namespace qc {

    // 启动UDP server,用来接收业务层(调用者/使用者)的消息
    void start_UDP_servers();

    // 启动lars_reporter client 线程
    void start_report_client();

    // 启动lars_dns client线程
    void start_dns_client();

}

