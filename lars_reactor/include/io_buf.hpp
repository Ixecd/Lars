/**
 * @file io_buf.hpp
 * @author qc
 * @brief 内存管理和buffer封装
 * @details 这里没有提供delete真是删除物理内存的方法，因为这里的buffer设计是不需要清理的，接下来是用一个buf_pool来管理全部未被使用的
 *          io_buf集合。而且buf_pool的管理的内存是程序开始预开辟的，不会做清理工作.
 * @version 0.1
 * @date 2024-04-26
 *
 * @copyright Copyright (c) 2024
 */
#ifndef __IO_BUF_HPP__
#define __IO_BUF_HPP__

namespace qc {

/**
 * @brief buffer存放数据的结构
 */
class io_buf {
public:
    io_buf(int capacity);
    /// @brief 清除buffer中的内容
    void clear();
    /// @brief 将other处的buffer copy 到当前data中
    /// @param other 
    void copy(const io_buf *other);
    /// @brief pop长度为len的数据
    /// @param len 
    void pop(int len);
    /// @brief 将已经处理过的数据，清空,将未处理的数据提前至数据首地址
    void adjust();

    /// @brief 如果有多个buffer,采用链表链接起来
    io_buf *next;
    /// @brief buffer容量大小
    int m_capacity;
    /// @brief buffer有效数据长度
    int m_length;
    /// @brief 未处理数据的头部位置索引,head之前的都是处理过的
    int m_head;
    /// @brief 当前io_buf所保存的数据地址
    char *m_data;
};

}  // namespace qc

#endif