/**
 * @file load_balance.hpp
 * @author qc
 * @brief 加载负载均衡
 * @version 0.1
 * @date 2024-05-24
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once
#include <list>
#include <unordered_map>

#include <lars_loadbalance_agent/host_info.hpp>
#include <proto/lars.pb.h>

namespace qc {

// key -> ip + port, value -> host_info
using host_map = std::unordered_map<uint64_t, qc::host_info*>;
using host_map_it = host_map::iterator;

// 下面是host_info list集合
using host_list = std::list<qc::host_info*>;
using host_list_it = host_list::iterator;

/// @brief 针对一组的modid/cmdid的负载均衡
class load_balance {
public:
    load_balance(int modid, int cmdid) : _modid(modid), _cmdid(cmdid), status(PULLING), last_update_time(0) {}

    // 判断是否已经没有host在当前的LB中
    bool empty() const { return _idle_list.empty() && _overload_list.empty(); }

    // 从当前的双队列中获取host信息
    int choice_one_host(lars::GetHostResponse &rsp);

    // 如果list中没有host信息,需要从远程的DNS Server中发送GetRouteRequest消息获取host信息
    int pull(); 

    // 根据dns_server返回的结果更新_host_map;
    void update(lars::GetRouteResponse &req);

    // 获取当前lb下所有host的信息
    void get_all_hosts(std::vector<host_info*> &vec);

    void report(uint ip, uint port, uint retcode);

    void commit();

    enum STATUS {
        PULLING,
        NEW
    };
    // 当前状态
    STATUS status;

private:
    int _modid;
    int _cmdid;
    int _access_cnt;

    host_map _host_map;

    host_list _idle_list;
    host_list _overload_list;

public:
    /// @details 给load_balance设置最后的update时间参数,以及最后一次从dns service拉取下来的更新host_map的时间,然后再route_lb每次执行get_host的时候,对每个host节点做最后的超市检查,如果超时,就从dns Service重新拉取
    
    long last_update_time;

};

}  // namespace qc