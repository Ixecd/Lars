/**
 * @file tcp_client.hpp
 * @author qc
 * @brief tcp客户端封装
 * @version 0.1
 * @date 2024-04-29
 *
 * @copyright Copyright (c) 2024
 *
 */

#ifndef __TCP_CLIENT_HPP__
#define __TCP_CLIENT_HPP__

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <functional>

#include "event_loop.hpp"
#include "io_buf.hpp"
#include "message.hpp"

namespace qc {

/**
 * @brief 封装Tcp连接客户端信息
 * @details 单纯为写客户端提供的接口
 */
class tcp_client {
public:

    /// @brief 完成基本的socket初始化和connect操作
    tcp_client(event_loop *loop, const char *ip, uint16_t prot,
               const char *name);
    /// @brief 发送消息
    int send_message(const char *data, int msglen, int msgid);
    /// @brief 创建连接
    void do_connect();
    /// @brief 处理链接的读事件
    int do_read();
    /// @brief 处理链接的写事件
    int do_write();
    /// @brief 清空连接资源
    void clean_conn();
    /// @brief 获取通信socket
    int get_socket() { return _sockfd; }
    /// @brief 获取event_loop
    event_loop *get_event_loop() { return _loop; }
    /// @brief 断开连接
    ~tcp_client();


    void set_msg_callback(msg_callback msg_cb) { this->_msg_callback = msg_cb; }

public:
    /// @brief 读buff
    io_buf _ibuf;
    /// @brief 写buff
    io_buf _obuf;
    /// @brief 当前是否连接
    bool connected;
    /// @brief 服务端地址
    struct sockaddr_in _server_addr;

private:
    /// @brief 通信socket
    int _sockfd;
    /// @brief socket 长度
    socklen_t _addrlen;
    /// @brief 管理当前socket的epoll实例
    event_loop *_loop;
    /// @brief 客户端名称
    const char *_name;
    /// @brief 信息处理回调函数
    msg_callback _msg_callback;
};

}  // namespace qc

#endif