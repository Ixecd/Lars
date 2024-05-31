#include "lars_reactor.hpp"
#include "main_server.hpp"
#include "route_lb.hpp"
#include <pthread.h>

// 与report_client通信的thread_queue消息队列
extern qc::thread_queue<lars::ReportStatusReq> *report_queue;
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

void *dns_client_thread(void *args) {
    std::cout << "cur dns_client_thread start..." << std::endl;

    event_loop loop;
    // 加载配置信息
    std::string ip =
        config_file::GetInstance()->GetString("dnsserver", "ip", "localhost");

    unsigned short port =
        config_file::GetInstance()->GetNumber("dnsserver", "port", 7890);

    // 创建dns_client
    // dns 客户端是使用tcp传输
    tcp_client client(&loop, ip.c_str(), port, "dns_client");

    // 将thread_queue消息回调事件,绑定到loop上
    dns_queue->set_loop(&loop);
    dns_queue->set_callback(new_dns_request, &client);

    // 设置route信息
    client.add_msg_router(lars::ID_GetRouteResponse, deal_recv_route);

    loop.event_process();
}

void start_dns_client() {
    // 创建一个个线程
    pthread_t tid;
    int rt = pthread_create(&tid, nullptr, dns_client_thread, nullptr);
    qc_assert(rt != -1);

    pthread_detach(tid);
}

}
