/**
 * @file dns_route.hpp
 * @author qc
 * @brief dns_route -> key : modid/cmdid.
 *                     value : set<string> ip + port
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

/// @brief 定义用来保存modid/cmdid 和 ip/port对应的关系
using route_map = std::unordered_map<uint64_t, std::unordered_set<uint64_t>>;
using route_map_iterator = route_map::iterator;

/// @brief 定义用来保存host的ip/port的port集合
using host_set = std::unordered_set<uint64_t>;
using host_set_iterator = host_set::iterator;

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

private:
    Route();

    const Route &operator=(const Route &);

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