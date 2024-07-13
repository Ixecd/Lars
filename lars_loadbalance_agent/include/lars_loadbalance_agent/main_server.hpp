/**
 * @file main_server.hpp
 * @author qc
 * @brief 主服务端头文件
 * @version 2
 * @date 2024-05-21
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <netdb.h>
#include <lars_reactor/lars_reactor.hpp>
#include "../../../base/proto/lars.pb.h"

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


    // ------- 过期窗口和过载超时 -------
    float window_err_rate;

    int idle_timeout;

    int overload_timeout;

    // ------- 定期更新本地路由 -------
    long update_timeout;
};

namespace qc {

    // 启动UDP server,用来接收业务层(调用者/使用者)的消息
    void start_UDP_servers();

    // 启动lars_reporter client 线程
    void start_report_client();

    // 启动lars_dns client线程
    void start_dns_client();

}

