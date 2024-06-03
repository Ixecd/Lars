#include "tcp_client.hpp"
#include <stdio.h>
#include <string.h>


using namespace qc;

// 客户端回调函数
void busi(const char *data, uint32_t len, int msgid, net_connection *conn, void *args) {
    // get data from server
    printf("[client] get data : %s\n", data);
    printf("[msgid] = [%d]\n", msgid);
    printf("[len] = [%d]\n", len);
}

// --- hook ---
void on_client_build(net_connection *conn, void *args) {
    printf("[client] : hook open!\n");

    int msgid = 1;
    const char *msg = "Hello, lars!";
    conn->send_message(msg, strlen(msg), msgid);

}

void on_client_lost(net_connection *conn, void *args) {
    printf("[client] : hook close!\n");

    printf("[client] : lost!\n");

}

int main() {

    event_loop loop;

    tcp_client client(&loop, "127.0.0.1", 7777, "lars_reactor v0.6");

    // 注册路由回调函数
    client._router.register_msg_router(1, busi);
    client._router.register_msg_router(101, busi);

    // 设置hook函数
    client.set_conn_start(on_client_build);
    client.set_conn_close(on_client_lost);

    loop.event_process();

    return 0;
}