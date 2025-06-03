#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <lars_reactor/lars_reactor.hpp>
#include <lars_dns/dns_route.hpp>
#include <lars_dns/subscribe.hpp>

#include "mysql.h"

using namespace std;
// using namespace co_async;
namespace qc {

Route::Route() {
    /// @brief 初始化map
    _data_pointer = new route_map();
    _temp_pointer = new route_map();
    _version = 0;

    /// @brief 链接数据库
    this->connect_db();

    if (this->load_version() == -1) exit(1);

    /// @brief 在数据库中创建两个map
    this->build_maps();
}

void Route::connect_db() {
    // --- mysql 配置 ---
    std::string db_host = config_file_instance::GetInstance()->GetString(
        "mysql", "db_host", "localhost");
    short db_port = config_file_instance::GetInstance()->GetNumber(
        "mysql", "db_port", 3306);
    std::string db_user = config_file_instance::GetInstance()->GetString(
        "mysql", "db_user", "qc");
    std::string db_passwd = config_file_instance::GetInstance()->GetString(
        "mysql", "db_passwd", "qcMysql");
    std::string db_name = config_file_instance::GetInstance()->GetString(
        "mysql", "db_name", "lars_dns");
    /// @brief 开始链接
    mysql_init(&_db_conn);
    /// @brief mysql超时30ms自动断开
    mysql_options(&_db_conn, MYSQL_OPT_CONNECT_TIMEOUT, "30");
    /// @brief 设置重连
    char reconnect = 1;
    mysql_options(&_db_conn, MYSQL_OPT_RECONNECT, &reconnect);
    // cout << "after mysql_init" << endl;

    // MYSQL *connection = mysql_real_connect(
    //     &_db_conn, db_host.c_str(), db_user.c_str(), db_passwd.c_str(),
    //     db_name.c_str(), db_port, nullptr, 0);

    // qc_assert(connection != nullptr);

    MYSQL *connection = mysql_real_connect(&_db_conn, db_host.c_str(),
                                           db_user.c_str(), db_passwd.c_str(),
                                           db_name.c_str(), db_port, nullptr, 0);

    if (connection == nullptr) {
        cout << "connection error\n" << endl;
        // 获取错误信息
        fprintf(stderr, "连接到MySQL数据库失败: %s\n", mysql_error(&_db_conn));
        exit(1);
    }

    cout << "数据库链接成功" << endl;
}

void Route::build_maps() {
    snprintf(_sql, 1000, "SELECT * FROM RouteData;");
    /// @brief success return 0
    int rt = mysql_real_query(&_db_conn, _sql, strlen(_sql));
    qc_assert(rt == 0);

    /// @brief 将得到的结果存起来
    MYSQL_RES *result = mysql_store_result(&_db_conn);

    /// @得到行数
    long line_num = mysql_num_rows(result);

    // cout << "cur line_num = " << line_num << endl;

    MYSQL_ROW row;
    for (long i = 0; i < line_num; ++i) {
        row = mysql_fetch_row(result);
        uint modID = atoi(row[1]);
        uint cmdID = atoi(row[2]);
        uint ip = atoi(row[3]);
        uint port = atoi(row[4]);

        // 组装map的key，有modID/cmdID组合
        uint64_t key = ((uint64_t)modID << 32) + cmdID;
        uint64_t value = ((uint64_t)ip << 32) + port;

        printf("modID = %u, cmdID = %u ip = %u, port = %u\n", modID, cmdID,
               ip, port);

        // 插入到RouterDataMap_A中
        (*_data_pointer)[key].insert(value);
    }
    mysql_free_result(result);
}

host_set Route::get_hosts(uint modid, uint cmdid) {
    host_set hosts;
    // 组装key
    uint64_t key = ((uint64_t)modid << 32) + cmdid;

    RWMutexType::ReadLock lock(mutex);
    route_map_iterator it = _data_pointer->find(key);
    if (it != _data_pointer->end()) hosts = it->second;
    // lock.~ReadScopedLockImpl();

    return hosts;
}

int Route::load_version() {
    // 1.从RouteVersion表中查找当前版本信息
    snprintf(_sql, 1000, "select version from RouteVersion where id = 1;");
    // 成功返回0,结果存储在MySQL服务端
    int rt = mysql_real_query(&_db_conn, _sql, strlen(_sql));
    if (rt) {
        fprintf(stderr, "load version error: %s\n", mysql_error(&_db_conn));
        return -1;
    }
    // 将MySQL服务端的数据存储到客户端的内存中
    MYSQL_RES *result = mysql_store_result(&_db_conn);
    if (result == nullptr) {
        fprintf(stderr, "mysql store result: %s\n", mysql_error(&_db_conn));
        return -1;
    }

    // 获得查询的行数
    long lines = mysql_num_rows(result);
    if (lines == 0) {
        fprintf(stderr, "No version in table RouteVersion: %s\n", mysql_error(&_db_conn));
    }

    // 这里等值查询,结果只有一行
    MYSQL_ROW row = mysql_fetch_row(result);

    // 得到version
    long new_version = atol(row[0]);

    if (new_version == this->_version) return 0;

    this->_version = new_version;

    // 当前版本号已经被修改,要通知其他客户端
    printf("now route version = %ld\n", this->_version);

    mysql_free_result(result);

    return 1;
}

int Route::load_route_data() {
    _temp_pointer->clear();

    snprintf(_sql, 1000,
             "select modid, cmdid, serverip, serverport from RouteData;");

    int rt = mysql_real_query(&_db_conn, _sql, strlen(_sql));
    qc_assert(rt == 0);

    MYSQL_RES *result = mysql_store_result(&_db_conn);
    qc_assert(result != nullptr);

    long lines = mysql_num_rows(result);
    MYSQL_ROW row;
    for (long i = 0; i < lines; ++i) {
        row = mysql_fetch_row(result);
        uint modid = atoi(row[0]), cmdid = atoi(row[1]);
        uint ip = atoi(row[2]);
        uint port = atoi(row[3]);

        uint64_t key = ((uint64_t)modid << 32) + cmdid;
        uint64_t value = ((uint64_t)ip << 32) + port;

        (*_temp_pointer)[key].insert(value);
    }
    // 释放内存
    mysql_free_result(result);

    return 0;
}

void Route::swap() {
    // std::cout << "Route::swap() begin..." << std::endl;
    // 加读写范围锁
    RWMutexType::WriteLock lock(mutex);
    std::swap(_data_pointer, _temp_pointer);
    // route_map *temp = _data_pointer;
    // _data_pointer = _temp_pointer;
    // _temp_pointer = temp;

    // std::cout << "Route::swap() end..." << std::endl;
    return;
}

void Route::load_changes(std::vector<uint64_t> &changes) {
    // 读取当前版本之前所有修改
    snprintf(_sql, 1000,
             "select modid, cmdid from RouteChange where version <= %ld;",
             _version);

    int rt = mysql_real_query(&_db_conn, _sql, strlen(_sql));
    qc_assert(rt == 0);

    MYSQL_RES *result = mysql_store_result(&_db_conn);
    qc_assert(result != nullptr);
    // --------------
    long lines = mysql_num_rows(result);
    if (lines == 0) {
        fprintf(stderr, "No version in table ChangeLog: %s\n", mysql_error(&_db_conn));
        return;
    }

    MYSQL_ROW row;
    for (long i = 0; i < lines; ++i) {
        row = mysql_fetch_row(result);
        uint modid = atoi(row[0]), cmdid = atoi(row[1]);
        uint64_t key = ((uint64_t)modid << 32) + cmdid;
        changes.push_back(key);
    }
    mysql_free_result(result);
}

void Route::remove_changes(bool remove_all) {
    if (remove_all) {
        snprintf(_sql, 1000, "delete from RouteChange;");
    } else {  // 删除当前版本和之前的所有修改记录,只保留最新的
        snprintf(_sql, 1000, "delete from RouteChange where version <= %ld;",
                 _version);
    }
    int rt = mysql_real_query(&_db_conn, _sql, strlen(_sql));
    if (rt != 0) {
        fprintf(stderr, "delete RouteChange: %s\n", mysql_error(&_db_conn));
        return;
    }
    return;
}

/// @brief 周期性的检查db中route的version信息,由Backend Thread业务调用
void *check_route_change(void *args) {
    // 每隔10s检查一次,由config文件配置
    int wait_time =
        config_file_instance::GetInstance()->GetNumber("route", "wait_time", 8);
    // printf("wait_time = %d\n", wait_time);
    long last_load_time = time(nullptr);

    // 清空全部RouteChange
    Route::GetInstance()->remove_changes(true);

    while (true) {
        std::cout << "[Backend_Thread] run..." << std::endl;
        sleep(3);

        long current_time = time(nullptr);

        // 1.加载当前版本信息
        int rt = Route::GetInstance()->load_version();

        // 版本号被修改了
        if (rt == 1) {
            // 意味着有modid/cmdid修改
            std::cout << "[version changed]" << std::endl;

            // 将最新的RouteData移动到_temp_pointer中
            if (Route::GetInstance()->load_route_data() == 0) {
                // 成功,交换_temp_pointer和_data_pointer中的数据
                Route::GetInstance()->swap();
                // 更新最后的加载时间
                last_load_time = current_time;
            }

            // 获取被修改的modid/cmdid对应的订阅客户端信息,推送
            std::vector<uint64_t> changes;
            Route::GetInstance()->load_changes(changes);

            for (auto num : changes)
                std::cout << "changes = " << num << std::endl;

            // 推送
            if (changes.size() != 0) {
                std::cout << "[start publish]" << std::endl;
                SubscribeList::GetInstance()->publish(changes);
                // GetInstance<SubscribeList>()->publish(changes);
            }

            // 删除当前版本之前的修改记录
            Route::GetInstance()->remove_changes();
        } else {
            // 版本号没有被修改
            if (current_time - last_load_time >= wait_time) {
                // 超时
                if (Route::GetInstance()->load_route_data() == 0) {
                    Route::GetInstance()->swap();
                    last_load_time = current_time;
                    std::cout << "[swapped]" << std::endl;
                }
            }
        }
    }
    return nullptr;
}

}  // namespace qc