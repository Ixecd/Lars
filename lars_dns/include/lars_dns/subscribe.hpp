#pragma once

#include <proto/lars.pb.h>
#include <pthread.h>
#include <lars_dns/dns_route.hpp>
#include <lars_reactor/lars_reactor.hpp>
#include <map>
#include <set>
#include <vector>

namespace qc {

/// @brief 定义订阅列表数据关系类型, key -> modid/cmdid, value -> fds
/// (订阅的客户端文件描述符)
/// 目前dns系统的总体订阅列表,记录了modid/cmdid都被哪些fds订阅了,
/// 一个modid/cmdid可以对应多个客户端
using subscribe_map = std::map<uint64_t, std::set<int>>;
/// @brief 发布列表的数据关系类型, key -> fd(订阅客户端的文件描述符) value ->
/// modids; 上面的反表
using publish_map = std::map<int, std::set<uint64_t>>;

// 单例模式
//: public Singleton<SubscribeList>
class SubscribeList : public Singleton<SubscribeList>{
    friend Singleton<SubscribeList>;
public:
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

    SubscribeList &operator=(SubscribeList &&) = delete;

private:
    /// @brief modid/cmdid订阅列表
    subscribe_map _book_list;

    std::mutex mutex_book_list;

    /// @brief fd对应的modid/cmdid列表
    publish_map _push_list;

    std::mutex mutex_push_list;
};


}  // namespace qc