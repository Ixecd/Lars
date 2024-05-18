#include "lars_reactor.hpp"
#include "store_report.hpp"
#include "lars.pb.h"
using namespace qc;

void report_status(net_connection *conn, void *args) {
    // 客户端负责发送消息
    tcp_client *client = (tcp_client *)args;

    lars::ReportStatusReq req;
    // 这里只是简单测试一下组装消息
    req.set_modid(rand() % 3);
    req.set_cmdid(1);
    req.set_caller(123);
    req.set_ts(time(nullptr));
    // 向req中添加三个客户端信息
    for (int i = 0; i < 3; ++i) {
        lars::HostCallResult host;
        host.set_ip(i + 1);
        host.set_port((i + 1) * (i + 1));
        host.set_err(3);
        host.set_succ(100);
        host.set_overload(true);

        req.add_results()->CopyFrom(host);
    }

    std::string request;
    req.SerializeToString(&request);

    conn->send_message(request.c_str(), request.size(),
                       lars::ID_ReportStatusReques);
}

void connection_build(net_connection *conn, void *args) {
        report_status(conn, args);
}

int main() {

    event_loop loop;

    tcp_client client(&loop, "127.0.0.1", 7778, "test_reportCient");

    // 设置Hook函数,建立成功立即执行
    client.set_conn_start(connection_build);

    loop.event_process();

    return 0;
}