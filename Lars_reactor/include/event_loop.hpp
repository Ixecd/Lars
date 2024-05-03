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
#ifndef __EVENT_LOOL_HPP__
#define __EVENT_LOOL_HPP__

#include <sys/epoll.h>
#include <unordered_map>
#include <unordered_set>
#include "event_base.hpp"

#define MAXEVENTS 128

namespace qc {

typedef std::unordered_map<int, io_event> io_event_map;

typedef std::unordered_map<int, io_event>::iterator io_event_map_it;

typedef std::unordered_set<int> listen_fd_set;

class event_loop {
public:
    /// @brief 构造函数,初始化epoll堆
    event_loop();
    /// @brief 阻塞循环处理事件
    void event_process();
    /// @brief 添加一个io事件到loop中
    void add_io_event(int fd, io_callback* proc, int mask, void *args = nullptr);
    /// @brief 删除一个io事件从loop中
    void del_io_event(int fd);
    /// @brief 删除一个io事件的EPOLLIN/EPOLLOUT
    void del_io_event(int fd, int mask);

private:
    /// @brief epoll句柄
    int _epfd;
    /// @brief io_event hash_map
    io_event_map _io_evs;
    /// @brief fd array
    listen_fd_set _listen_fds;
    /// @brief epoll_event
    struct epoll_event _fired_evs[MAXEVENTS];
};


}


#endif