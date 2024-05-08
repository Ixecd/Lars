/**
 * @file udp_client.hpp
 * @author qc
 * @brief udp通讯 - 客户端实现
 * @version 0.1
 * @date 2024-05-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include "net_connection.hpp"
#include "message.hpp"
#include "event_loop.hpp"

namespace qc {

class udp_client : public net_connection {
public:
    udp_client(event_loop *loop, const char *ip, uint16_t port);

    ~udp_client();

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