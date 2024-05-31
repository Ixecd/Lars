/**
 * @file main_server.cc
 * @author qc
 * @brief 主服务器实现
 * @version 0.1
 * @date 2024-05-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "main_server.hpp"
#include "route_lb.hpp"

#define ROUTE_NUM 3

using namespace qc;

// 与report_client通信的thread_queue消息队列,agent负责上报请求消息
thread_queue<lars::ReportStatusReq>* report_queue;
// 与dns_client通信的thread_queue消息队列,agent负责
thread_queue<lars::GetRouteRequest>* dns_queue;
// 每个Agent UDP server的负载均衡路由 route_lb
// 一个route_lb管理多个load_balance
route_lb *r_lb[3];

/// @brief 默认route_loadbalance的编号从1开始
static void create_route_lb() {
    int id = 0;
    for (int i = 0; i < ROUTE_NUM; ++i) {
        id = i + 1;
        r_lb[i] = new route_lb(id);
        qc_assert(r_lb[i] != nullptr);
    }
}

static void init_lb_agent() {
    // 配置环境
    config_file::GetInstance()->setPath(
        "/home/qc/Lars/lars_loadbalance_agent/conf/lars_lb_agent.conf");
    lb_config.probe_nunm = config_file::GetInstance()->GetNumber("loadbalance", "probe_num", 10);
    lb_config.init_succ_cnt = config_file::GetInstance()->GetNumber("loadbalance", "init_succ_cnt", 180);

    create_route_lb();
}


int main() {
    init_lb_agent();

    // 1. 配置文件
    config_file::GetInstance()->setPath(
        "/home/qc/Lars/lars_loadbalance_agent/conf/lars_lb_agent.conf");
    std::string reporter_ip =
        config_file::GetInstance()->GetString("reporter", "ip", "localhost");
    unsigned short reporter_port =
        config_file::GetInstance()->GetNumber("reporter", "port", 6789);

    std::string dnsserver_ip = config_file::GetInstance()->GetString(
        "dnsserver", "127.0.0.1", "localhost");
    unsigned short dnsserver_port =
        config_file::GetInstance()->GetNumber("dnsserver", "port", 5678);

    // 2. 启动UDP线程实现监听业务
    start_UDP_servers();

    // 3. 启动report client
    report_queue = new thread_queue<lars::ReportStatusReq>();
    qc_assert(report_queue != nullptr);
    start_report_client();

    // 4. 启动dns
    dns_queue = new thread_queue<lars::GetRouteRequest>();
    qc_assert(dns_queue != nullptr);
    start_dns_client();

    while (1) sleep(10);

    return 0;
}
