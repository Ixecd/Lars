#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__

#define MAX_CONNS 2

#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "event_loop.hpp"
#include "tcp_conn.hpp"
#include "message.hpp"


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

private:
    /// @brief 默认最大连接数量为2
    static int _max_conns;
    static int _curr_conns;
    /// @brief 互斥锁
    static pthread_mutex_t _conns_mutex;
};
}  // namespace qc

#endif