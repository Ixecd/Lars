#include "tcp_server.hpp"

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "qc.hpp"

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
            int writed;
            char* data = "Hello Lars\n";
            /// @brief 如果writed err
            /// 并且错误码为EINTR表示被信号中断,此时应该继续写,其他情况直接退出
            do {
                writed = write(connfd, data, strlen(data) + 1);
            } while (writed == -1 && errno == EINTR);

            if (writed > 0) printf("write success!\n");
            // socket -> NONBLOCK
            if (writed == -1 && errno == EAGAIN) {
                writed = 0;
            }
        }
    }
}

tcp_server::~tcp_server() { close(_sockfd); }
}  // namespace qc