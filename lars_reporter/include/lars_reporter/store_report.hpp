#pragma once

#include "mysql.h"
#include <proto/lars.pb.h>


namespace qc {

/// @brief 专门用来持久化数据的类, 每个线程都有这么一个对象
class StoreReport {
public:
    StoreReport();

    void store(lars::ReportStatusRequest &req);

private:
    /// @brief 一个存储记录类包含一个db_conn;
    MYSQL _db_conn;
};
}