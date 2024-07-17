#include <pthread.h>
#include <lars_reactor/lars_reactor.hpp>
#include <lars_loadbalance_agent/main_server.hpp>

// 与report_client通信的thread_queue消息队列
// thread_queue<lars::ReportStatusReq> *report_queue;
// 与dns_client通信的thread_queue消息队列
// thread_queue<lars::GetRouteRequest> *dns_queue;

// 与report_client通信的thread_queue消息队列
extern qc::thread_queue<lars::ReportStatusReq> *report_queue;
// 与dns_client通信的thread_queue消息队列
extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;

namespace qc {

/// @brief 只要任务队列中有数据epoll就会调用这个函数来处理业务
void new_report_request(event_loop *loop, int fd, void *args) {
    udp_client *client = (udp_client *)args;

    // 1. 将请求从队列中取出来
    std::queue<lars::ReportStatusReq> msgs;
    qc_assert(report_queue != nullptr);
    report_queue->recv(msgs);

    // 2. 遍历队列,通过client依次将每个msg发送给reporter service
    while (!msgs.empty()) {
        lars::ReportStatusReq reportReq;
        reportReq = msgs.front();
        msgs.pop();

        // 3. 序列化数据
        std::string requestString;
        reportReq.SerializeToString(&requestString);
        qc_assert(requestString.size() != 0);

        // 4. 客户端发送消息
        client->send_message(requestString.c_str(), requestString.size(),
                             lars::ID_ReportStatusReques);
    }
}

void *report_client_thread(void *args) {
    std::cout << "report client thread is running...\n";
#if 1
    event_loop loop;

    // 通过配置文件得到配置信息
    std::string ip =
        config_file_instance::GetInstance()->GetString("reporter", "ip", "localhost");

    unsigned short port =
        config_file_instance::GetInstance()->GetNumber("reporter", "port", 0);

    // udp_client client(&loop, ip.c_str(), port);
    // 只有处理业务的一方才是UDP, reporter和dns都是tcp传输
    tcp_client client(&loop, ip.c_str(), port, "reporter client");

    // 将thread_queue消息回调事件绑定到loop中
    report_queue->set_loop(&loop);
    report_queue->set_callback(new_report_request, &client);

    loop.event_process();
#endif
    return nullptr;
}

void start_report_client() {
    // 开辟一个线程
    pthread_t tid;

    int rt = pthread_create(&tid, nullptr, report_client_thread, nullptr);
    qc_assert(rt != -1);

    // 线程分离
    pthread_detach(tid);
}
}

