/**
 * @file net_connection.hpp
 * @author qc
 * @brief 抽象类,其子类有tcp_client和tcp_conn
 * @version 0.1
 * @date 2024-05-04
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#pragma once

#include <functional>

namespace qc {

// 创建Hook机制
class net_connection;
using conn_callback = std::function<void(net_connection*, void*)>;
class net_connection{
public:
    net_connection() {}

    virtual int send_message(const char *data, int datalen, int msgid) = 0;
};



}

