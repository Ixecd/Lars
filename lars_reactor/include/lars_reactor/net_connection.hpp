#pragma once

#include <functional>

namespace qc {

class net_connection;
// 创建链接和销毁链接的时候触发的回调函数
using conn_callback = std::function<void(net_connection*, void*)>;


/// @brief tcp_server : net_connection tcp_server只保存链接相关信息
///        tcp_conn   : net_connection tcp_conn 用来进行通信

class net_connection{
public:

    net_connection() : param(nullptr) {}

    virtual int get_fd() = 0;

    /// @brief tcp客户端 通信的时候可以通过这个参数传递一些自定义的参数 
    void *param = nullptr;

    virtual int send_message(const char *data, int datalen, int msgid) = 0;

};



}

