#include "tcp_server.hpp"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qc.hpp"
#include "reactor_buf.hpp"
#include "tcp_conn.hpp"

namespace qc {

struct message {
    char data[m4K];
    char len;
};

struct message msg;

void server_rd_callback(event_loop *loop, int fd, void *args);
void server_wt_callback(event_loop *loop, int fd, void *args);

/// @brief 这里_sockfd 一旦触发,就会调用对应的do_accept()
/// @param loop 
/// @param fd 
/// @param args 
void accept_callback(event_loop *loop, int fd, void *args) {
    tcp_server *server = (tcp_server*) args;
    server->do_accept();
}
/**
 * @brief Construct a new tcp server::tcp server object
 * @details 一个TCP_SERVER对应一个服务端,一个服务端对应一个epoll,
 * @param loop 
 * @param ip 服务端的ip
 * @param port 服务端的port
 */
tcp_server::tcp_server(event_loop *loop, const char* ip, uint16_t port) {
    bzero(&_connaddr, sizeof(_connaddr));
    /**
     * SIGPIPE: 如果客户端关闭,服务端再次write就会产生
     * SIGHUP:如果终端关闭,就会给当前进程发送该信号
     * 前提应该忽略这两个signal
     * 前面是要处理的信号,后面是动作
     * signal(int signum, sighandler_t handler);
     */
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR)
        fprintf(stderr, "signal ignore SIGHUP\n");
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        fprintf(stderr, "signal ignore SIGPIPE\n");

    // -------------1. 创建socket -------------
    /// @brief
    /// 创建socket,可以设置为非阻塞(这里要设置为非阻塞,要不然accept的时候会返回EAGAIN)
    /// | SOCK_NONBLOCK
    /// @param[in] IPPROTO_TCP -> IP Protocal TCP 代表IP下的TCP
    _sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, IPPROTO_TCP);

    qc_assert(_sockfd != -1);

    // -------------2. 初始化地址 ---------------
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_aton(ip, &server_addr.sin_addr);  // arpa/inet.h
    server_addr.sin_port = htons(port);    // host to net

    // 端口复用
    int op = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0)
        fprintf(stderr, "setsocketopt SO_REUSERADDR\n");

    // -------------3. 绑定 ---------------
    if (bind(_sockfd, (const struct sockaddr*)&server_addr,
             sizeof(server_addr)) < 0) {
        fprintf(stderr, "bind error\n");
        exit(1);
    }

    // -------------4. 监听 ---------------
    // listen
    qc_assert(listen(_sockfd, 500) != -1);

    // ------------- V0.2 ----------------
    // 将_sockfd 添加到event_loop中
    _loop = loop;
    _loop->add_io_event(_sockfd, accept_callback, EPOLLIN, this);
}


/// @brief V0.1的时候需要手动进行 do_accept,加入epoll之后就不需要显示调用这个函数
void tcp_server::do_accept() {
    int connfd;
    while (1) {
        printf("begin accept\n");
        connfd = accept(_sockfd, (struct sockaddr*)&_connaddr, &_addrlen);
        if (connfd == -1) {
            if (errno == EINTR) {
                fprintf(stderr, "accept errno=EINTR\n");
                continue;
            } else if (errno == EMFILE) {
                // 资源不够
                fprintf(stderr, "accept errno=EMFILE\n");
            } else if (errno == EAGAIN) {
                fprintf(stderr, "accept errno=EAGAIN\n");
                break;
            } else {
                fprintf(stderr, "accept error\n");
                exit(0);
            }
        } else {
            // accept succ!

            // int ret = 0;
            // input_buf ibuf;
            // output_buf obuf;

            // char* msg = NULL;
            // int msg_len = 0;
            // do {
            //     ret = ibuf.read_data(connfd);
            //     qc_assert(ret != -1);

            //     //将读到的数据放在msg中
            //     msg_len = ibuf.length();
            //     msg = (char*)malloc(msg_len);
            //     bzero(msg, msg_len);
            //     //printf("ibuf.data() = %s\n", ibuf.data());

            //     memcpy(msg, ibuf.data(), msg_len);
            //     //printf("recv data = %s\n", msg);

            //     ibuf.pop(msg_len);
            //     // printf("pop success\n");
                
            //     ibuf.adjust();
            //     // printf("adjust success\n");

            //     qc_assert(msg);
                
            //     printf("recv data = %s\n", msg);

            //     //回显数据
            //     obuf.send_data(msg, msg_len);
            //     while (obuf.length()) {
            //         int write_ret = obuf.write2fd(connfd);
            //         if (write_ret == -1) {
            //             fprintf(stderr, "write connfd error\n");
            //             return;
            //         } else if (write_ret == 0) {
            //             //不是错误，表示此时不可写
            //             break;
            //         }
            //     }

            //     free(msg);

            // } while (ret != 0);

            // Peer is closed
            // close(connfd);

            // --------- V0.2 -------------
            this->_loop->add_io_event(connfd, server_rd_callback, EPOLLIN, &msg);
            break;

            // --------- V0.3 -------------
            tcp_conn *conn = new tcp_conn(connfd, _loop);
            qc_assert(conn);
            
            printf("get new connection succ!\n");
            break;
        }
    }
}


/// @brief 服务端的读回调函数
void server_rd_callback(event_loop *loop, int fd, void *args) {
    int rt = 0;
    struct message *msg = (struct message*) args;
    input_buf ibuf;

    rt = ibuf.read_data(fd);
    if (rt == -1) {
        fprintf(stderr, "ibuf read_data error\n");
        loop->del_io_event(fd);
        close(fd);
        return;
    } else if (rt == 0) {
        loop->del_io_event(fd);
        close(fd);
        return;
    }

    printf("ibuf.length() = %d\n", ibuf.length());

    //将读到的数据放在msg中
    msg->len = ibuf.length();
    bzero(msg->data, msg->len);
    memcpy(msg->data, ibuf.data(), msg->len);

    ibuf.pop(msg->len);
    ibuf.adjust();

    printf("recv data = %s\n", msg->data);

    // 这个时候服务端已经读取到了来自客户端的信息,删除对应的事件,注册写回调事件
    // 删除读事件,添加写事件
    loop->del_io_event(fd, EPOLLIN);
    loop->add_io_event(fd, server_wt_callback, EPOLLOUT, msg);
}

/// @brief 服务端的写回调函数
void server_wt_callback(event_loop *loop, int fd, void *args) {
    struct message *msg = (struct message *)args;
    output_buf obuf;

    // 回显数据
    obuf.send_data(msg->data, msg->len);
    while (obuf.length()) {
        int write_ret = obuf.write2fd(fd);
        qc_assert(write_ret != -1);
        if (write_ret == 0) break; // 不是错误
    }

    // 删除写事件
    loop->del_io_event(fd, EPOLLOUT);
    loop->add_io_event(fd, server_rd_callback, EPOLLIN, msg);

}

tcp_server::~tcp_server() { close(_sockfd); }
}  // namespace qc