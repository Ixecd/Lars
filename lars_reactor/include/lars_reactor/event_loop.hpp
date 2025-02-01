/**
 * @file event_loop.hpp
 * @author qc
 * @brief 实现io_event的基本增删操作
 * @version 0.1
 * @date 2024-04-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <sys/epoll.h>
#include <unordered_map>
#include <unordered_set>
#include <functional>
#include <coroutine>
#include <vector>
#include <lars_reactor/event_base.hpp>
#include <lars_reactor/task_msg.hpp>
// !集成协程
// 协程应该是独立的,不应该和这里的事件循环绑定
// #include <co_async/ioLoop.hpp>
// #include <co_async/timerLoop.hpp>

#define MAXEVENTS 128

namespace qc {

class event_loop;

/// @brief 定义异步任务回调函数类型
typedef void (*task_func)(event_loop *loop, void *args);
// using task_func = std::function<void(event_loop *loop, void *args)>;

using io_event_map = std::unordered_map<int, io_event>;

using io_event_map_it = std::unordered_map<int, io_event>::iterator;

using listen_fd_set = std::unordered_set<int>;

class event_loop {
public:
    // 只保留默认构造函数
    event_loop& operator = (event_loop&&) = delete;

public:
    /// @brief 构造函数,初始化epoll
    event_loop();
    /// @brief 阻塞循环处理事件
    void event_process();
    /// @brief 添加一个io事件到loop中
    void add_io_event(int fd, io_callback proc, int mask, void *args = nullptr);
    /// @brief 删除一个io事件从loop中
    void del_io_event(int fd);
    /// @brief 删除一个io事件的EPOLLIN/EPOLLOUT
    void del_io_event(int fd, int mask);
    /// @brief 获取全部监听文件描述符
    void get_listen_fds(listen_fd_set &fds) { fds = _listen_fds; }

    // === 异步任务task模板 === 
    /// @brief 添加一个任务到任务队列中
    void add_task(task_func func ,void *args);
    /// @brief 执行所有准备好的任务
    void execute_ready_tasks();

private:
    /// @brief epoll句柄
    int _epfd;
    /// @brief 记录事件总数
    int _count;
    /// @brief io_event hash_map
    io_event_map _io_evs;
    /// @brief fd array
    listen_fd_set _listen_fds;
    /// @brief epoll_event
    struct epoll_event _fired_evs[MAXEVENTS];
    // === 异步任务队列 ===
    /// @brief 需要被执行的任务
    typedef std::pair<task_func, void*> task_func_pair;

    std::vector<task_func_pair> _ready_tasks;
};

}

