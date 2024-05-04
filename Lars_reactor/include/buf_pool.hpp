/**
 * @file buf_pool.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2024-04-26
 *
 * @copyright Copyright (c) 2024
 *
 */
#ifndef __BUF_POOL_HPP__
#define __BUF_POOL_HPP__

#include <stdint.h>
#include <thread>
#include <unordered_map>

#include "io_buf.hpp"
#include "singleton.hpp"

// 总内存池最大限制 Kb 5GB
#define MAX_MEM_LIMIT (5U * 1024 * 1024)

namespace qc {
/// @brief 每个key下面挂一个io_buf链表,会预先开辟好一定数量的内存块,上层取一块,就从这里头去掉一块,用完再放回来
typedef std::unordered_map<int, io_buf *> pool_t;
// typedef Singleton<buf_pool> _instance;

enum MEM_CAP {
    m4K = 4096,
    m16K = 16384,
    m64K = 65536,
    m256K = 262144,
    m1M = 1048576,
    m4M = 4194304,
    m8M = 8388608
};
/**
 * @brief 单例模式
 */
class buf_pool {
public:
    // 初始化单例对象
    static void init() {
        _instance = new buf_pool();
    }
    // 获取单例方法
    static buf_pool *instance() {
        // 保证init在整个进程中只被执行一次
        pthread_once(&_once, init);
        return _instance;
    }
    /// @brief 开辟一个io_buf
    /// @param size
    /// @return 
    io_buf *alloc_buf(int size);
    io_buf *alloc_buf() { return alloc_buf(m4K); }
    /// @brief 重置一个buffer
    /// @param buffer 
    void revert(io_buf *buffer);

    
private:
    buf_pool();

    // 拷贝构造私有化
    buf_pool(const buf_pool&);
    const buf_pool& operator = (const buf_pool&);

private:
    /// @brief k-v方式,所有buffer的一个map集合句柄
    pool_t _pool;
    /// @brief 总buffer池的内存大小,单位为KB
    uint64_t _total_mem;
    /// @brief 单例模式
    static buf_pool *_instance;
    /// @brief 用于保证创建单例的init方法只执行一次的锁
    static pthread_once_t _once;
    /// @brief 用于保存内存池链表修改的互斥锁
    static pthread_mutex_t _mutex;
};

}  // namespace qc

#endif
