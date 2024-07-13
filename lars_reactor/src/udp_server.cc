/**
 * @file udp_server.cc
 * @author qc
 * @brief udp通信实现(简单版单线程)
 * @version 0.1
 * @date 2024-05-07
 *
 * @copyright Copyright (c) 2024
 *
 */

// #include "udp_server.hpp"
#include <lars_reactor/udp_server.hpp>

#include <signal.h>
#include <string.h>
#include <strings.h>

// #include "qc.hpp"
#include <lars_reactor/qc.hpp>

namespace qc {

void read_server_callback(event_loop *loop, int fd, void *args) {
    udp_server *server = (udp_server *)args;
    server->do_read();
}

int udp_server::get_fd() {
    return _sockfd;
}

void udp_server::do_read() {
    while (1) {
        /// @brief 这里_client_addr 和 _client_addr_len 存储的是发送发的信息
        int pkg_len =
            recvfrom(_sockfd, _read_buf, sizeof(_read_buf), 0,
                     (struct sockaddr *)&_client_addr, &_client_addrlen);
        if (pkg_len == -1)
            if (errno == EAGAIN)
                break;
            else if (errno == EINTR)
                continue;
            else {
                perror("recvfrom err\n");
                break;
            }
        // 处理数据
        msg_head head;
        // 这里直接Memcpy来判断
        memcpy(&head, _read_buf, MESSAGE_HEAD_LEN);

        if (head.msglen > MESSAGE_LENGTH_LIMIT || head.msglen < 0 ||
            head.msglen + MESSAGE_HEAD_LEN != pkg_len) {
            // 包有问题
            fprintf(stderr,
                    "do_read(), data error, msgid = %d, msglen = %d, pkg_len = "
                    "%d\n",
                    head.msgid, head.msglen, pkg_len);
            continue;
        }

        // 没有问题调用相应的读回调函数
        _router.call(head.msgid, head.msglen, _read_buf + MESSAGE_HEAD_LEN,
                     this);
    }
}

udp_server::udp_server(event_loop *loop, const char *ip, uint16_t port) {
    // 1. 忽略一些信号
    qc_assert(signal(SIGHUP, SIG_IGN) != SIG_ERR);
    qc_assert(signal(SIGPIPE, SIG_IGN) != SIG_ERR);

    // 2. 创建套接字
    // AF_INET + SOCK_DGRAM 后面可以直接为0 系统默认为UDP
    // SOCK_DGRAM -> 数据报的形式传输
    // SOCK_CLOEXEC -> 在执行exec函数族的时候关闭套接字
    // 避免在派生的子进程中意外地继承套接字，从而导致资源泄漏或安全风险。
    _sockfd =
        socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);

    // 3. 设置服务ip + port
    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_aton(ip, &serveraddr.sin_addr);  // 设置ip地址
    serveraddr.sin_port = htons(port);

    // 4. 绑定
    bind(_sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr));

    // 5. 添加业务事件
    _loop = loop;

    // 6.清理对应的client
    bzero(&_client_addr, sizeof(_client_addr));
    _client_addrlen = sizeof(_client_addr);

    printf("server on %s:%u is running...\n", ip, port);

    _loop->add_io_event(_sockfd, read_server_callback, EPOLLIN, this);
}

int udp_server::send_message(const char *data, int msglen, int msgid) {
    // 封装成msg_head 格式
    qc_assert(msglen < MESSAGE_LENGTH_LIMIT);

    msg_head head;
    head.msgid = msgid;
    head.msglen = msglen;

    memcpy(_write_buf, &head, MESSAGE_HEAD_LEN);
    memcpy(_write_buf + MESSAGE_HEAD_LEN, data, msglen);

    int rt = sendto(_sockfd, _write_buf, msglen + MESSAGE_HEAD_LEN, 0,
                    (struct sockaddr *)&_client_addr, _client_addrlen);

    qc_assert(rt != -1);

    return rt;
}
/// @brief 这里需要用户自己注册,调用自己的回调函数
void udp_server::add_msg_router(int msgid, msg_callback cb, void *user_data) {
    _router.register_msg_router(msgid, cb, user_data);
}

udp_server::~udp_server() {
    _loop->del_io_event(_sockfd);
    close(_sockfd);
}

}  // namespace qc