/**
 * @file tcp_client.cc
 * @author qc
 * @brief tcp连接客户端的封装
 * @version 0.1
 * @date 2024-04-29
 *
 * @copyright Copyright (c) 2024
 *
 */

// #include "tcp_client.hpp"
// #include "tcp_server.hpp"
#include <lars_reactor/tcp_client.hpp>
#include <lars_reactor/tcp_server.hpp>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

// #include "qc.hpp"
#include <lars_reactor/qc.hpp>
namespace qc {


static void write_callback(event_loop *loop, int fd, void *args) {
    ((tcp_client *)args)->do_write();
}

static void read_callback(event_loop *loop, int fd, void *args) {
    ((tcp_client *)args)->do_read();
}

tcp_client::tcp_client(event_loop *loop, const char *ip, unsigned short port,
                       const char *name)
    : _obuf(4194304), _ibuf(4194304) {
    _sockfd = -1;
    _name = name;
    _loop = loop;

    bzero(&_server_addr, sizeof(_server_addr));

    _server_addr.sin_family = AF_INET;
    inet_aton(ip, &_server_addr.sin_addr);
    _server_addr.sin_port = htons(port);

    _addrlen = sizeof(_server_addr);

    this->do_connect();
}


//判断链接是否是创建链接，主要是针对非阻塞socket 返回EINPROGRESS错误
static void connection_delay(event_loop *loop, int fd, void *args) {
    tcp_client *cli = (tcp_client *)args;
    
    // printf("cur fd = %d\n", fd);

    // fd = cli->get_socket();
    // loop = cli->get_event_loop();

    // printf("cur fd = %d\n", fd);

    loop->del_io_event(fd);

    int result = 0;
    socklen_t result_len = sizeof(result);
    getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &result_len);
    if (result == 0) {
        //链接是建立成功的
        cli->connected = true;

        printf("connect %s:%d succ!\n", inet_ntoa(cli->_server_addr.sin_addr),
               ntohs(cli->_server_addr.sin_port));

        // --- 调用开发者注册的创建的Hook链接
        if (cli->_conn_start_cb) {
            cli->_conn_start_cb(cli, cli->_conn_start_cb_args);
        }

        // ================ 发送msgid：1 =====
        //建立连接成功之后，主动发送send_message
        //const char *msg = "hello lars!";
        //int msgid = 1;
        //cli->send_message(msg, strlen(msg), msgid);

        // ================ 发送msgid：2 =====
        //const char *msg2 = "hello Aceld!";
        //msgid = 2;
        //cli->send_message(msg2, strlen(msg2), msgid);
        // ================

        /// @brief 连接建立好就检测读事件
        loop->add_io_event(fd, read_callback, EPOLLIN, cli);

        ///
        //printf("before send_message\n");
        ///cli->send_message("hello,lars!", 12, 1);
        //printf("after send_message\n");
        //printf("cur cli->_obuf.m_length = %d\n", cli->_obuf.m_length);
        if (cli->_obuf.m_length != 0) {
            //输出缓冲有数据可写
            loop->add_io_event(fd, write_callback, EPOLLOUT, cli);
        }
    } else {
        //链接创建失败
        fprintf(stderr, "connection %s:%d error\n",
                inet_ntoa(cli->_server_addr.sin_addr),
                ntohs(cli->_server_addr.sin_port));
    }
}

//创建链接
void tcp_client::do_connect() {
    if (_sockfd != -1) {
        close(_sockfd);
    }

    //创建套接字
    _sockfd = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
                     IPPROTO_TCP);
    if (_sockfd == -1) {
        fprintf(stderr, "create tcp client socket error\n");
        exit(1);
    }

    int ret = connect(_sockfd, (const struct sockaddr *)&_server_addr, _addrlen);
    if (ret == 0) {
        //链接创建成功
        connected = true;

        // --- 处理Hook
        if (_conn_start_cb) {
            _conn_start_cb(this, _conn_start_cb_args);
        }


        //注册读回调
        _loop->add_io_event(_sockfd, read_callback, EPOLLIN, this);
        //如果写缓冲去有数据，那么也需要触发写回调
        if (this->_obuf.m_length != 0) {
            _loop->add_io_event(_sockfd, write_callback, EPOLLOUT, this);
        }

        printf("connect %s:%d succ!\n", inet_ntoa(_server_addr.sin_addr),
               ntohs(_server_addr.sin_port));
    } else {
        if (errno == EINPROGRESS) {
            // ======== 判断链接管理 =========

            // fd是非阻塞的，可能会出现这个错误,但是并不表示链接创建失败
            //如果fd是可写状态，则为链接是创建成功的.
            fprintf(stderr, "do_connect EINPROGRESS\n");

            //让event_loop去触发一个创建判断链接业务 用EPOLLOUT事件立刻触发
            // 等待socket可写,就会触发EPOLLOUT事件
            _loop->add_io_event(_sockfd, connection_delay, EPOLLOUT, this);
        } else {
            fprintf(stderr, "connection error\n");
            exit(1);
        }
    }
}

