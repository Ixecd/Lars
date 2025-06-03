#include <lars_reactor/lars_reactor.hpp>
#include <lars_reporter/store_report.hpp>

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
    std::string db_host = config_file_instance::GetInstance()->GetString(
        "mysql", "db_host", "localhost");
    unsigned short db_port = config_file_instance::GetInstance()->GetNumber(
        "mysql", "db_port", 3306);
    std::string db_user = config_file_instance::GetInstance()->GetString(
        "mysql", "db_user", "qc");
    std::string db_passwd = config_file_instance::GetInstance()->GetString(
        "mysql", "db_passwd", "qcMysql");
    std::string db_name = config_file_instance::GetInstance()->GetString(
        "mysql", "db_name", "lars_dns");

    MYSQL *connection = mysql_real_connect(
        &_db_conn, db_host.c_str(), db_user.c_str(), db_passwd.c_str(),
        db_name.c_str(), db_port, nullptr, 0);

    if (connection == nullptr) {
        std::cout << "connection error\n";
        // 获取错误信息
        fprintf(stderr, "连接到MySQL数据库失败: %s\n", mysql_error(&_db_conn));
        exit(1);
    }

    qc_assert(connection != nullptr);
}

/// @brief
/// 对于StoreReport类来说,不需要单独的数据结构来存储结果,直接通过MYSQL将结果存储在数据库中
void StoreReport::store(lars::ReportStatusRequest &req) {
    // std::cout << "[report_store] report..." << std::endl;
    for (int i = 0; i < req.results_size(); ++i) {
        /// @brief
        /// 在ReportStatusRep中可以有多个客户端发起Report,这些信息都记录在ReportStatusReq中的results中
        // 一条记录
        const lars::HostCallResult &result = req.results(i);
        int overload = result.overload() ? 1 : 0;
        // 增加缓冲区安全余量（建议至少 2KB）
        char sql[2048];

        int written = snprintf(sql, sizeof(sql),
                               "INSERT INTO ServerCallStatus("
                               "modid, cmdid, ip, port, caller, "
                               "succ_cnt, err_cnt, ts, overload"
                               ") VALUES ("
                               "%u, %u, %u, %u, %u, "  // 前5个字段
                               "%u, %u, %u, %d"        // 后4个VALUES值
                               ") ON DUPLICATE KEY UPDATE "
                               "succ_cnt = VALUES(succ_cnt), "
                               "err_cnt = VALUES(err_cnt), "
                               "ts = VALUES(ts), "
                               "overload = VALUES(overload)",
                               // INSERT 部分参数
                               req.modid(),    // %u
                               req.cmdid(),    // %u
                               result.ip(),    // %u (无符号IP)
                               result.port(),  // %u (无符号端口)
                               req.caller(),   // %u (无符号调用方)
                               result.succ(),  // %u
                               result.err(),   // %u
                               req.ts(),       // %u
                               overload        // %d (整型)
        );

        // 检查缓冲区是否足够
        if (written >= sizeof(sql)) {
            // 处理缓冲区溢出错误
            fprintf(stderr, "SQL query truncated! Needed %d bytes\n", written);
            return ;
        }

        // 检查实际写入长度
        if (written < 0) {
            perror("snprintf failed");
            return ;
        }

        // ping 一下
        mysql_ping(&_db_conn);

        int rt = mysql_real_query(&_db_conn, sql, strlen(sql));
        if (rt) printf("%s\n", mysql_error(&_db_conn));

        qc_assert(rt == 0);

        // std::cout << "[report_store] succ" << std::endl;
    }
}

}  // namespace qc
