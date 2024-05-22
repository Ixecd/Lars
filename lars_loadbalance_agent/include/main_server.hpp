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
#include "lars_reactor.hpp"
#include "lars.pb.h"

namespace qc{

// 启动UDP server,用来接收业务层(调用者/使用者)的消息
void start_UDP_servers(void);

// 启动lars_reporter client 线程
void start_report_client(void);

// 启动lars_dns client线程
void start_dns_client(void);
}




