#include <string.h>

#include <string>

#include "config_file.hpp"
#include "tcp_server.hpp"

using namespace qc;

tcp_server *server;

void print_lars_callback(event_loop *loop, void *args) {
    printf("======== Active Task Running ========\n");
    listen_fd_set fds;
    loop->get_listen_fds(fds);

    // 开始遍历fds
    for (auto it = fds.begin(); it != fds.end(); ++it) {
        int fd = *it;
        tcp_conn *conn = tcp_server::conns[fd];
        if (conn != nullptr) {
            int msgid = 101;
            const char *data = "Hello, here is a task!";
            conn->send_message(data, strlen(data), msgid);
        }
    }
}

//回显业务的回调函数
void callback_busi(const char *data, uint32_t len, int msgid,
                   net_connection *conn, void *user_data) {
    printf("callback_busi ...\n");
    //直接回显
    conn->send_message(data, len, msgid);
}

//打印信息回调函数
void print_busi(const char *data, uint32_t len, int msgid, net_connection *conn,
                void *user_data) {
    printf("recv client: [%s]\n", data);
    printf("msgid: [%d]\n", msgid);
    printf("len: [%d]\n", len);
}

//新客户端创建的回调
void on_client_build(net_connection *conn, void *args) {
    int msgid = 101;
    const char *msg = "welcome! you online..";

    conn->send_message(msg, strlen(msg), msgid);

    // 触发任务
    thread_pool *cur_pool = server->get_thread_pool();
    qc_assert(cur_pool != nullptr);
    cur_pool->send_task(print_lars_callback);
}

//客户端销毁的回调
void on_client_lost(net_connection *conn, void *args) {
    printf("connection is lost !\n");
}

int main() {
    event_loop loop;

    //加载配置1文件
    config_file::setPath("./serv.conf");
    std::string ip =
        config_file::GetInstance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::GetInstance()->GetNumber("reactor", "port", 9876);

    printf("ip = %s, port = %d\n", ip.c_str(), port);

    server = new tcp_server(&loop, ip.c_str(), port);
    // tcp_server server(&loop, ip.c_str(), port);

    //注册消息业务路由
    server->add_msg_router(1, callback_busi);
    server->add_msg_router(2, print_busi);

    //在任务队列中添加任务
    //server.get_thread_pool()->send_task(print_lars_callback, &loop);

    //注册链接hook回调
    server->set_conn_start(on_client_build);
    server->set_conn_close(on_client_lost);

    loop.event_process();

    return 0;
}

