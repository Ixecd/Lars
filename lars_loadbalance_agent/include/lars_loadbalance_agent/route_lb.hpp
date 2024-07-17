/**
 * @file route_lb.hpp
 * @author qc
 * @brief
 * @version 0.1
 * @date 2024-05-30
 *
 * @copyright Copyright (c) 2024
 *
 */

/// @details
/// 关于route->指的是客户端业务函数可能有很多种,应该对他们进行分类,之后根据id分发给不同的线程

#pragma once

#include <unordered_map>

#include <proto/lars.pb.h>
#include <lars_loadbalance_agent/load_balance.hpp>
#include <lars_reactor/mutex.hpp>

namespace qc {

// key -> modid + cmdid, value -> load_balance
// 一个load_balance管理一组modid/cmdid的集群负载
using route_map = std::unordered_map<uint64_t, load_balance *>;
using rote_map_it = route_map::iterator;

/**
 * @brief
 * 针对多组modid/cmdid,route_lb是管理多个load_balance模块的,目前设计有3个,和udp-server的数量一致,每个route_lb分别根据
 * modid/cmdid的值做hash,分管不同的modid/cmdid
 */
class route_lb {
public:
    typedef Mutex MutexType;
    /// @brief 构造初始化函数
    route_lb(int id);
    /// @brief agent获取一个host主机,将返回的主机结果存放在rsp
    int get_host(int modid, int cmdid, lars::GetHostResponse &rsp);
    /// @brief 根据DNS_Service返回的结果更新自己的route_lb_map
    int update_host(int modid, int cmdid, lars::GetRouteResponse &rsp);
    /// @brief 由route_lb上报某主机的获取结果
    void report_host(lars::ReportRequest req);
    /// @brief 重置lb状态
    void reset_lb_status();
    /// @brief 获取route信息
    int get_route(int modid, int cmdid, lars::GetRouteResponse &rsp);

private:
    /// @brief 当前route_lb下的管理的loadbalance
    route_map _route_lb_map;
    /// @brief 互斥锁
    MutexType _mutex;
    /// @brief 当前route_lb的Id
    int _id;
};

}  // namespace qc