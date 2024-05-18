/**
 * @file store_report.cc
 * @author qc
 * @brief 存储记录封装
 * @version 0.1
 * @date 2024-05-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "store_report.hpp"
#include "lars_reactor.hpp"

namespace qc {

StoreReport::StoreReport() {
    // 1.初始化
    // 1.1 多线程使用MYSQL需要先调用mysql_library_init
    mysql_library_init(0, nullptr, nullptr);
    // 2. 初始化链接
    mysql_init(&_db_conn);
    // 2.1 设置time_out 30 ms
    mysql_options(&_db_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
    // 2.2 设置超时重连
    char reconnect;
    mysql_options(&_db_conn, MYSQL_OPT_RECONNECT, &reconnect);

    // 3. 加载配置
    std::string db_host = config_file::GetInstance()->GetString("mysql", "db_host", "127.0.0.1");
    short db_port = config_file::GetInstance()->GetNumber("mysql", "db_port", 3306);
    std::string db_user = config_file::GetInstance()->GetString("mysql", "db_user", "qc");
    std::string db_passwd = config_file::GetInstance()->GetString("mysql", "db_passwd", "123456");
    std::string db_name = config_file::GetInstance()->GetString("mysql", "db_name", "lars_dns");

    MYSQL *connection = mysql_real_connect(&_db_conn, db_host.c_str(), db_user.c_str(), db_passwd.c_str(), db_name.c_str(), db_port, nullptr, 0);

    qc_assert(connection != nullptr);
}

/// @brief 对于StoreReport类来说,不需要单独的数据结构来存储结果,直接通过MYSQL将结果存储在数据库中
void StoreReport::store(lars::ReportStatusReq &req) {
    for (int i = 0; i < req.results_size(); ++i) {
        /// @brief 在ReportStatusRep中可以有多个客户端发起Report,这些信息都记录在ReportStatusReq中的results中
        // 一条记录
        const lars::HostCallResult &result = req.results(i);
        int overload = result.overload() ? 1 : 0;

        // 往数据库中存储数据
        char sql[1024];
        // snprintf(sql, 1024, "insert into ServerCallStatus(modid, cmdid, ip, port, caller, succ_cnt, err_cnt, ts, overload) values (%d %d %u %u %u %u %u %u %d) on duplicate key update succ_cnt = %u, err_cnt = %u, ts = %u, overload = %d", req.modid(), req.cmdid(), result.ip(), result.port(), req.caller(), result.succ(), result.err(), req.ts(), overload, result.succ(), result.err(), req.ts(), overload);

        "INSERT INTO ServerCallStatus(modid, cmdid, ip, port, caller, "
        "succ_cnt, err_cnt, ts, overload) VALUES (%d, %d, %u, %u, %u, %u, %u, "
        "%u, %d) ON DUPLICATE KEY UPDATE succ_cnt = %u, err_cnt = %u, ts = %u, "
        "overload = %d";
        snprintf(sql, 1024,
                 "INSERT INTO ServerCallStatus(modid, cmdid, ip, port, caller, "
                 "succ_cnt, err_cnt, ts, overload) VALUES (%d, %d, %u, %u, %u, "
                 "%u, %u, "
                 "%u, %d) ON DUPLICATE KEY UPDATE succ_cnt = %u, err_cnt = %u, "
                 "ts = %u, "
                 "overload = %d",
                 req.modid(), req.cmdid(), result.ip(), result.port(),
                 req.caller(), result.succ(), result.err(), req.ts(), overload,
                 result.succ(), result.err(), req.ts(), overload);

            // ping 一下
        mysql_ping(&_db_conn);

        int rt = mysql_real_query(&_db_conn, sql, strlen(sql));
        qc_assert(rt == 0);
    }
}


}