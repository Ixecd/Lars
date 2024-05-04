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

namespace qc {

class net_connection{
public:
    virtual int send_message(const char *data, int datalen, int msgid) = 0;
};



}

