/**
 * @file store_report.hpp
 * @author qc
 * @brief 存储report
 * @version 0.1
 * @date 2024-05-18
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include "mysql.h"
#include "lars.pb.h"


namespace qc {

class StoreReport {
public:
    StoreReport();

    void store(lars::ReportStatusReq &req);

private:
    /// @brief 一个存储记录类包含一个db_conn;
    MYSQL _db_conn;
};
}