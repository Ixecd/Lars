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
    cout << db_host << endl;
    short db_port =
        config_file::GetInstance()->GetNumber("mysql", "db_port", 3306);
    cout << db_port << endl;
    std::string db_user =
        config_file::GetInstance()->GetString("mysql", "db_user", "qc");
    cout << db_user << endl;
    std::string db_passwd =
        config_file::GetInstance()->GetString("mysql", "db_passwd", "qcMysql");
    cout << db_passwd << endl;
    std::string db_name =
        config_file::GetInstance()->GetString("mysql", "db_name", "lars_dns");
    cout << db_name << endl;
    /// @brief 开始链接
    mysql_init(&_db_conn);
    /// @brief mysql超时30ms自动断开
    mysql_options(&_db_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
    /// @brief 设置重连
    char reconnect = 1;
    mysql_options(&_db_conn, MYSQL_OPT_RECONNECT, &reconnect);

    MYSQL *connection = mysql_real_connect(&_db_conn, "localhost", "root", "yqc2192629378", "lars_dns", 3306, nullptr, 0);
    if (connection == nullptr) {
        cout << "connection error\n" << endl;
        // 获取错误信息
        fprintf(stderr, "连接到MySQL数据库失败: %s\n", mysql_error(&_db_conn));
        exit(1);
    }

}

void Route::build_maps() {
    int rt = 0;
    snprintf(_sql, 1000, "select * from RouteData;");
    qc_assert(mysql_real_query(&_db_conn, _sql, strlen(_sql)) != 0);
}

}  // namespace qc