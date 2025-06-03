#ifndef __MUTEX_HPP__
#define __MUTEX_HPP__

#include <pthread.h>
#include <semaphore.h>

#include <atomic>
#include <cstdint>  // For uint32_t ...
#include <functional>
#include <mutex>

#include <lars_reactor/noncopyable.hpp>

namespace qc {

/**
 * @brief 信号量
 *        P V Light
 */
class Semaphore : public Noncopyable {
public:
    Semaphore(uint32_t count = 0);

    ~Semaphore();
    // P
    void wait();
    // V
    void notify();

private:
    sem_t m_semaphore;
};

/**
 * @brief 局部锁的模板实现,传进来一把锁(什么类型都可以)
 *        RAII 初始化加锁 析构解锁
 */
template <class T>
class ScopedLockImpl {
public:
    ScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.lock();  // 穿进来的锁的自己的实现
        m_locked = true;
    }
    ~ScopedLockImpl() {
        unlock();  // ScopedLockImpl中的unlock();
    }

    void lock() {
        if (m_locked == false) {
            m_mutex.lock();
            m_locked = true;
        }
    }

    void unlock() {
        if (m_locked) {
            m_mutex.unlock();
            m_locked = false;
        }
    }

private:
    T& m_mutex;
    bool m_locked;
};

/**
 * @brief 局部读锁模板实现,只上读锁
 */
template <class T>
class ReadScopedLockImpl {
public:
    ReadScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.rdlock();
        is_locked = true;
    }
    ~ReadScopedLockImpl() { unlock(); }
    void lock() {
        if (!is_locked) {
            m_mutex.rdlock();
            is_locked = true;
        }
    }
    void unlock() {
        if (is_locked) {
            m_mutex.unlock();
            is_locked = false;
        }
    }

private:
    T& m_mutex;  // 这里必须是引用,要对同一把锁进行操作
    bool is_locked;
};

/**
 * @brief 局部写锁模板实现,只关心写
 */
template <class T>
class WriteScopedLockImpl {
public:
    WriteScopedLockImpl(T& mutex) : m_mutex(mutex) {
        m_mutex.wrlock();
        is_locked = true;
    }
    ~WriteScopedLockImpl() { unlock(); }
    void lock() {
        if (!is_locked) {
            m_mutex.wrlock();
            is_locked = true;
        }
    }
    void unlock() {
        if (is_locked) {
            m_mutex.unlock();
            is_locked = false;
        }
    }

private:
    T& m_mutex;
    bool is_locked;
};

/**
 * @brief 封装互斥量Mutex
 */
class Mutex : public Noncopyable {
public:
    typedef ScopedLockImpl<Mutex> Lock;
    Mutex() { pthread_mutex_init(&m_mutex, nullptr); }
    ~Mutex() { pthread_mutex_destroy(&m_mutex); }
    void lock() { pthread_mutex_lock(&m_mutex); }
    void unlock() { pthread_mutex_unlock(&m_mutex); }

private:
    pthread_mutex_t m_mutex;
};
/**
 * @brief 空锁，用于调试
 */
class NullMutex : Noncopyable {
public:
    typedef ScopedLockImpl<NullMutex> Lock;
    NullMutex() {}
    ~NullMutex() {}
    void lock() {}
    void unlock() {}
};

/**
 * @brief 读写锁
 */
class RWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<RWMutex> ReadLock;
    typedef WriteScopedLockImpl<RWMutex> WriteLock;
    RWMutex() { pthread_rwlock_init(&m_lock, nullptr); }
    ~RWMutex() { pthread_rwlock_destroy(&m_lock); }
    void rdlock() { pthread_rwlock_rdlock(&m_lock); }
    void wrlock() { pthread_rwlock_wrlock(&m_lock); }
    void unlock() { pthread_rwlock_unlock(&m_lock); }

private:
    pthread_rwlock_t m_lock;
};

/**
 * @brief 空读写锁,用于调试
 */
class NullRWMutex : Noncopyable {
public:
    typedef ReadScopedLockImpl<NullMutex> ReadLock;
    typedef WriteScopedLockImpl<NullMutex> WriteLock;
    NullRWMutex() {}
    ~NullRWMutex() {}
    void rdlock() {}
    void wrlock() {}
    void unlock() {}
};

/**
 * @brief 自旋锁,CPU空转
 */
class Spinlock : Noncopyable {
public:
    typedef ScopedLockImpl<Spinlock> Lock;
    Spinlock() {
        // the sec par if nonzero different process can shared the same spinlock
        pthread_spin_init(&m_mutex, 0);
    }
    ~Spinlock() { pthread_spin_destroy(&m_mutex); }
    void lock() { pthread_spin_lock(&m_mutex); }
    void unlock() { pthread_spin_unlock(&m_mutex); }

private:
    pthread_spinlock_t m_mutex;
};

/**
 * @brief 原子锁CAS
 * @details 本质上是test and set, 存储了一个状态
 */
class CASLock : Noncopyable {
public:
    typedef ScopedLockImpl<CASLock> Lock;
    CASLock() { m_mutex.clear(); }
    ~CASLock() {}

    void lock() {
        while (std::atomic_flag_test_and_set_explicit(
            &m_mutex, std::memory_order_acquire))
            ;
    }
    void unlock() {
        std::atomic_flag_clear_explicit(&m_mutex, std::memory_order_release);
    }

private:
    /// 原子状态
    volatile std::atomic_flag m_mutex;
};

}  // namespace qc

#endif