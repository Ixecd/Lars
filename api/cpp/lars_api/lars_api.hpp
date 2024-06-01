/**
 * @file lars_api.hpp
 * @author qc
 * @brief api调用层 -> 实现 Report 和 API通信 
 * @version 0.2
 * @date 2024-05-30
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include "lars_reactor.hpp"
#include <string>

namespace qc {

class lars_client {
public:
    lars_client();
    ~lars_client();
    /// @brief lars系统获取host信息,得到可用的host的ip和port 
    int get_host(int modid, int cmdid, std::string &ip, int &port);

    void report(int modid, int cmdid, std::string &ip, int port, int retcode);
private:
    /// @brief 3个udp socket fd对应agent 3个upd server
    int _sockfd[3];
    /// @brief 消息的序列号
    uint32_t _seqid;
};

}