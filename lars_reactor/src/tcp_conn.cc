#include <lars_reactor/tcp_conn.hpp>

#include <fcntl.h>
#include <netinet/in.h>   // for IPPROTO_TCP
#include <netinet/tcp.h>  // for TCP_NODELAY
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <lars_reactor/message.hpp>
#include <lars_reactor/tcp_server.hpp>
#include <lars_reactor/net_connection.hpp>


namespace qc {

/// @brief 回显业务
//回显业务
void callback_busi(const char *data, uint32_t len, int msgid, void *args,
                   net_connection *conn) {
    conn->send_message(data, len, msgid);
}

//连接的读事件回调
static void conn_rd_callback(event_loop *loop, int fd, void *args) {
    tcp_conn *conn = (tcp_conn *)args;
    conn->do_read();
}
//连接的写事件回调
static void conn_wt_callback(event_loop *loop, int fd, void *args) {
    tcp_conn *conn = (tcp_conn *)args;
    conn->do_write();
}

//初始化tcp_conn
tcp_conn::tcp_conn(int connfd, event_loop *loop) {
    _connfd = connfd;
    _loop = loop;
    // 1. 将connfd设置成非阻塞状态
    int flag = fcntl(_connfd, F_GETFL, 0);
    fcntl(_connfd, F_SETFL, O_NONBLOCK | flag);

    // 2. 设置TCP_NODELAY禁止做读写缓存，降低小包延迟
    int op = 1;
    setsockopt(_connfd, IPPROTO_TCP, TCP_NODELAY, &op,
               sizeof(op));  // need netinet/in.h netinet/tcp.h

    // 2.5 如果用户注册了Hook就立即调用
    if (tcp_server::conn_start_cb) {
        tcp_server::conn_start_cb(this, tcp_server::conn_start_cb_args);
    }

    // 3. 将该链接的读事件让event_loop监控
    _loop->add_io_event(_connfd, conn_rd_callback, EPOLLIN, this);

    // 4 将该链接集成到对应的tcp_server中
    tcp_server::increase_conn(_connfd, this);
}

//处理读业务
void tcp_conn::do_read() {
    // 1. 从套接字读取数据
    int ret = ibuf.read_data(_connfd);
    if (ret == -1) {
        fprintf(stderr, "read data from socket\n");
        this->clean_conn();
        return;
    } else if (ret == 0) {
        //对端正常关闭
        printf("connection closed by peer\n");
        clean_conn();
        return;
    }

    // 2. 解析msg_head数据
    msg_head head;

    //[这里用while，可能一次性读取多个完整包过来]
    // printf("cur ibuf.length() = %d\n", ibuf.length()); --> 19
    while (ibuf.length() >= MESSAGE_HEAD_LEN) {
        // 2.1 读取msg_head头部，固定长度MESSAGE_HEAD_LEN
        memcpy(&head, ibuf.data(), MESSAGE_HEAD_LEN);
        // printf("cur head.msgid = %d, head.msglen = %d\n", head.msgid, head.msglen);
        if (head.msglen > MESSAGE_LENGTH_LIMIT || head.msglen < 0) {
            fprintf(stderr, "data format error, need close, msglen = %d\n",
                    head.msglen);
          	// 如果信息属性不符合预定的设定,就直接关闭当前链接
            this->clean_conn();
            break;
        }
        if (ibuf.length() < MESSAGE_HEAD_LEN + head.msglen) {
            //缓存buf中剩余的数据，小于实际上应该接受的数据
            //说明是一个不完整的包，不应该抛弃,而是将数据继续缓存在buf中，由于通信方式是TCP,所以要等待下一次read将缺少的数据再读取到缓冲区中
          	//等待下一次读事件发生,这个时候就不会发生这种情况
          	//如果使用UDP，就有可能导致数据包的丢失，就需要在这里额外添加一些机制保证数据的正确性
            break;
        }

        // 2.2 再根据头长度读取数据体，然后针对数据体处理 业务
        // 添加写IO,触发回调函数
        // 回显数据,触发client的业务回调函数
        
        // _loop->add_io_event(_connfd, conn_wt_callback, EPOLLOUT, this);

        //头部处理完了，往后偏移MESSAGE_HEAD_LEN长度
        ibuf.pop(MESSAGE_HEAD_LEN);

        

        // printf("[tcp_server] : get data %s\n", ibuf.data());
        // char *buf;
        // memcpy(buf, ibuf.data(), head.msglen);
        // printf("[tcp_server]: get message : %s\n", buf);
        
        // 消息包路由模式
        tcp_server::router.call(head.msgid, head.msglen, ibuf.data(), this);

        // 回显
        //if (head.msgid == 1)
        //    callback_busi(ibuf.data(), strlen(ibuf.data()), head.msgid, nullptr, this);
        // this->send_message(ibuf.data(), strlen(ibuf.data()),
        //                                  head.msgid);

        //消息体处理完了,往后便宜msglen长度
        ibuf.pop(head.msglen);
    }

    ibuf.adjust();

    return;
}

//处理写业务
void tcp_conn::do_write() {
    // do_write是触发玩event事件要处理的事情，
    //应该是直接将out_buf力度数据io写会对方客户端
    //而不是在这里组装一个message再发
    //组装message的过程应该是主动调用
    /**
     * @brief 暂时在这里整一下
     */
    // this->send_message("hello,Lars!", 12, 2);

    // printf("cur server obuf.length() = %d\n", obuf.length());

    //只要obuf中有数据就写
    while (obuf.length()) {
        int ret = obuf.write2fd(_connfd);
        if (ret == -1) {
            fprintf(stderr, "write2fd error, close conn!\n");
            this->clean_conn();
            return;
        }
        if (ret == 0) {
            //不是错误，仅返回0表示不可继续写
            break;
        }
    }

    if (obuf.length() == 0) {
        //数据已经全部写完，将_connfd的写事件取消掉
        _loop->del_io_event(_connfd, EPOLLOUT);
    }

    return;
}

//发送消息的方法
int tcp_conn::send_message(const char *data, int msglen, int msgid) {
    bool active_epollout = false;
    // printf("tcp_conn::send_message start...\n");
    // printf("cur obuf.length() = %d\n", obuf.length());
    if (obuf.length() == 0) {
        //如果现在已经数据都发送完了，那么是一定要激活写事件的
        //如果有数据，说明数据还没有完全写完到对端，那么没必要再激活等写完再激活
        active_epollout = true;
    }

    // 1 先封装message消息头
    msg_head head;
    head.msgid = msgid;
    head.msglen = msglen;

    // 1.1 写消息头
    int ret = obuf.send_data((const char *)&head, MESSAGE_HEAD_LEN);
    if (ret != 0) {
        fprintf(stderr, "send head error\n");
        return -1;
    }

    // 1.2 写消息体
    ret = obuf.send_data(data, msglen);
    if (ret != 0) {
        //如果写消息体失败，那就回滚将消息头的发送也取消
        obuf.pop(MESSAGE_HEAD_LEN);
        return -1;
    }

    //  printf("call backing, and the obuf.length() = %d\n", obuf.length());

    if (active_epollout == true) {
        //激活EPOLLOUT写事件
        _loop->add_io_event(_connfd, conn_wt_callback, EPOLLOUT, this);
    }

    return 0;
}

//销毁tcp_conn
void tcp_conn::clean_conn() {
    //链接清理工作

    // 0 如果注册了连接销毁Hook函数
    if (tcp_server::conn_close_cb) {
        tcp_server::conn_close_cb(this, tcp_server::conn_close_cb_args);
    }

    // 1 将该链接从tcp_server摘除掉
    tcp_server::decrease_conn(_connfd);
    // 2 将该链接从event_loop中摘除
    _loop->del_io_event(_connfd);
    // 3 buf清空
    ibuf.clear();
    obuf.clear();
    // 4 关闭原始套接字
    int fd = _connfd;
    _connfd = -1;
    close(fd);
}

}  // namespace qc
