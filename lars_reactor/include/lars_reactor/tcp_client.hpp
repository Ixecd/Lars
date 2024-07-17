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


#include <lars_reactor/io_buf.hpp>
#include <lars_reactor/message.hpp>
#include <lars_reactor/event_loop.hpp>
#include <lars_reactor/net_connection.hpp>

namespace qc {

/**
 * @brief 封装Tcp连接客户端信息
 * @details 单纯为写客户端提供的接口
 */
class tcp_client : public net_connection {
public:
    /// @brief 完成基本的socket初始化和connect操作
    tcp_client(event_loop *loop, const char *ip, uint16_t prot,
               const char *name);
    /// @brief 发送消息
    int send_message(const char *data, int msglen, int msgid) override;
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

    // 业务处理回调函数,有了消息路由就不需要了

    // void set_msg_callback(msg_callback msg_cb) { this->_msg_callback =
    // msg_cb; }
    void add_msg_router(int msgid, msg_callback cb, void *user_data = nullptr) {
        _router.register_msg_router(msgid, cb, user_data);
    }

public:
    // 处理消息的分发路由
    msg_router _router;

public:
    // --------- Hook ---------
    void set_conn_start(conn_callback cb, void *args = nullptr) {
        _conn_start_cb = cb;
        _conn_start_cb_args = args;
    }

    void set_conn_close(conn_callback cb, void *args = nullptr) {
        _conn_close_cb = cb;
        _conn_close_cb_args = args;
    }

    conn_callback _conn_start_cb;
    void *_conn_start_cb_args;

    conn_callback _conn_close_cb;
    void *_conn_close_cb_args;

public:
    /// @brief 获取当前通信描述符
    int get_fd() { return _sockfd; }

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