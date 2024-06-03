#include <string.h>

#include "tcp_server.hpp"

using namespace qc;

void callback_busi(const char *data, uint32_t len, int msgid,
                   net_connection *conn, void *user_data) {
    printf("callback_busi...\n");
    conn->send_message(data, len, msgid);
}

void print_busi(const char *data, uint32_t len, int msgid, net_connection *conn,
                void *user_data) {
    printf("[server] get: [%s]\n", data);
    printf("[msgid] = [%d]\n", msgid);
    printf("[len] = [%d]\n", len);
}

// --- Hook ---
void on_client_build(net_connection *conn, void *args) {
    int msgid = 101;
    const char *msg = "welcome! you online...\n";

    conn->send_message(msg, strlen(msg), msgid);
}

void on_client_lost(net_connection *conn, void *args) {
    printf("connection is lost...\n");
}

int main() {
    event_loop loop;

    tcp_server server(&loop, "127.0.0.1", 7777);

    // 注册消息
    tcp_server::router.register_msg_router(1, callback_busi);
    tcp_server::router.register_msg_router(2, print_busi);


    // 注册链接hook回调
    tcp_server::set_conn_start(on_client_build);
    tcp_server::set_conn_close(on_client_lost);

    loop.event_process();
    return 0;
}
