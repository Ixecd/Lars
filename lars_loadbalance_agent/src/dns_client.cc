#include "lars_reactor.hpp"
#include "main_server.hpp"
#include <pthread.h>

// 与report_client通信的thread_queue消息队列
extern qc::thread_queue<lars::ReportStatusReq> *report_queue;
// 与dns_client通信的thread_queue消息队列
extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;

namespace qc {

void new_dns_request(event_loop *loop, int fd, void *args) {
    udp_client *client = (udp_client *)args;

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

void *dns_client_thread(void *args) {
    std::cout << "cur dns_client_thread start..." << std::endl;

    event_loop loop;
    // 加载配置信息
    std::string ip =
        config_file::GetInstance()->GetString("dnsserver", "ip", "localhost");

    unsigned short port =
        config_file::GetInstance()->GetNumber("dnsserver", "port", 7890);

    // 创建dns_client
    udp_client client(&loop, ip.c_str(), port);

    // 将thread_queue消息回调事件,绑定到loop上
    dns_queue->set_loop(&loop);
    dns_queue->set_callback(new_dns_request, &client);

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
