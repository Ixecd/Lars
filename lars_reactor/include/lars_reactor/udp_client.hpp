#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <lars_reactor/net_connection.hpp>
#include <lars_reactor/event_loop.hpp>
#include <lars_reactor/message.hpp>

namespace qc {

class udp_client : public net_connection {
public:
    udp_client(event_loop *loop, const char *ip, uint16_t port);

    ~udp_client();

    int get_fd() override;

    void add_msg_router(int msgid, msg_callback cb, void *user_data = nullptr);

    int send_message(const char *data, int msglen, int msgid) override;

    void do_read();

private:
    /// @brief 通信socket
    int _sockfd;
    /// @brief 每个连接都有自己的loop
    event_loop* _loop;
    /// @brief buffer
    char _read_buf[MESSAGE_LENGTH_LIMIT];
    char _write_buf[MESSAGE_LENGTH_LIMIT];

    msg_router _router;
};


}