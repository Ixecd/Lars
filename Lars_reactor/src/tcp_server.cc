#include "tcp_server.hpp"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qc.hpp"
#include "reactor_buf.hpp"

namespace qc {

tcp_server::tcp_server(const char* ip, uint16_t port) {
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
    if (listen(_sockfd, 500) == -1) {
        fprintf(stderr, "listen error\n");
        exit(1);
    }
}

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

            int ret = 0;
            input_buf ibuf;
            output_buf obuf;

            char* msg = NULL;
            int msg_len = 0;
            do {
                ret = ibuf.read_data(connfd);
                if (ret == -1) {
                    fprintf(stderr, "ibuf read_data error\n");
                    break;
                }
                printf("ibuf.length() = %d\n", ibuf.length());

                //将读到的数据放在msg中
                msg_len = ibuf.length();
                msg = (char*)malloc(msg_len);
                bzero(msg, msg_len);
                memcpy(msg, ibuf.data(), msg_len);
                ibuf.pop(msg_len);
                ibuf.adjust();
                
                qc_assert(msg);
                
                printf("recv data = %s\n", msg);

                //回显数据
                obuf.send_data(msg, msg_len);
                while (obuf.length()) {
                    int write_ret = obuf.write2fd(connfd);
                    if (write_ret == -1) {
                        fprintf(stderr, "write connfd error\n");
                        return;
                    } else if (write_ret == 0) {
                        //不是错误，表示此时不可写
                        break;
                    }
                }

                free(msg);

            } while (ret != 0);

            // Peer is closed
            close(connfd);
        }
    }
}

tcp_server::~tcp_server() { close(_sockfd); }
}  // namespace qc