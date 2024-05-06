#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__

#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "event_loop.hpp"
#include "tcp_conn.hpp"
#include "message.hpp"
#include "thread_pool.hpp"


namespace qc {
/**
 * @brief tcp_server类
 * @details
 * 当前提供创建连接服务,以及释放对象连接服务,这里一个tcp_server类对应一个sockfd,也就是一个连接
 * @version 0.2
 */
class tcp_server {
public:
    /**
     * @brief Construct a new tcp server object
     * @details RAII
     * @param ip
     * @param port
     */
    tcp_server(event_loop *loop, const char *ip, uint16_t port);
    /// @brief server tcp accept
    void do_accept();
    /// @brief release tcp accept
    ~tcp_server();

private:
    int _sockfd;
    struct sockaddr_in _connaddr;
    socklen_t _addrlen;

    /// @brief 再当前tcp_server中添加event_loop epoll事件机制,这里使用指针,解耦合
    event_loop* _loop;

    /// @brief 下面是对tcp_conn的封装,我们将tcp_server封装再tcp_conn中,之后调用tcp_conn方便管理

public:
    // ======== 消息分发路由 ========
    void add_msg_router(int msgid, msg_callback cb, void *user_data = nullptr) {
        router.register_msg_router(msgid, cb, user_data);
    }

    static msg_router router;

public:
    /// @brief 新增一个链接
    static void increase_conn(int connfd, tcp_conn *conn);
    /// @brief 减掉一个链接
    static void decrease_conn(int connfd);
    /// @brief 获得当前链接的数量
    /// @param[out] curr_conn 
    static void get_conn_num(int *curr_conn);
    /// @brief 全部在线的链接信息
    /// @details 如果是动态数量的链接,使用二级指针更灵活;如果是静态数量的链接,使用数组更好一点
    static tcp_conn **conns;

public:
    // ------------ 下面是Hook部分 ------------
    /// 客户端的Hook是自己的,服务端的Hook只有一份.
    /// 设置链接创建hook函数
    static void set_conn_start(conn_callback cb, void *args = nullptr) {
        conn_start_cb = cb;
        conn_start_cb_args = args;
    }
    /// 设置链接销毁hook函数
    static void set_conn_close(conn_callback cb, void *args = nullptr) {
        conn_close_cb = cb;
        conn_close_cb_args = args;
    }
    // 创建链接之后要触发的回调函数
    static conn_callback conn_start_cb;
    static void *conn_start_cb_args;
    // 销毁链接之后要触发的回调函数
    static conn_callback conn_close_cb;
    static void *conn_close_cb_args;

private:

#define MAX_CONNS 2
    /// @brief 默认最大连接数量为2
    static int _max_conns;
    static int _curr_conns;
    /// @brief 互斥锁
    static pthread_mutex_t _conns_mutex;

private:
    thread_pool *_thread_pool;

};
}  // namespace qc

#endif