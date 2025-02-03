#include <string>

#include <proto/lars.pb.h>
#include <lars_reactor/lars_reactor.hpp>
#include <lars_reporter/store_report.hpp>
#include <lars_reporter/store_threads.hpp>
using namespace qc;

tcp_server *server;

// 直接用现成写好的就可以
thread_queue<lars::ReportStatusRequest> **reportQueues = nullptr;
// 处理io入库操作的线程数量
int threads = 0;

/// @details 这里server的io入库操作,会占用我们server的CPU时间
///          优化:将io入库操作交给专门的一个消息队列线程池来做,之后的get_report_status调用就会立即返回,来处理其他客户端请求
void get_report_status(const char *data, uint len, int msgid,
                       net_connection *conn, void *args) {
    // 封装消息访问数据库
    lars::ReportStatusRequest req;
    req.ParseFromArray(data, len);
    // 上报数据
    // StoreReport sr;
    // sr.store(req);

    // 轮询将消息平均发送到每个线程的消息队列中
    static int index = 0;
    qc_assert(reportQueues != nullptr);
    reportQueues[index++]->send(req);
    qc_assert(threads != 0);
    index = index % threads;
}

void create_reportdb_threads() {
    // 创建线程实现Reporter,这里默认两个线程
    threads = config_file_instance::GetInstance()->GetNumber("reporter", "db_thread_cnt", 2);

    // 开辟线程池的消息队列
    reportQueues = new thread_queue<lars::ReportStatusRequest> *[threads];
    qc_assert(reportQueues != nullptr);

    for (int i = 0; i < threads; ++i) {
        // std::cout << "cur create the " << i << " threads" << std::endl;
        // 给每个创建的线程创建一个消息队列
        reportQueues[i] = new thread_queue<lars::ReportStatusRequest>();
        qc_assert(reportQueues[i] != nullptr);

        // 创建线程执行report函数
        pthread_t tid;
        int rt = pthread_create(&tid, nullptr, store_main, reportQueues[i]);
        if (rt == -1) {
            perror("pthread_create");
            exit(1);
        }

        // 设置线程分离
        pthread_detach(tid);
    }
}


int main() {
    event_loop loop;

    // 配置文件
    config_file_instance::GetInstance()->setPath(
        "/home/qc/Lars/lars_reporter/config/lars_reporter.conf");

    std::string ip =
        config_file_instance::GetInstance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file_instance::GetInstance()->GetNumber("reactor", "port", 7777);

    server = new tcp_server(&loop, ip.c_str(), port);
    qc_assert(server != nullptr);

    // 这里先启动io线程池还是添加消息分发处理业务??

    // 添加数据上报请求处理的消息分发处理业务
    server->add_msg_router(lars::ID_ReportStatusRequest, get_report_status);
    // std::cout << "after add_msg_router..." << std::endl;

    // 启动io线程池
    create_reportdb_threads();

    loop.event_process();

    return 0;
}