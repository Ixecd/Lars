/**
 * @file dns_route.cc
 * @author qc
 * @brief dns_route的封装 + Mysql
 * @version 0.1
 * @date 2024-05-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "dns_route.hpp"

#include <string.h>
#include <unistd.h>

#include <iostream>

#include "lars_reactor.hpp"
#include "mysql.h"
#include "qc.hpp"

using namespace std;

namespace qc {

Route::Route() {
    /// @brief 初始化map
    _data_pointer = new route_map();
    _temp_pointer = new route_map();

    /// @brief 链接数据库
    this->connect_db();
    /// @brief 在数据库中创建两个map
    this->build_maps();
}

void Route::connect_db() {
    // --- mysql 配置 ---
    std::string db_host =
        config_file::GetInstance()->GetString("mysql", "db_host", "127.0.0.1");
    short db_port =
        config_file::GetInstance()->GetNumber("mysql", "db_port", 3306);
    std::string db_user =
        config_file::GetInstance()->GetString("mysql", "db_user", "qc");
    std::string db_passwd =
        config_file::GetInstance()->GetString("mysql", "db_passwd", "qcMysql");
    std::string db_name =
        config_file::GetInstance()->GetString("mysql", "db_name", "lars_dns");
    /// @brief 开始链接
    mysql_init(&_db_conn);
    /// @brief mysql超时30ms自动断开
    mysql_options(&_db_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
    /// @brief 设置重连
    char reconnect = 1;
    mysql_options(&_db_conn, MYSQL_OPT_RECONNECT, &reconnect);
    cout << "after mysql_init" << endl;

    qc_assert(mysql_real_connect(&_db_conn, db_host.c_str(), db_user.c_str(),
    db_passwd.c_str(), db_name.c_str(), db_port, nullptr, 0) != nullptr);

    // MYSQL *connection =
    //     mysql_real_connect(&_db_conn, "localhost", "root", "yqc2192629378",
    //                        "lars_dns", 3306, nullptr, 0);
    // if (connection == nullptr) {
    //     cout << "connection error\n" << endl;
    //     // 获取错误信息
    //     fprintf(stderr, "连接到MySQL数据库失败: %s\n", mysql_error(&_db_conn));
    //     exit(1);
    // }
    cout << "end of connect_db()..." << endl;
}

void Route::build_maps() {
    snprintf(_sql, 1000, "SELECT * FROM RouteData;");
    /// @brief success return 0
    qc_assert(mysql_real_query(&_db_conn, _sql, strlen(_sql)) == 0);

    /// @brief 将得到的结果存起来
    MYSQL_RES *result = mysql_store_result(&_db_conn);

    /// @得到行数
    long line_num = mysql_num_rows(result);

    cout << "cur line_num = " << line_num << endl;

    MYSQL_ROW row;
    for (long i = 0; i < line_num; ++i) {
        row = mysql_fetch_row(result);
        int modID = atoi(row[1]);
        int cmdID = atoi(row[2]);
        unsigned long ip = atoi(row[3]);
        int port = atoi(row[4]);

        //组装map的key，有modID/cmdID组合
        uint64_t key = ((uint64_t)modID << 32) + cmdID;
        uint64_t value = ((uint64_t)ip << 32) + port;

        printf("modID = %d, cmdID = %d, ip = %lu, port = %d\n", modID, cmdID,
               ip, port);

        //插入到RouterDataMap_A中
        (*_data_pointer)[key].insert(value);
    }
    mysql_free_result(result);
}

host_set Route::get_hosts(int modid, int cmdid) {
    host_set hosts;
    // 组装key
    uint64_t key = ((uint64_t)modid << 32) + cmdid;

    RWMutexType::ReadLock lock(mutex);
    route_map_iterator it = _data_pointer->find(key);
    if (it != _data_pointer->end()) hosts = it->second;
    // lock.~ReadScopedLockImpl();

    return hosts;
}



}  // namespace qc