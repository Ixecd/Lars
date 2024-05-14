/**
 * @file message.hpp
 * @author qc
 * @brief 解决TCP粘包问题,规定使用的协议
 * @version 0.1
 * @date 2024-04-28
 *
 * @copyright Copyright (c) 2024
 *
 */

#pragma once

#include <unordered_map>
#include "tcp_conn.hpp"
#include "net_connection.hpp"

//  Id  DataLen    Data      Id  DataLen    Data
// | --head-- | |--body --| | --head-- | |--body--|

namespace qc {

#define MESSAGE_HEAD_LEN 8
#define MESSAGE_LENGTH_LIMIT (65535 - MESSAGE_HEAD_LEN) //2^16 -> 两个字节

/// @brief 用来解决tcp粘包问题 -> id + len 一共8个字节
struct msg_head {
    int msgid;
    int msglen;
};

// msg 业务回调函数原型
/// @brief 信息回调函数 用户开发者自己在业务上注册的回调业务函数 -> net_connection -> tcp_client - tcp_conn(tcp_server)
using msg_callback = std::function<void(const char *data, uint32_t len, int msgid, net_connection *client, void *user_data)>;

// ============= 消息分发路由机制 =============
/**
 * @details
 * 其中这里面第4个参数，只能是tcp_client类型的参数，也就是我们之前的设计的msg_callback只支持tcp_client的消息回调机制，
 * 但是很明显我们的需求是不仅是tcp_client要用，tcp_server中的tcp_conn也要用到这个机制，那么很显然这个参数在这就不是很合适，
 * 那么如果设定一个形参既能指向tcp_client又能能指向tcp_conn两个类型呢，当然答案就只能是将这两个类抽象出来一层，用父类指针指向
 * 子类然后通过多态特性来调用就可以了，所以我们需要先定义一个抽象类。
 */
class msg_router {
public:
    msg_router(){
        printf("msg router init...\n");
    }

    // 给一个消息ID注册一个对应的回调业务函数
    int register_msg_router(int msgid, msg_callback msg_cb, void *user_data = nullptr) {
        if (_router.find(msgid) != _router.end()) return -1;
        printf("add msg cb msgid = %d\n", msgid);
        _router[msgid] = msg_cb;
        _args[msgid] = user_data;
        return 0;
    }

    // 调用注册的对应的回调业务函数
    void call(int msgid, uint32_t msglen, const char *data, net_connection *client) {
        //printf("call msgid = %d\n", msgid);
        if (_router.find(msgid) == _router.end()) {
            fprintf(stderr, "msgid %d is not register!\n", msgid);
            return;
        }
        // void(const char *data, uint32_t len, int msgid, tcp_conn *conn, void
        // *user_data
        msg_callback callback = _router[msgid];
        void *user_data = _args[msgid];
        callback(data, msglen, msgid, client, user_data);
        //printf("=======\n");
    }

private:
    /// @brief key -> msgID, value -> 回调函数
    std::unordered_map<int, msg_callback> _router;
    /// @brief key -> msgID, value -> 参数
    std::unordered_map<int, void*> _args;
};


}  // namespace qc
