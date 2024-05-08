/**
 * @file udp_client.cc
 * @author qc
 * @brief udp_client 封装
 * @version 0.1
 * @date 2024-05-07
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "udp_client.hpp"
#include <strings.h>
#include <string.h>

namespace qc {

void read_callback(event_loop *loop, int fd, void *args) {
    udp_client *client = (udp_client*)args;
    client->do_read();
}



udp_client::udp_client(event_loop *loop, const char *ip, uint16_t port) {
    // 1. socket
    _sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
    qc_assert(_sockfd != -1);

    struct sockaddr_in serveraddr;
    bzero(&serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    inet_aton(ip, &serveraddr.sin_addr);
    serveraddr.sin_port = htons(port);
    // 2. 链接
    int rt = connect(_sockfd, (const struct sockaddr *)&serveraddr, sizeof(serveraddr));
    qc_assert(rt != -1);

    // 3. 添加io事件
    _loop = loop;
    _loop->add_io_event(_sockfd, read_callback, EPOLLIN, this);
}

udp_client::~udp_client() {
    _loop->del_io_event(_sockfd);
    close(_sockfd);
}

int udp_client::send_message(const char *data, int msglen, int msgid) {
    // 封装成msg_head 格式
    qc_assert(msglen < MESSAGE_LENGTH_LIMIT);

    msg_head head;
    head.msgid = msgid;
    head.msglen = msglen;

    memcpy(_write_buf, &head, MESSAGE_HEAD_LEN);
    memcpy(_write_buf + MESSAGE_HEAD_LEN, data, msglen);

    int rt = sendto(_sockfd, _write_buf, msglen + MESSAGE_HEAD_LEN, 0,
                    nullptr, 0);

    qc_assert(rt != -1);

    return rt;
}

void udp_client::add_msg_router(int msgid, msg_callback cb, void *user_data) {
    _router.register_msg_router(msgid, cb, user_data);
}

void udp_client::do_read() {
    while (1) {
        int pkg_len =
            recvfrom(_sockfd, _read_buf, sizeof(_read_buf), 0,
                     nullptr, nullptr);
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

}