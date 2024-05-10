#include <string.h>
#include <strings.h>

#include "net_connection.hpp"
#include "qc.hpp"
#include "tcp_client.hpp"

using namespace qc;

// 客户端业务
void busi(const char *data, int len, int msgid, net_connection *conn,
          void *user_data) {
    printf("call client busi...\n");

    char *str = nullptr;
    str = (char *)malloc(len);
    memset(str, 0, len + 1);
    memcpy(str, data, len);
    printf("[client] get data : [%s]\n", str);
    printf("[client] get len : [%d]\n", len);
    printf("[client] get msgid : [%d]\n", msgid);
}

void on_client_build(net_connection *conn, void *user_data) {
    printf("client_building...\n");
    printf("conn->param = [%s]\n", (char *)conn->param);
    int msgid = 1;
    const char *data = "Hello, Lars!";
    conn->send_message(data, strlen(data), msgid);
}

void on_client_lost(net_connection *conn, void *user_data) {
    printf("client_lost...\n");
    printf("lost conn->param = [%s]\n", (char *)conn->param);
    

}

int main() {
    event_loop loop;

    tcp_client client(&loop, "127.0.0.1", 7777, "lars_reactor_v1.0");

    // 注册路由
    client.add_msg_router(1, busi);
    client.add_msg_router(101, busi);

    // 注册HOOK
    const char *param = "param_build";
    client.set_conn_start(on_client_build, (void *)param);
    const char *param2 = "param_lost";
    client.set_conn_close(on_client_lost, (void *)param2);

    loop.event_process();

    return 0;
}