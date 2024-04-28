/**
 * @file tcp_conn.cc
 * @author qc
 * @brief tcp_conn 封装类
 * @version 0.1
 * @date 2024-04-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/tcp.h>    // for TCP_NODELAY
#include <netinet/in.h>     // for IPPROTO_TCP
#include "tcp_conn.hpp"
#include "message.hpp"
namespace qc {

/// @brief 回显业务
void callback_busi(const char *data, uint32_t len, int msgid, void *args, tcp_conn *conn) {
    conn->send_message(data, len, msgid);
}

static void conn_rd_callback(event_loop *loop, int fd, void *args) {
    tcp_conn *conn = (tcp_conn*) args;
    conn->do_read();
}

static void conn_wt_callback(event_loop *loop, int fd, void *args) {
    tcp_conn *conn = (tcp_conn *)args;
    conn->do_write();
}

// 初始化tcp_conn
tcp_conn::tcp_conn(int connfd, event_loop *loop) {
    _connfd = connfd;
    _loop = loop;
    // 设置connfd 为非阻塞
    qc_assert(qc::SetNonblocking(_connfd) == 1);

    // 设置TCP_NODELAY 禁止做读写缓存,降低小包延迟
    int op = 1;
    setsockopt(_connfd, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));

    //3.将该链接的读事件让epoll检测
    _loop->add_io_event(_connfd, conn_rd_callback, EPOLLIN, this);

    //4.集成到tcp_server中
    // TODO

}


}