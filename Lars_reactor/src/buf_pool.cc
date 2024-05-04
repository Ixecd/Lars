/**
 * @file buf_pool.cc
 * @author qc
 * @brief 内存池管理模板封装
 * @version 0.1
 * @date 2024-04-26
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "buf_pool.hpp"

#include <stdlib.h>

#include "qc.hpp"

namespace qc {

buf_pool *buf_pool::_instance = nullptr;

pthread_once_t buf_pool::_once = PTHREAD_ONCE_INIT;

pthread_mutex_t buf_pool::_mutex = PTHREAD_MUTEX_INITIALIZER;

buf_pool::buf_pool() : _total_mem(0) {
    io_buf *prev;

    // 开辟4KB buf内存池
    _pool[m4K] = new io_buf(m4K);
    qc_assert(_pool[m4K]);

    prev = _pool[m4K];
    // 4kB -> 5000个 20MB
    for (int i = 0; i < 5000; ++i) {
        prev->next = new io_buf(m4K);
        qc_assert(prev->next);
        prev = prev->next;
    }
    _total_mem += 4 * 5000;

    // 开辟16KB buf 内存池
    _pool[m16K] = new io_buf(m16K);
    qc_assert(_pool[m16K]);

    prev = _pool[m16K];
    // 16KB -> 1000个 16MB
    for (int i = 0; i < 1000; ++i) {
        prev->next = new io_buf(m16K);
        qc_assert(prev->next);
        prev = prev->next;
    }
    _total_mem += 16 * 1000;

    // 开辟64KB buf 内存池
    _pool[m64K] = new io_buf(m64K);
    qc_assert(_pool[m64K]);
    prev = _pool[m64K];
    // 64KB -> 500个 32MB
    for (int i = 0; i < 500; ++i) {
        prev->next = new io_buf(m64K);
        qc_assert(prev->next);
        prev = prev->next;
    }
    _total_mem += 64 * 500;

    // 开辟256KB buf 内存池
    _pool[m256K] = new io_buf(m256K);
    qc_assert(_pool[m256K]);
    prev = _pool[m256K];
    // 256KB -> 200个 50MB
    for (int i = 0; i < 200; ++i) {
        prev->next = new io_buf(m256K);
        qc_assert(prev->next);
        prev = prev->next;
    }
    _total_mem += 256 * 200;

    // 开辟1M buf 内存池
    _pool[m1M] = new io_buf(m1M);
    qc_assert(_pool[m1M]);
    prev = _pool[m1M];
    // 1M -> 50个 50MK
    for (int i = 0; i < 50; ++i) {
        prev->next = new io_buf(m1M);
        qc_assert(prev->next);
        prev = prev->next;
    }
    _total_mem += 1024 * 50;

    // 开辟4M buf 内存池
    _pool[m4M] = new io_buf(m4M);
    qc_assert(_pool[m4M]);

    prev = _pool[m4M];
    // 4M -> 20个 80MB
    for (int i = 1; i < 20; i++) {
        prev->next = new io_buf(m4M);
        qc_assert(prev->next);
        prev = prev->next;
    }
    _total_mem += 4096 * 20;

    // 开辟8M buf 内存池
    _pool[m8M] = new io_buf(m8M);
    qc_assert(_pool[m8M]);

    prev = _pool[m8M];
    // 8M -> 10个 80MB
    for (int i = 1; i < 10; i++) {
        prev->next = new io_buf(m8M);
        qc_assert(prev->next);
        prev = prev->next;
    }
    _total_mem += 8192 * 10;
}

/**
 * @brief 开辟一个io_buf
 * @details 1 如果上层需要N个字节的大小的空间，找到与N最接近的bufhash组，取出
            2 如果该组已经没有节点使用，可以额外申请
            3 总申请长度不能够超过最大的限制大小 EXTRA_MEM_LIMIT
            4 如果有该节点需要的内存块，直接取出，并且将该内存块从pool摘除
 * @param N 需要的字节数
 * @return io_buf* umap中取出来一个
 */
io_buf *buf_pool::alloc_buf(int N) {
    int index;
    if (N <= m4K) {
        index = m4K;
    } else if (N <= m16K) {
        index = m16K;
    } else if (N <= m64K) {
        index = m64K;
    } else if (N <= m256K) {
        index = m256K;
    } else if (N <= m1M) {
        index = m1M;
    } else if (N <= m4M) {
        index = m4M;
    } else if (N <= m8M) {
        index = m8M;
    } else {
        return NULL;
    }
    pthread_mutex_lock(&_mutex);
    if (_pool[index] == nullptr) {
        if (_total_mem + index / 1024 >= MAX_MEM_LIMIT) {
            fprintf(stderr, "already use too many memory!\n");
            exit(1);
        }
        // 可以再分配一点, 这时候不需要修改_pool因为用户用完之后会返回来
        io_buf *new_buf = new io_buf(index);
        qc_assert(new_buf);
        _total_mem += index / 1024;
        pthread_mutex_unlock(&_mutex);
        return new_buf;
    }

    // 从_pool中摘除该内存块
    io_buf *target = _pool[index];
    _pool[index] = target->next;
    pthread_mutex_unlock(&_mutex);

    target->next = nullptr;

    return target;
}

// 重置一个io_buf,将一个buf返回到_pool中
void buf_pool::revert(io_buf *buffer) {
    int index = buffer->m_capacity;

    buffer->m_length = 0, buffer->m_head = 0;

    pthread_mutex_lock(&_mutex);
    qc_assert(_pool.find(index) != _pool.end());

    buffer->next = _pool[index];
    _pool[index] = buffer;
    pthread_mutex_unlock(&_mutex);
}

}  // namespace qc
