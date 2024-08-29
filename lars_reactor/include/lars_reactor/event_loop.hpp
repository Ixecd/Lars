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
// 集成协程
// #include <co_async/ioLoop.hpp>
// #include <co_async/timerLoop.hpp>

#define MAXEVENTS 128

namespace qc {

class event_loop;

// struct EventFilePromise : co_async::Promise<std::uint32_t> {

//     auto get_return_object() {
//         return std::coroutine_handle<EventFilePromise>::from_promise(*this);
//     }

//     ~EventFilePromise();

//     EventFilePromise& operator=(EventFilePromise&&) = delete;

//     // Promise和IoLoop相互依赖,所以这里用指针,并且析构函数要放下实现,还要记得+inline
//     // struct event_loop *mLoop;
//     // int mFd;
//     // uint32_t mEvents;

//     // 直接记录Awaiter,由Awaiter记录信息
//     struct EventFileAwaiter *mAwaiter{};
// };

/// @brief 定义异步任务回调函数类型
typedef void (*task_func)(event_loop *loop, void *args);
// using task_func = std::function<void(event_loop *loop, void *args)>;

using io_event_map = std::unordered_map<int, io_event>;

using io_event_map_it = std::unordered_map<int, io_event>::iterator;

using listen_fd_set = std::unordered_set<int>;

// 这里实现的事件循环只是针对于普通函数而言
// 要集成函数的话,不能简单直接继承IoLoop,因为即使同名也会被子类隐藏,但是依然存在
// 也就是说如果 IoLoop中的_epfd和event_loop中的_epfd重名,那么IoLoop中的就没有办法用到(除非特别指定)
// 所以我们的设计如下
// 每个事件循环都有两个epoll,一个专门用来监听普通成员,另一个专门用来监听协程函数
// 这样的设计好不好? 目前如果不是所有的函数都是协程,那么是好的
// 但是对于IO来说,其只涉及read和write这样的话,所有的read和write都应该被hook成协程函数
// 那么只使用IoLoop是好的
// 但是考虑到epoll只是监听事件,如何触发回调函数和执行协程,是由程序员自己编写实现
// 所以最好的解决办法是融合在一起

// 总结
// 这里的设计 最好是将两者直接整合在一起, qc::event_loop 重新实现 co_async::event_loop
class event_loop {
public:
    // bool addListener(EventFilePromise &promise, int op);

    // void removeListener(co_async::AsyncFile &file) {
    //     co_async::checkError(epoll_ctl(_epfd, EPOLL_CTL_DEL, file.fileNo(), nullptr));
    //     --_count;
    // }

    // bool hasEvent() const noexcept { return _count != 0; }

    // bool tryRun(std::optional<std::chrono::system_clock::duration> timeout = std::nullopt);

    // void process() {
    //     while (1) {
    //         bool rt = tryRun(1s);
    //         if (rt == false) break;
    //     }
    // }

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

// struct EventFileAwaiter {

//     bool await_ready() const noexcept { return false; }

//     void await_suspend(std::coroutine_handle<EventFilePromise> coroutine) {
//         auto &promise = coroutine.promise();
//         promise.mAwaiter = this;
//         if (!mLoop.addListener(promise, op)) {
//             // 添加失败
//             promise.mAwaiter = nullptr;
//             coroutine.resume();
//         }
//     }

//     uint32_t await_resume() const noexcept {
//         return mResumeEvents;
//     }

//     using ClockType = std::chrono::system_clock;

//     event_loop &mLoop;
//     // int mFd;
//     co_async::AsyncFile &mFd;
//     uint32_t mEvents;
//     uint32_t mResumeEvents = 0;
//     int op = EPOLL_CTL_ADD;
// };




}

