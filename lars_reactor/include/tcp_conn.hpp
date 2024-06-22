/**
 * @file tcp_conn.hpp
 * @author qc
 * @brief tcp_conn连接类
 * @version 0.1
 * @date 2024-04-28
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __TCP_CONN_HPP__
#define __TCP_CONN_HPP__

#include "qc.hpp"
#include "event_loop.hpp"
#include "reactor_buf.hpp"
#include "net_connection.hpp"


namespace qc {

/**
 * @brief tcp_conn class
 * @details 一个tcp_conn 对应两个独立的buff,一个tcp_conn类相当于一个tcp_server类,对其又专门进行了一次封装,专门用来
 * 			和客户端进行通信。
 *			使用event_loop* 是为了降低耦合。
 *			关于tcp_conn中的event_loop*从哪里来,服务器框架集成了线程池之后,一个tcp_server会生成多个线程,
 *			每个线程都有自己的消息队列,所以每个线程都有自己独有的一个event_loop,这里的event_loop*就是对应线程的
 *			event_loop*.
 */	
class tcp_conn : public net_connection{
public:
    /// @brief 初始化tcp_conn
    tcp_conn(int connfd, event_loop *loop);
    /// @brief 处理读事件
    void do_read();
    /// @brief 处理写事件
    void do_write();
    /// @brief 销毁tcp_conn
    void clean_conn();
    /// @brief 发送消息的方法 
    int send_message(const char *data, int msglen, int msgid);
    /// @brief 获取当前连接fd
    /// @return 
    int get_fd() { return _connfd; }
private:    
    /// @brief 当前连接的fd
    int _connfd;
    /// @brief 该连接所属的event_poll,对应线程的
    event_loop *_loop; 
    /// @brief 输入buff
    input_buf ibuf;
    /// @brief 输出buff
    output_buf obuf;
};



} // namespace q



#endif
