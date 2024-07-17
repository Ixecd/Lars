/**
 * @file udp_server.hpp
 * @author qc
 * @brief 简单处理,单线程
 * @details UDP是一种无连接的协议，每个数据包都是独立的，所以在UDP通信中需要保存对方的地址信息才能进行通信
 * @version 0.1
 * @date 2024-05-07
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <lars_reactor/event_loop.hpp>
#include <lars_reactor/net_connection.hpp>
#include <lars_reactor/message.hpp>

namespace qc {

class udp_server : public net_connection {
public:
    udp_server(event_loop *loop, const char *ip, uint16_t port);
    
    int send_message(const char *data, int msglen, int msgid) override;

    int get_fd() override;

    //注册路由回调函数
    void add_msg_router(int msgid, msg_callback cb, void* user_data = nullptr);

    ~udp_server();

    //处理业务
    void do_read();
private:
    /// @brief 通信文件描述符
    int _sockfd;

    char _read_buf[MESSAGE_LENGTH_LIMIT];
    char _write_buf[MESSAGE_LENGTH_LIMIT];

    event_loop *_loop;

    /// @brief 绑定的服务端的地址
    struct sockaddr_in _client_addr;
    socklen_t _client_addrlen;
    /// @brief 消息路由分发
    msg_router _router;

};




}