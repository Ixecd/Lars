/**
 * @file dns_route.hpp
 * @author qc
 * @brief dns_route -> key : modid/cmdid.
 *                     value : set<uint64_t> ip + port
 * @version 0.1
 * @date 2024-05-13
 *
 * @copyright Copyright (c) 2024
 *
 */
#pragma once
#include <pthread.h>

#include <mutex.hpp>
#include <unordered_map>
#include <unordered_set>

#include "mysql.h"
#include "qc.hpp"
#include "singleton.hpp"

namespace qc {

/// @brief 定义用来保存modid/cmdid 和 ip/port对应的关系, 这里保证modid和cmdid非负
using route_map = std::unordered_map<uint64_t, std::unordered_set<uint64_t>>;
using route_map_iterator = route_map::iterator;

/// @brief 定义用来保存host的ip/port的port集合
using host_set = std::unordered_set<uint64_t>;
using host_set_iterator = host_set::iterator;

void* check_route_change(void *args);

/// @brief 单例模式
class Route : public Singleton<Route> {
    friend Singleton<Route>;

public:
    typedef RWMutex RWMutexType;

    /// @brief 链接数据库
    void connect_db();
    /// @brief 查询数据库,创建两个map
    void build_maps();
    /// @brief 设置数据库
    void set_mysql(MYSQL dbconn) { _db_conn = dbconn; }

public:
    /// @brief 获取Host信息
    host_set get_hosts(uint modid, uint cmdid);

public:
    /// @brief 加载版本信息, 1 success, 0 everything is update
    int load_version();
    /// @brief 当前Route的version
    long _version;

public:
    /// @brief 加载Routedata到_temp_pointer中
    int load_route_data();
    /// @brief 将_temp_pointer中的数据放到_data_pointer中
    void swap();
    /// @brief 将修改了的modid/cmdid加载到当前Route_map中
    void load_changes(std::vector<uint64_t> &changes);
    /// @brief 删除RouteChange的全部修改记录数据
    /// @param remove_all 删除所有
    void remove_changes(bool remove_all = false);

private:
    /// @brief 私有三大件
    Route();
    Route(const Route&);
    const Route &operator=(const Route &);

private:

    // ========== 属性 ==========
    /// @brief 数据库链接
    MYSQL _db_conn;
    /// @brief sql语句
    char _sql[1000];
    /// @brief modid/cmdid --- ip/port对应关系
    /// @brief RouterDataMap_A
    route_map *_data_pointer;
    /// @brief RouterDataMap_B, 临时的
    route_map *_temp_pointer;
    /// @brief 读写锁
    RWMutexType mutex;
};

}  // namespace qc