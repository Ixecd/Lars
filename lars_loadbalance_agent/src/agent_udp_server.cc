#include "lars_reactor.hpp"
#include "main_server.hpp"

// 与report_client通信的thread_queue消息队列
extern qc::thread_queue<lars::ReportStatusReq> *report_queue;
// 与dns_client通信的thread_queue消息队列
extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;

namespace qc {

// 来接收业务
void *agent_server_main(void *args) {
    int *index = (int *)args;
    // std::cout << "index = " << *index << std::endl;
    unsigned short port = *index + 8888;
    event_loop loop;

    udp_server server(&loop, "0.0.0.0", port);

    // TODO

    std::cout << "agent UDP server: port " << port << " is started...\n";

    loop.event_process();

    return nullptr;
}

void start_UDP_servers(void) {
    // 创建三个线程
    for (int i = 0; i < 3; ++i) {
        pthread_t tid;
        int rt = pthread_create(&tid, nullptr, agent_server_main, (int *)&i);
        qc_assert(rt != -1);
        pthread_detach(tid);
    }
}
}



