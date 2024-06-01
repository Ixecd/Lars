/**
 * @file lars_api.cc
 * @author qc
 * @brief lars_api的实现
 * @version 0.1
 * @date 2024-05-30
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "lars_api.hpp"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>

#include "lars.pb.h"

#define DEFAULT_BUFFER_SIZE 4 * 1024

namespace qc {

lars_client::lars_client() : _seqid(0) {
    std::cout << "lars_client start..." << std::endl;
    // 1.初始化服务器地址
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // UDP通信只需要ip和port
    inet_aton("127.0.0.1", &servaddr.sin_addr);

    // 2.创建3个UDPsocket
    for (int i = 0; i < 3; ++i) {
        _sockfd[i] = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, IPPROTO_UDP);
        qc_assert(_sockfd[i] != -1);

        // 一个struct sockaddr_in可以多次使用
        servaddr.sin_port = htons(8888 + i);
        int rt = connect(_sockfd[i], (const struct sockaddr *)&servaddr,
                         sizeof(servaddr));
        qc_assert(rt != -1);

        std::cout << "connection agent udp server succ!\n";
    }
}

lars_client::~lars_client() {
    std::cout << "lars_client::~lars_client()..." << std::endl;
    for (int i = 0; i < 3; ++i) close(_sockfd[i]);
}

/// @brief 从lars系统中获取host信息,得到可用的host的ip和port
int lars_client::get_host(int modid, int cmdid, std::string &ip, int &port) {
    // 1.从本地agent service获取host信息
    uint32_t seq = _seqid++;

    lars::GetHostRequest req;
    req.set_seq(seq);
    req.set_modid(modid);
    req.set_cmdid(cmdid);

    // 2.send
    char write_buf[4096], read_buf[80 * 1024];
    // 消息头
    msg_head head;
    head.msglen = req.ByteSizeLong();
    head.msgid = lars::ID_GetHostRequest;
    // 先写头
    memcpy(write_buf, &head, MESSAGE_HEAD_LEN);

    // 再写消息体
    req.SerializeToArray(write_buf + MESSAGE_HEAD_LEN, head.msglen);

    // 简单的hash来发送给对应的agent udp server
    int index = (modid + cmdid) % 3;
    // UDP sendto
    int rt = sendto(_sockfd[index], write_buf, head.msglen + MESSAGE_HEAD_LEN, 0, nullptr, 0);

    if (rt == -1) {
        perror("sendto");
        return lars::RET_SYSTEM_ERR;
    }

    // 3. recv
    int message_len = 0;
    lars::GetHostResponse rsp;
    do {
        message_len = recvfrom(_sockfd[index], read_buf, sizeof(read_buf), 0, nullptr, nullptr);
        if (message_len == -1) {
            perror("recvfrom");
            return lars::RET_SYSTEM_ERR;
        }

        // 消息头
        memcpy(&head, read_buf, MESSAGE_HEAD_LEN);
        if (head.msgid != lars::ID_GetHostResponse) {
            // 不是Agent返回给API host信息的ID
            fprintf(stderr, "message ID error!\n");
            return lars::RET_SYSTEM_ERR;
        }

        // 消息体
        rt = rsp.ParseFromArray(read_buf + MESSAGE_HEAD_LEN, message_len - MESSAGE_HEAD_LEN);

        if (rt == 0) {
            fprintf(stderr, "message format error: head.msglen = %d, message_len = %d, message_len - MESSAGE_HEAD_LEN = %d, head msgid = %d, ID_GetHostResponse = %d\n", head.msglen, message_len, message_len - MESSAGE_HEAD_LEN, head.msgid, lars::ID_GetHostResponse);
            return lars::RET_SYSTEM_ERR;
        }
    } while (rsp.seq() < seq); // ------------><------------

    if (rsp.seq() != seq || rsp.modid() != modid || rsp.cmdid() != cmdid) {
        fprintf(stderr, "message format error\n");
        return lars::RET_SYSTEM_ERR;
    }

    // 4.处理消息 retcode() == 0 表示成功
    if (rsp.retcode() == 0) {
        lars::HostInfo host = rsp.host();
        
        struct in_addr inaddr;
        inaddr.s_addr = host.ip();
        ip = inet_ntoa(inaddr);
        port = host.port();
    }
    return rsp.retcode();
}

/**
 * @brief API 客户端将host主机信息发送给UDP server
 * @details 所谓report其实就是向服务端发送请求,由服务端控制,具体实现还是由route_lb控制由load_balance模块自己实现.
 */
void lars_client::report(int modid, int cmdid, std::string &ip, int port, int retcode) {
    // 1.封装消息
    lars::ReportRequest report;
    report.set_modid(modid);
    report.set_cmdid(cmdid);
    report.set_retcode(retcode);

    // 1.1 封装host信息
    /// @brief 由于无法直接修改message中的message信息,所以proto3提供了mutable_message()返回指针来修改message
    lars::HostInfo* hi = report.mutable_host();

    // 1.2 转换类型(ip)
    struct in_addr inaddr; // in.h

    /// @details 将一个点分十进制表示的 IPv4 地址（如 "192.168.1.1"）转换成一个二进制的网络字节序（big-endian）的整数形式。 
    inet_aton(ip.c_str(), &inaddr);
    
    hi->set_ip(inaddr.s_addr);
    hi->set_port(port);

    // 2.发送消息
    char write_buf[DEFAULT_BUFFER_SIZE];

    // 消息头
    msg_head head;
    head.msglen = report.ByteSizeLong();
    head.msgid = lars::ID_ReportRequest;

    // 先写头,再写体
    memcpy(write_buf, &head, MESSAGE_HEAD_LEN);
    // 发送是序列化,接受是反序列化
    report.SerializeToArray(write_buf + MESSAGE_HEAD_LEN, head.msglen);

    // 根据哈希结果向对应的UDP Service发送请求
    int index = (modid + cmdid) % 3;
    // UDP 注意:发送的是所有的信息,包括头和体
    int rt = sendto(_sockfd[index], write_buf, report.ByteSizeLong() + MESSAGE_HEAD_LEN, 0, nullptr, 0);
    qc_assert(rt != -1);
}

}  // namespace qc
