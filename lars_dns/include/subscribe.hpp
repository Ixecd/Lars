/**
 * @file subscribe.hpp
 * @author qc
 * @brief Route订阅模式
 * @version 0.1
 * @date 2024-05-15
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once

#include <pthread.h>

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "dns_route.hpp"
#include "lars.pb.h"
#include "lars_reactor.hpp"
#include "mutex.hpp"
#include "singleton.hpp"

namespace qc {

/// @brief 定义订阅列表数据关系类型, key -> modid/cmdid, value -> fds
/// (订阅的客户端文件描述符)
/// 目前dns系统的总体订阅列表,记录了modid/cmdid都被哪些fds订阅了,
/// 一个modid/cmdid可以对应多个客户端
using subscribe_map = std::unordered_map<uint64_t, std::unordered_set<int>>;
/// @brief 发布列表的数据关系类型, key -> fd(订阅客户端的文件描述符) value ->
/// modids; 上面的反表
using publish_map = std::unordered_map<int, std::unordered_set<uint64_t>>;

// 单例模式
class SubscribeList : public Singleton<SubscribeList> {
    friend Singleton<SubscribeList>;

public:
    typedef Mutex MutexType;
    /// @brief 订阅
    void subscribe(uint64_t mod, int fd);

    /// @brief 取消订阅
    void unsubscribe(uint64_t mod, int fd);

    /// @brief 发布
    void publish(std::vector<uint64_t> &change_mods);

    /// @brief 根据在线用户fd得到需要发布的列表
    void make_publish_map(listen_fd_set &online_fds, publish_map &need_publish);

private:
    /// @brief 构造函数私有化
    SubscribeList();
    /// @brief 拷贝构造函数私有化
    SubscribeList(const SubscribeList &);
    /// @brief 拷贝赋值函数私有化
    const SubscribeList &operator=(const SubscribeList);

    /// @brief 订阅清单
    subscribe_map _book_list;
    /// @brief 订阅清单的互斥锁
    MutexType mutex_book_list;
    /// @brief 发布列表
    publish_map _push_list;
    /// @brief 发布列表互斥锁
    MutexType mutex_push_list;
};

}  // namespace qc