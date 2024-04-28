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

// DataLen  Id    Data        DataLen Id   Data
// | --head-- | |--body --| | --head-- | |--body--|

namespace qc {

#define MESSAGE_HEAD_LEN 8
#define MESSAGE_LENGTH_LIMIT (65535 - MESSAGE_HEAD_LEN)

struct msg_head {
    int msgid;
    int msglen;
};

}  // namespace qc

#endif