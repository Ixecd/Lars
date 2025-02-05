/**
 * @file main_server.cc
 * @author qc
 * @brief 主服务器实现
 * @version 0.2
 * @date 2024-05-21
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <lars_loadbalance_agent/main_server.hpp>
#include <lars_loadbalance_agent/route_lb.hpp>

#define ROUTE_NUM 3

using namespace qc;

// 与report_client通信的thread_queue消息队列,agent负责上报请求消息
// thread_queue<lars::ReportStatusReq>* report_queue;
// 与dns_client通信的thread_queue消息队列,agent负责
// thread_queue<lars::GetRouteRequest>* dns_queue;

// 每个Agent UDP server的负载均衡路由 route_lb
// 一个route_lb管理多个load_balance
route_lb* r_lb[ROUTE_NUM];
// 在源文件中声明一个变量,之后都用这个
struct load_balance_config lb_config;

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
    config_file_instance::GetInstance()->setPath(
        "/home/qc/Lars/lars_loadbalance_agent/conf/lars_lb_agent.conf");

    lb_config.probe_num =
        config_file_instance::GetInstance()->GetNumber("loadbalance", "probe_num", 10);

    lb_config.init_succ_cnt = config_file_instance::GetInstance()->GetNumber(
        "loadbalance", "init_succ_cnt", 180);

    lb_config.err_rate =
        config_file_instance::GetInstance()->GetFloat("loadbalance", "err_rate", 0.1);

    lb_config.succ_rate =
        config_file_instance::GetInstance()->GetFloat("loadbalance", "succ_rate", 0.9);

    lb_config.contin_err_limits = config_file_instance::GetInstance()->GetNumber(
        "loadbalance", "contin_err_limits", 10);

    lb_config.contin_succ_limits = config_file_instance::GetInstance()->GetNumber(
        "loadbalance", "contin_succ_limits", 10);

    // -------- 过期窗口和过载队列 ---------
    // 要在load_balance进行report()的时候,触发判定
    lb_config.window_err_rate = config_file_instance::GetInstance()->GetFloat(
        "loadbalance", "window_err_rate", 0.6);

    lb_config.idle_timeout = config_file_instance::GetInstance()->GetNumber(
        "loadbalance", "idle_timeout", 10);

    lb_config.overload_timeout = config_file_instance::GetInstance()->GetNumber(
        "loadbalance", "overload_timeout", 10);

    // -------- 定期更新路由信息 --------
    lb_config.update_timeout = config_file_instance::GetInstance()->GetNumber(
        "loadbalance", "update_timeout", 15);

    // 初始化3个route_lb模块
    create_route_lb();

    // 加载本地ip
    /// @details
    /// 之后在上报的时候,发送消息需要一个caller参数,这个caller参数我们暂时默认为当前agent的ip为caller
    char my_host_name[1024];
    if (gethostname(my_host_name, 1024) == 0) {
        struct hostent* hd = gethostbyname(my_host_name);
        if (hd) {
            struct sockaddr_in myaddr;
            myaddr.sin_addr = *(struct in_addr*)hd->h_addr;
            lb_config.local_ip = ntohl(myaddr.sin_addr.s_addr);
        }
    }

    if (!lb_config.local_ip) {
        struct in_addr inaddr;
        inet_aton("127.0.0.1", &inaddr);
        lb_config.local_ip = ntohl(inaddr.s_addr);
    }
}

int main() {
    // 1. 配置文件
    init_lb_agent();
    std::string reporter_ip =
        config_file_instance::GetInstance()->GetString("reporter", "ip", "localhost");
    unsigned short reporter_port =
        config_file_instance::GetInstance()->GetNumber("reporter", "port", 6789);

    std::string dnsserver_ip = config_file_instance::GetInstance()->GetString(
        "dnsserver", "127.0.0.1", "localhost");
    unsigned short dnsserver_port =
        config_file_instance::GetInstance()->GetNumber("dnsserver", "port", 5678);

    // 2. 启动UDP线程实现监听业务
    // 采用多进程的方式,读时共享,写时拷贝
    start_UDP_servers();

    // 只有父进程才会下来, worker进程并没有两个任务队列,所以不能和report和dns通信
    // 3. 启动report client
    // report_queue = new thread_queue<lars::ReportStatusReq>();
    // qc_assert(report_queue != nullptr);
    // start_report_client();

    // 4. 启动dns
    // dns_queue = new thread_queue<lars::GetRouteRequest>();
    // qc_assert(dns_queue != nullptr);
    // start_dns_client();

    while (1) sleep(10);

    return 0;
}
