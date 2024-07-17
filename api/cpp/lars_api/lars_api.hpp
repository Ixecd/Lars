/**
 * @file lars_api.hpp
 * @author qc
 * @brief api调用层 -> 实现 Report 和 API通信  -> 获取Route信息API
 * @version 0.2
 * @date 2024-05-30
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include <lars_reactor/lars_reactor.hpp>
#include <string>

namespace qc {

using ip_port = std::pair<std::string, int>;
using route_set = std::vector<ip_port>;
using route_set_it = route_set::iterator;

class lars_client {
public:
    lars_client();
    ~lars_client();
    /// @brief lars系统获取host信息,得到可用的host的ip和port 
    int get_host(int modid, int cmdid, std::string &ip, int &port);

    void report(int modid, int cmdid, std::string &ip, int port, int retcode);

    int get_route(int modid, int cmdid, route_set &route);

    /// @brief 由于get_host/get_route的时候,首次拉取一定是不成功的,因为首次拉取,agentserver并没有对当前的modid/cmdid的route信息做本地缓存。来注册初始化一次
    int reg_init(int modid, int cmdid);

private:
    /// @brief 3个udp socket fd对应agent 3个upd server
    int _sockfd[3];
    /// @brief 消息的序列号
    uint32_t _seqid;
};

}