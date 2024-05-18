#include <string>

#include "lars.pb.h"
#include "store_report.hpp"
#include "lars_reactor.hpp"
using namespace qc;

tcp_server *server;

void get_report_status(const char *data, uint len, int msgid, net_connection *conn, void *args) {

    // 封装消息访问数据库
    lars::ReportStatusReq req;
    req.ParseFromArray(data, len);
    // 上报数据
    StoreReport sr;
    sr.store(req);
}

int main() {
    event_loop loop;

    // 配置文件
    config_file::GetInstance()->setPath(
        "/home/qc/Lars/Reporter/config/lars_reporter.conf");

    std::string ip =
        config_file::GetInstance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::GetInstance()->GetNumber("reactor", "port", 7777);

    server = new tcp_server(&loop, ip.c_str(), port);
    qc_assert(server != nullptr);

    // 创建线程实现Reporter,这里默认两个线程
    int threads =
        config_file::GetInstance()->GetNumber("reporter", "db_thread_cnt", 2);

    // 添加数据上报请求处理的消息分发处理业务
    server->add_msg_router(lars::ID_ReportStatusReques, get_report_status);

    loop.event_process();

    return 0;
}