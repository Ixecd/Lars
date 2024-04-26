#ifndef __TCP_SERVER_HPP__
#define __TCP_SERVER_HPP__

#include <arpa/inet.h>
#include <netinet/in.h>

namespace qc {
/**
 * @brief tcp_server类
 * @details
 * 当前提供创建连接服务,以及释放对象连接服务,这里一个tcp_server类对应一个sockfd,也就是一个连接
 */
class tcp_server {
public:
    /**
     * @brief Construct a new tcp server object
     * @details RAII
     * @param ip
     * @param port
     */
    tcp_server(const char* ip, uint16_t port);
    /// @brief server tcp accept
    void do_accept();
    /// @brief release tcp accept
    ~tcp_server();

private:
    int _sockfd;
    struct sockaddr_in _connaddr;
    socklen_t _addrlen;
};
}  // namespace qc

#endif