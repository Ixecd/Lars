/**
 * @file dns_client.cc
 * @author qc
 * @brief 定期拉取最新路由信息
 * @version 0.1
 * @date 2024-06-03
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <lars_loadbalance_agent/main_server.hpp>
#include <lars_loadbalance_agent/route_lb.hpp>
#include <lars_reactor/lars_reactor.hpp>

#include <pthread.h>

// 与report_client通信的thread_queue消息队列
// qc::thread_queue<lars::ReportStatusReq> *report_queue;
// 与dns_client通信的thread_queue消息队列
extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;
// route_loadbalance
extern qc::route_lb *r_lb[3];

namespace qc {

void new_dns_request(event_loop *loop, int fd, void *args) {
    // ------------------------------------
    // --------- DNS Tcp_client -----------
    tcp_client *client = (tcp_client *)args;

    // 1. 将请求从dns_queue中取出来
    std::queue<lars::GetRouteRequest> msgs;
    qc_assert(dns_queue != nullptr);
    dns_queue->recv(msgs);

    while (!msgs.empty()) {
        lars::GetRouteRequest req = msgs.front();
        msgs.pop();

        std::string requestString;
        req.SerializeToString(&requestString);
        qc_assert(requestString.size() != 0);

        client->send_message(requestString.c_str(), requestString.size(),
                             lars::ID_GetRouteRequest);
    }
}

// 处理dns service返回的路由信息
void deal_recv_route(const char *data, int msglen, int msgid, net_connection *conn, void *args) {
    lars::GetRouteResponse rsp;

    rsp.ParseFromArray(data, msglen);
    // rsp.ParseFromString(data);

    int modid = rsp.modid();
    int cmdid = rsp.cmdid();
    int index = (modid + cmdid) % 3;

    // 将该modid/cmdid交给一个route_lb
    // 收到一个DNS请求,更新route_lb下的host信息
    r_lb[index]->update_host(modid, cmdid, rsp);
}

/// @details agent首次连接到dns service时,将全部的load_balance都设置为NEW状态.如果dns service重新启动,或者断开连接重连,都会将之前的拉去中或没拉取的load_balance状态都设置为NEW状态,只有NEW状态的load_balance才能定期自动拉取

/// @brief hook
static void conn_init(net_connection *conn, void *args) {
    for (int i = 0; i < 3; ++i) {
        r_lb[i]->reset_lb_status();
    }
}

void *dns_client_thread(void *args) {
    std::cout << "cur dns_client_thread start..." << std::endl;

    event_loop loop;
    // 加载配置信息
    std::string ip =
        config_file_instance::GetInstance()->GetString("dnsserver", "ip", "localhost");

    unsigned short port =
        config_file_instance::GetInstance()->GetNumber("dnsserver", "port", 7890);

    // 创建dns_client
    // dns 客户端是使用tcp传输
    tcp_client client(&loop, ip.c_str(), port, "dns_client");

    dns_queue = (qc::thread_queue<lars::GetRouteRequest> *)args;

    // 将thread_queue消息回调事件,绑定到loop上
    dns_queue->set_loop(&loop);
    dns_queue->set_callback(new_dns_request, &client);

    // 设置route信息
    client.add_msg_router(lars::ID_GetRouteResponse, deal_recv_route);

    // 设置连接成功/连接断开重连成功之后,通过conn_init来清理之前的任务
    client.set_conn_start(conn_init);

    loop.event_process();
}

void start_dns_client(qc::thread_queue<lars::GetRouteRequest> *dns_queue) {
    // 创建一个个线程
    pthread_t tid;
    int rt = pthread_create(&tid, nullptr, dns_client_thread, dns_queue);
    qc_assert(rt != -1);

    pthread_detach(tid);
}

}
