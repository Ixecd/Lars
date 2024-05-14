/**
 * @file reactor_buf.hpp
 * @author qc
 * @brief 读写buffer机制
 * @version 0.1
 * @date 2024-04-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef __REACTOR_BUF_HPP__
#define __REACTOR_BUF_HPP__

#include <unistd.h>

#include "buf_pool.hpp"
#include "io_buf.hpp"
#include "qc.hpp"

namespace qc {

/**
 * @brief 提供给业务层,封装io_buf
 * @details 反应堆的buf只提供length pop clear方便业务端操作
 */
class reactor_buf {
public:
    reactor_buf();
    ~reactor_buf();

    const int length() const;
    void pop(int len);
    void clear();
protected:
    /// @brief 降低耦合
    io_buf *_buf;
};

//读(输入) 缓存buffer
class input_buf : public reactor_buf {
public:
    //从一个fd中读取数据到reactor_buf中
    int read_data(int fd);

    //取出读到的数据
    const char *data() const;

    //重置缓冲区
    void adjust();
};

//写(输出)  缓存buffer
class output_buf : public reactor_buf {
public:
    //将一段数据 写到一个reactor_buf中
    int send_data(const char *data, int datalen);

    //将reactor_buf中的数据写到一个fd中
    int write2fd(int fd);
};
}


#endif