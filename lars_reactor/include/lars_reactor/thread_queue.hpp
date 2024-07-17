/**
 * @file thread_queue.hpp
 * @author qc
 * @brief 消息队列封装,每个线程都有自己的消息队列
 * @version 0.1
 * @date 2024-05-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <queue>
#include <functional>
#include <sys/eventfd.h>
#include <lars_reactor/event_loop.hpp>
#include <lars_reactor/task_msg.hpp>
#include <lars_reactor/mutex.hpp>
#include <lars_reactor/qc.hpp>

namespace qc {


/// @brief 这里使用泛型编程,任务队列中的任务可以是不同种类的
/// @tparam 类型
/// @details 每个线程都有一个自己的消息队列
template <class T>
class thread_queue {
public: 
    typedef Mutex MutexType;
    thread_queue() {
        _loop = nullptr;
        /// @brief 创建一个事件文件描述符_evfd,用于在后续的程序中进行事件通知,并设置了非阻塞的读取模式。
        ///        主要用来在多线程或多进程中进行事件通知和同步
        ///         前面这个参数是计数器,read减少,write增加,计数器的值发生变化的时候,就可以出发事件.
        _evfd = eventfd(0, EFD_NONBLOCK);

        qc_assert(_evfd != -1);
    }
    
    ~thread_queue() {
        close(_evfd);
    }
    /// @brief 向任务队列中添加一个任务
    void send(const T& task) {
        /// @brief 消息事件的占位传输内容
        unsigned long long idle_num = 1;
        /// @brief 范围锁
        MutexType::Lock lock(_mutex); 
        _queue.push(task);

        int ret = write(_evfd, &idle_num, sizeof(unsigned long long));
        qc_assert (ret != -1);

    }
    /// @brief 获取当前队列中已有的任务
    void recv(std::queue<T>& new_queue) {
        unsigned long long idle_num = 1;

        MutexType::Lock lock(_mutex);
        int ret = read(_evfd, &idle_num, sizeof(unsigned long long));
        qc_assert(ret != -1);
        // 将任务队列中的所有任务放到new_queue中,_queue置空
        std::swap(new_queue, _queue);
    }
    /// @brief 设置当前thread_queue是被哪个事件触发event_loop监控
    void set_loop(event_loop *loop) {
        _loop = loop;
    }
    /// @brief 设置任务队列中每个任务触发的回调函数
    void set_callback(io_callback cb, void *args = nullptr) {
        if (_loop != nullptr) 
            _loop->add_io_event(_evfd, cb , EPOLLIN, args);
        else printf("cur _loop is nullptr !\n");
    }
    /// @brief 获取当前消息队列的loop
    event_loop *get_loop() {
        return _loop;
    }
    
private:
    /// @brief 触发消息任务队列读取的每个消息业务的fd,事件对象
    /// @details 为了触发消息队列消息到达,处理该消息的作用,将_evfd添加到对应的线程event_loop中,然后通过set_callback设置通用的
    ///          该queue全部消息锁出发的处理业务call_back,在这个call_back里开发者可以实现一些自定义的业务流程
    int _evfd;
    /// @brief 当前消息队列绑定在哪一个event_loop事件触发机制中
    /// @details 触发消息到达,捕获消息并且触发处理消息业务的动作
    event_loop *_loop;
    /// @brief 任务队列
    std::queue<T> _queue;
    /// @brief 静态初始化_mutex 锁属性为PTHREAD_MUTEX_NORMAL,这里的_mutex是一个class,构造函数调用的时候锁就初始化好了
    MutexType _mutex;
};


}

