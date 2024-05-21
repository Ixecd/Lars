#include <pthread.h>
#include "lars_reactor.hpp"
#include "main_server.hpp"

namespace qc {

void *report_client_thread(void *args) {
    std::cout << "report client thread is running...\n";
// 如果 定义了0那就创建客户端
#if 0
    event_loop loop;

    // 通过配置文件得到配置信息
    std::string ip = config_file::GetInstance()->GetString("reporter", "ip", "localhost");

    unsigned short port = config_file::GetInstance()->GetNumber("reporter","port", 0);
    
    udp_client client(&loop, ip.c_str(), port, "reporter client");

    // 将thread_queue消息回调事件绑定到loop中
    report_queue->set_loop(&loop);
    report_queue->set_callback();
    
    loop.event_process();
#endif
    return nullptr;
}

void start_report_client(void) {
    // 开辟一个线程
    pthread_t tid;

    int rt = pthread_create(&tid, nullptr, report_client_thread, nullptr);
    qc_assert(rt != -1);

    // 线程分离
    pthread_detach(tid);
}

}