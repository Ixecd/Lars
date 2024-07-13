#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>

#include "event_loop.hpp"
#include "message.hpp"
#include "mutex.hpp"
#include "tcp_conn.hpp"
#include "thread_pool.hpp"

namespace qc {
/**
 * @brief tcp_server类
 * @details
 * 当前提供创建连接服务,以及释放对象连接服务,这里一个tcp_server类对应一个sockfd,这里的sockfd是专门用来监听的,并不是作为通信描述符存在
 * 对于一个tcp_server而言,ip作为字符串,port作为unsigned short来和tcp_client通信。
 * @version 0.2
 */
class tcp_server {
public:
    tcp_server(event_loop *loop, const char *ip, uint16_t port);
    /// @brief server tcp accept
    void do_accept();
    /// @brief release tcp accept
    ~tcp_server();

private:
    int _sockfd;
    struct sockaddr_in _connaddr;
    socklen_t _addrlen;

    /// @brief 再当前tcp_server中添加event_loop
    event_loop *_loop;

public:
    // ======== 消息分发路由 ========
    void add_msg_router(int msgid, msg_callback cb, void *user_data = nullptr) {
        router.register_msg_router(msgid, cb, user_data);
    }
    
    static msg_router router;

public:
    typedef Mutex MutexType;
    /// @brief 新增一个链接
    static void increase_conn(int connfd, tcp_conn *conn);
    /// @brief 减掉一个链接
    static void decrease_conn(int connfd);
    /// @brief 获得当前链接的数量
    /// @param[out] curr_conn
    static void get_conn_num(int *curr_conn);
    /// @brief 全部在线的链接信息
    /// @details
    /// 如果是动态数量的链接,使用二级指针更灵活;如果是静态数量的链接,使用数组更好一点
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
    static MutexType _conns_mutex;
    // static pthread_mutex_t _conns_mutex;

public:
    thread_pool *get_thread_pool() { return _thread_pool; }

private:
    thread_pool *_thread_pool;
};
}  // namespace qc

#endif
