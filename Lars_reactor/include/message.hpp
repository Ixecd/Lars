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

#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include "tcp_conn.hpp"

// DataLen  Id    Data        DataLen Id   Data
// | --head-- | |--body --| | --head-- | |--body--|

namespace qc {

#define MESSAGE_HEAD_LEN 8
#define MESSAGE_LENGTH_LIMIT (65535 - MESSAGE_HEAD_LEN)

struct msg_head {
    int msgid;
    int msglen;
};

// msg 业务回调函数原型
/// @brief 信息回调函数 用户开发者自己在业务上注册的回调业务函数
using msg_callback = std::function<void(const char *data, uint32_t len, int msgid, tcp_conn *conn, void *user_data)>;

// typedef void msg_callback(const char *data, uint32_t len, int msgid,
//                           tcp_conn *conn, void *user_data);

}  // namespace qc

#endif