/**
 * @file io_buf.cc
 * @author qc
 * @brief io_buffer封装
 * @version 0.1
 * @date 2024-04-26
 * 
 * @copyright Copyright (c) 2024
 *
 */

#include <string.h>
#include "io_buf.hpp"
#include "qc.hpp"
namespace qc {

io_buf::io_buf(int capacity)
    : m_capacity(capacity), next(nullptr), m_length(0), m_head(0), m_data(nullptr) {
        m_data = new char[capacity];
        qc_assert(m_data);
    }

// 不用删除,直接覆盖,效率最好
void io_buf::clear() {
    m_length = m_head = 0;
}

void io_buf::adjust() {
    if (m_head) {
        if (m_length) {
            memmove(m_data, m_data + m_head, m_length);
        }
        m_head = 0;
    }
}

void io_buf::copy(const io_buf *other) {
    memcpy(m_data, other->m_data + other->m_head, other->m_length);
    m_head = 0;
    m_length = other->m_length;
}
/// @brief 这里pop,将有效长度增长,head后移
/// @param len 
void io_buf::pop(int len) {
    m_length -= len;
    m_head += len;
}

}  // namespace qc
