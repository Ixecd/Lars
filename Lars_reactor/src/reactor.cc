/**
 * @file reactor.cc
 * @author qc
 * @brief 提供给业务层的buf封装
 * @version 0.1
 * @date 2024-04-26
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "reactor_buf.hpp"
#include "qc.hpp"
#include <sys/ioctl.h>
#include <string.h>

namespace qc {

reactor_buf::reactor_buf() { _buf = nullptr; }

reactor_buf::~reactor_buf() { clear(); }

const int reactor_buf::length() const {
    return _buf != nullptr ? _buf->m_length : 0;
}

void reactor_buf::pop(int len) {
    qc_assert(_buf != nullptr && len <= _buf->m_length);

    _buf->pop(len);

    //printf("cur _buf->length = %d\n", _buf->m_length);
    //当此时_buf的可用长度已经为0
    if (_buf->m_length == 0) {
        //将_buf重新放回buf_pool中
        buf_pool::instance()->revert(_buf);
        _buf = nullptr;
    }
}

void reactor_buf::clear() {
    if (_buf != nullptr) {
        //将_buf重新放回buf_pool中
        buf_pool::instance()->revert(_buf);
        _buf = nullptr;
    }
}

//从一个fd中读取数据到reactor_buf中
int input_buf::read_data(int fd) {
    int need_read;  //硬件有多少数据可以读

    //一次性读出所有的数据
    //需要给fd设置FIONREAD,
    //得到read缓冲中有多少数据是可以读取的
    if (ioctl(fd, FIONREAD, &need_read) == -1) {
        fprintf(stderr, "ioctl FIONREAD\n");
        return -1;
    }

    if (_buf == NULL) {
        //如果io_buf为空,从内存池申请
        _buf = buf_pool::instance()->alloc_buf(need_read);
        if (_buf == NULL) {
            fprintf(stderr, "no idle buf for alloc\n");
            return -1;
        }
    } else {
        //如果io_buf可用，判断是否够存
        // 这里_buf->m_head == 0 表示所有的数据都已经处理完了.
        qc_assert(_buf->m_head == 0);
        if (_buf->m_capacity - _buf->m_length < (int)need_read) {
            //不够存，内存池申请
            io_buf *new_buf =
                buf_pool::instance()->alloc_buf(need_read + _buf->m_length);
            if (new_buf == NULL) {
                fprintf(stderr, "no ilde buf for alloc\n");
                return -1;
            }
            //将之前的_buf的数据考到新申请的buf中
            new_buf->copy(_buf);
            //将之前的_buf放回内存池中
            buf_pool::instance()->revert(_buf);
            //新申请的buf成为当前io_buf
            _buf = new_buf;
        }
    }

    //读取数据
    int already_read = 0;
    do {
        //读取的数据拼接到之前的数据之后
        if (need_read == 0) {
            //可能是read阻塞读数据的模式，对方未写数据
            already_read = read(fd, _buf->m_data + _buf->m_length, m4K);
        } else {
            already_read = read(fd, _buf->m_data + _buf->m_length, need_read);
        }
    } while (already_read == -1 &&
             errno == EINTR);  // systemCall引起的中断 继续读取
    if (already_read > 0) {
        if (need_read != 0) {
            qc_assert(already_read == need_read);
        }
        _buf->m_length += already_read;
    }

    return already_read;
}

//取出读到的数据
const char *input_buf::data() const {
    return _buf != NULL ? _buf->m_data + _buf->m_head : NULL;
}

//重置缓冲区
void input_buf::adjust() {
    if (_buf != NULL) {
        _buf->adjust();
    }
}

//将一段数据 写到一个reactor_buf中
int output_buf::send_data(const char *data, int datalen) {
    if (_buf == NULL) {
        //如果io_buf为空,从内存池申请
        _buf = buf_pool::instance()->alloc_buf(datalen);
        if (_buf == NULL) {
            fprintf(stderr, "no idle buf for alloc\n");
            return -1;
        }
    } else {
        //如果io_buf可用，判断是否够存
        //printf("cur _buf->m_head = %d\n", _buf->m_head);
        qc_assert(_buf->m_head == 0);
        if (_buf->m_capacity - _buf->m_length < datalen) {
            //不够存，冲内存池申请
            io_buf *new_buf =
                buf_pool::instance()->alloc_buf(datalen + _buf->m_length);
            if (new_buf == NULL) {
                fprintf(stderr, "no ilde buf for alloc\n");
                return -1;
            }
            //将之前的_buf的数据考到新申请的buf中
            new_buf->copy(_buf);
            //将之前的_buf放回内存池中
            buf_pool::instance()->revert(_buf);
            //新申请的buf成为当前io_buf
            _buf = new_buf;
        }
    }

    //将data数据拷贝到io_buf中,拼接到后面
    memcpy(_buf->m_data + _buf->m_length, data, datalen);
    _buf->m_length += datalen;

    return 0;
}

//将reactor_buf中的数据写到一个fd中
int output_buf::write2fd(int fd) {
    // !! _buf->m_head == 0; _buf->m_head = 6 ??
    qc_assert(_buf != NULL && _buf->m_head == 0);

    int already_write = 0;

    do {
        already_write = write(fd, _buf->m_data, _buf->m_length);
    } while (already_write == -1 &&
             errno == EINTR);  // systemCall引起的中断，继续写

    if (already_write > 0) {
        //已经处理的数据清空
        _buf->pop(already_write);
        //未处理数据前置，覆盖老数据
        _buf->adjust();
    }

    //如果fd非阻塞，可能会得到EAGAIN错误
    if (already_write == -1 && errno == EAGAIN) {
        already_write = 0;  //不是错误，仅仅返回0，表示目前是不可以继续写的
    }

    return already_write;
}
}