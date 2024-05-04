#include "tcp_server.hpp"

using namespace qc;

msg_callback callback_busi(const char *data, uint32_t len, int msgid,
                           net_connection *conn, void *user_data) {
    printf("callback_busi ...\n");
    //直接回显
    conn->send_message(data, len, msgid);
    return nullptr;
}

msg_callback print_busi(const char *data, uint32_t len, int msgid,
                        net_connection *conn, void *user_data) {
    printf("recv client: [%s]\n", data);
    printf("msgid: [%d]\n", msgid);
    printf("len: [%d]\n", len);
    return nullptr;
}

int main() {
    event_loop loop;

    tcp_server server(&loop, "127.0.0.1", 7777);

    //server.add_msg_router(1, callback_busi);

    //server.add_msg_router(2, print_busi);

    loop.event_process();

    return 0;
}