//主动发送message方法
int tcp_client::send_message(const char *data, int msglen, int msgid) {
    if (connected == false) {
        fprintf(stderr, "no connected , send message stop!\n");
        return -1;
    }
    //是否需要添加写事件触发
    //如果当前obuf中有数据，没必要添加，如果没有数据，添加完数据需要触发
    bool need_add_event = (_obuf.m_length == 0) ? true : false;
    if (msglen + MESSAGE_HEAD_LEN > this->_obuf.m_capacity - _obuf.m_length) {
        fprintf(stderr, "No more space to Write socket!\n");
        return -1;
    }

    //封装消息头
    msg_head head;
    head.msgid = msgid;
    head.msglen = msglen;
    // 先是头
    memcpy(_obuf.m_data + _obuf.m_length, &head, MESSAGE_HEAD_LEN);
    _obuf.m_length += MESSAGE_HEAD_LEN;
    // 再是体
    memcpy(_obuf.m_data + _obuf.m_length, data, msglen);
    _obuf.m_length += msglen;

    if (need_add_event) {
        _loop->add_io_event(_sockfd, write_callback, EPOLLOUT, this);
    }

    return 0;
}

//处理读业务
int tcp_client::do_read() {
    //确定已经成功建立连接
    assert(connected == true);
    // 1. 一次性全部读取出来

    //得到缓冲区里有多少字节要被读取，然后将字节数放入b里面。
    int need_read = 0;
    if (ioctl(_sockfd, FIONREAD, &need_read) == -1) {
        fprintf(stderr, "ioctl FIONREAD error");
        return -1;
    }

    //确保_buf可以容纳可读数据
    qc_assert(need_read <= _ibuf.m_capacity - _ibuf.m_length);

    int ret;

    do {
        ret = read(_sockfd, _ibuf.m_data + _ibuf.m_length, need_read);
    } while (ret == -1 && errno == EINTR);

    if (ret == 0) {
        //对端关闭
        if (_name != NULL) {
            printf("%s client: connection close by peer!\n", _name);
        } else {
            printf("client: connection close by peer!\n");
        }

        clean_conn();
        return -1;
    } else if (ret == -1) {
        fprintf(stderr, "client: do_read() , error\n");
        clean_conn();
        return -1;
    }

    assert(ret == need_read);
    _ibuf.m_length += ret;

    // 2. 解包
    msg_head head;
    int msgid, length;
    char *buf;
    while (_ibuf.m_length >= MESSAGE_HEAD_LEN) {
        memcpy(&head, _ibuf.m_data + _ibuf.m_head, MESSAGE_HEAD_LEN);
        msgid = head.msgid;
        length = head.msglen;

        /*
        if (length + MESSAGE_HEAD_LEN < _ibuf.length) {
            break;
        }
        */

        //头部读取完毕
        _ibuf.pop(MESSAGE_HEAD_LEN);

        // 消息路由分发
        this->_router.call(msgid, length, _ibuf.m_data + _ibuf.m_head, this);

        //数据区域处理完毕
        _ibuf.pop(length);
    }
    // 调用业务回调函数
    // this->_msg_callback((const char*)&head, head.msglen, msgid,
    //                    nullptr,nullptr);

    


    //重置head指针
    _ibuf.adjust();

    return 0;
}

//处理写业务
int tcp_client::do_write() {
    // printf("tcp_client::do_write() begin()...\n");
    //数据有长度，切头部索引是起始位置
    qc_assert(_obuf.m_head == 0 && _obuf.m_length);

    int ret;

    while (_obuf.m_length) {
        //写数据
        do {
            ret = write(_sockfd, _obuf.m_data, _obuf.m_length);
        } while (ret == -1 && errno == EINTR);  //非阻塞异常继续重写

        if (ret > 0) {
            _obuf.pop(ret);
            _obuf.adjust();
        } else if (ret == -1 && errno != EAGAIN) {
            fprintf(stderr, "tcp client write \n");
            this->clean_conn();
        } else {
            //出错,不能再继续写
            break;
        }
    }

    if (_obuf.m_length == 0) {
        //已经写完，删除写事件
        // printf("do write over, del EPOLLOUT\n");
        this->_loop->del_io_event(_sockfd, EPOLLOUT);
    }

    return 0;
}

//释放链接资源,重置连接
void tcp_client::clean_conn() {
    if (_sockfd != -1) {
        printf("clean conn, del socket!\n");
        _loop->del_io_event(_sockfd);
        close(_sockfd);
    }

    connected = false;

    if (_conn_close_cb) {
        _conn_close_cb(this, _conn_close_cb_args);
    }

    //重新连接
    this->do_connect();
}

tcp_client::~tcp_client() { close(_sockfd); }

}  // namespace qc