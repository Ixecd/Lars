#include <string.h>
#include <strings.h>

#include <lars_reactor/config_file.hpp>
#include <lars_reactor/net_connection.hpp>
#include <lars_reactor/qc.hpp>
#include <lars_reactor/tcp_server.hpp>

using namespace qc;

tcp_server *server;

void callback_busi(const char *data, int len, int msgid, net_connection *conn, void *user_data)  {
    printf("callback_busi running...\n");
    // 回显给服务端
    conn->send_message(data, len, msgid);

    // 获得客户端传过来的param
    printf("conn param = %s\n", (const char *)conn->param);
}

void print_busi(const char *data, int len, int msgid, net_connection *conn, void *user_data) {
    printf("[server] get data : %s\n", data);
    printf("[server] get msgid : %d\n", msgid);
    printf("[server] get msglen = %d\n", len);
}


// 新客户创建的回调
void on_client_build(net_connection *conn, void *user_data) {
    int msgid = 101;
    const char *msg = "welcome ! you are online...";

    conn->send_message(msg, strlen(msg), msgid);

    // 将当前的net_connection绑定一些参数
    const char *conn_param_test = "I am a param for you";
    conn->param = (void *)conn_param_test;
}

// 客户端销毁链接
void on_client_lost(net_connection *conn, void *user_data) {
    printf("connection is lost!\n");
}


int main() {
    event_loop loop;
    // 配置路径
    config_file_instance::GetInstance()->setPath("/home/qc/Lars/lars_reactor/example/lars_reactor_1.0/serv.conf");

    // config_file_instance::GetInstance()->get_all_info(config_file_instance::GetInstance());

    std::string ip =
        config_file_instance::GetInstance()->GetString("reactor", "ip", "0.0.0.0");

    int port = config_file_instance::GetInstance()->GetNumber("reactor", "port", 9876);

    printf("server ip = %s, port = %d\n", ip.c_str(), port);

    tcp_server server(&loop, ip.c_str(), port);

    // 注册路由信息
    server.add_msg_router(1, callback_busi);
    server.add_msg_router(2, print_busi);

    // 建立Hook信息
    server.set_conn_start(on_client_build);
    server.set_conn_close(on_client_lost);

    loop.event_process();
    return 0;
}