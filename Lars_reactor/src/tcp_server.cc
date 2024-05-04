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

// ========链接资源管理=========
tcp_conn **tcp_server::conns = nullptr;

int tcp_server::_max_conns = 0;

int tcp_server::_curr_conns = 0;

msg_router tcp_server::router;

    // 静态初始化互斥锁
    pthread_mutex_t tcp_server::_conns_mutex = PTHREAD_MUTEX_INITIALIZER;
// 动态初始化互斥锁 -->
// 先声明一个pthread_mutex_t类型的互斥锁,之后再pthread_mutex_init();

// 添加一个conn
void tcp_server::increase_conn(int connfd, tcp_conn *conn) {
    pthread_mutex_lock(&_conns_mutex);
    conns[connfd] = conn;
    _curr_conns++;
    pthread_mutex_unlock(&_conns_mutex);
}

// 删除一个conn
void tcp_server::decrease_conn(int connfd) {
    pthread_mutex_lock(&_conns_mutex);
    conns[connfd] = nullptr;
    --_curr_conns;
    pthread_mutex_unlock(&_conns_mutex);
}

/// @brief 获取当前所有链接的数量
/// @param[out] curr_conn 数量
void tcp_server::get_conn_num(int *curr_conn) {
    pthread_mutex_lock(&_conns_mutex);
    *curr_conn = _curr_conns;
    pthread_mutex_unlock(&_conns_mutex);
}

// listen fd 客户端有新链接请求过来的回调函数
void accept_callback(event_loop *loop, int fd, void *args) {
    tcp_server *server = (tcp_server *)args;
    server->do_accept();
}

// server的构造函数
tcp_server::tcp_server(event_loop *loop, const char *ip, uint16_t port) {
    bzero(&_connaddr, sizeof(_connaddr));

    //忽略一些信号 SIGHUP, SIGPIPE
    // SIGPIPE:如果客户端关闭，服务端再次write就会产生
    // SIGHUP:如果terminal关闭，会给当前进程发送该信号
    if (signal(SIGHUP, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "signal ignore SIGHUP\n");
    }
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
        fprintf(stderr, "signal ignore SIGPIPE\n");
    }

    // 1. 创建socket
    _sockfd = socket(AF_INET, SOCK_STREAM | /*SOCK_NONBLOCK |*/ SOCK_CLOEXEC,
                     IPPROTO_TCP);
    if (_sockfd == -1) {
        fprintf(stderr, "tcp_server::socket()\n");
        exit(1);
    }

    // 2 初始化地址
    struct sockaddr_in server_addr;
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    inet_aton(ip, &server_addr.sin_addr);
    server_addr.sin_port = htons(port);

    // 2-1可以多次监听，设置REUSE属性
    int op = 1;
    if (setsockopt(_sockfd, SOL_SOCKET, SO_REUSEADDR, &op, sizeof(op)) < 0) {
        fprintf(stderr, "setsocketopt SO_REUSEADDR\n");
    }

    // 3 绑定端口
    if (bind(_sockfd, (const struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        fprintf(stderr, "bind error\n");
        exit(1);
    }

    // 4 监听ip端口
    if (listen(_sockfd, 500) == -1) {
        fprintf(stderr, "listen error\n");
        exit(1);
    }

    // 5 将_sockfd添加到event_loop中
    _loop = loop;

    // 6 ========== 链接管理 ===========
    _max_conns = MAX_CONNS;
    // 这里动态的创建链接信息数组, +3 是因为stdin, stdout, stderr已经被占用,fd 一定是从3开始
    conns = new tcp_conn*[_max_conns + 3]; 
    qc_assert(conns);
    // printf("cour _max_conns = %d\n", _max_conns);

    // 7 创建线程池

    // 8 注册_socket读事件-->accept处理
    _loop->add_io_event(_sockfd, accept_callback, EPOLLIN, this);
}

//开始提供创建链接服务
// accept 之后会将其封装成tcp_conn类
void tcp_server::do_accept() {
    int connfd;
    while (true) {
        // accept与客户端创建链接
        printf("begin accept\n");
        connfd = accept(_sockfd, (struct sockaddr *)&_connaddr, &_addrlen);
        if (connfd == -1) {
            if (errno == EINTR) {
                fprintf(stderr, "accept errno=EINTR\n");
                continue;
            } else if (errno == EMFILE) {
                //建立链接过多，资源不够
                fprintf(stderr, "accept errno=EMFILE\n");
            } else if (errno == EAGAIN) {
                fprintf(stderr, "accept errno=EAGAIN\n");
                break;
            } else {
                fprintf(stderr, "accept error\n");
                exit(1);
            }
        } else {
            // accept succ!

            // ======== 加入链接管理 ========
            int cur_conns;
            get_conn_num(&cur_conns);
            // printf("after get_conn_num, the num = %d\n", cur_conns);
            if (cur_conns >= _max_conns) {
                fprintf(stderr, "so many connections, max = %d\n", _max_conns);
                close(connfd);
            } else {
                //启动单线程模式
                tcp_conn *conn = new tcp_conn(connfd, _loop);
                if (conn == NULL) {
                    fprintf(stderr, "new tcp_conn error\n");
                    exit(1);
                }
                // 这里是否应该将其加入到数组中? 不需要,要降低耦合(设计到锁),在tcp_conn中increase_conn即可
                //conns[connfd] = conn;
                printf("[tcp_server]: get new connection succ!\n");
            }
            break;
        }
    }
}

//链接对象释放的析构
tcp_server::~tcp_server() {
    //将全部的事件删除
    _loop->del_io_event(_sockfd);
    //关闭套接字
    close(_sockfd);
}



}  // namespace qc