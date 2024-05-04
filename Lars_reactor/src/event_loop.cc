/**
 * @file event_loop.cc
 * @author qc
 * @brief 实现io_event的基本增删操作
 * @version 0.1
 * @date 2024-04-27
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "event_loop.hpp"

#include "qc.hpp"

namespace qc {

event_loop::event_loop() {
    _epfd = epoll_create(5000);
    qc_assert(_epfd != -1);
}

// 阻塞循环处理函数
// read事件一直监听,write事件触发一次删除一次
void event_loop::event_process() {
    while (true) {
        io_event_map_it ev_it;
        int nfds = epoll_wait(_epfd, _fired_evs, MAXEVENTS, 10);
        for (int i = 0; i < nfds; i++) {
            //通过触发的fd找到对应的绑定事件
            ev_it = _io_evs.find(_fired_evs[i].data.fd);
            qc_assert(ev_it != _io_evs.end());

            io_event *ev = &(ev_it->second);

            if (_fired_evs[i].events & EPOLLIN) {
                //读事件，掉读回调函数
                void *args = ev->rcb_args;
                ev->read_callback(this, _fired_evs[i].data.fd, args);
            } else if (_fired_evs[i].events & EPOLLOUT) {
                //写事件，掉写回调函数
                void *args = ev->wcb_args;
                ev->write_callback(this, _fired_evs[i].data.fd, args);
            } else if (_fired_evs[i].events & (EPOLLHUP | EPOLLERR)) {
                //水平触发未处理，可能会出现HUP事件，正常处理读写，没有则清空
                if (ev->read_callback != nullptr) {
                    void *args = ev->rcb_args;
                    ev->read_callback(this, _fired_evs[i].data.fd, args);
                } else if (ev->write_callback != nullptr) {
                    void *args = ev->wcb_args;
                    ev->write_callback(this, _fired_evs[i].data.fd, args);
                } else {
                    //删除
                    fprintf(stderr, "fd %d get error, delete it from epoll\n",
                            _fired_evs[i].data.fd);
                    this->del_io_event(_fired_evs[i].data.fd);
                }
            }
        }
    }
}

/*
 * 这里我们处理的事件机制是
 * 如果EPOLLIN 在mask中， EPOLLOUT就不允许在mask中
 * 如果EPOLLOUT 在mask中， EPOLLIN就不允许在mask中
 * 如果想注册EPOLLIN|EPOLLOUT的事件， 那么就调用add_io_event() 方法两次来注册。
 * */

//添加一个io事件到loop中
void event_loop::add_io_event(int fd, io_callback proc, int mask, void *args) {
    int final_mask;
    int op;

    //  找到当前fd是否已经有事件
    io_event_map_it it = _io_evs.find(fd);
    
    op = it == _io_evs.end()
             ? (final_mask = mask, EPOLL_CTL_ADD)
             : (final_mask = it->second.mask | mask, EPOLL_CTL_MOD);

    // 注册回调函数
    if (mask & EPOLLIN) {
        //读事件回调函数注册
        _io_evs[fd].read_callback = proc;
        _io_evs[fd].rcb_args = args;
    } else if (mask & EPOLLOUT) {
        _io_evs[fd].write_callback = proc;
        _io_evs[fd].wcb_args = args;
    }

    // epoll_ctl添加到epoll堆里
    _io_evs[fd].mask = final_mask;
    //创建原生epoll事件
    struct epoll_event event;
    event.events = final_mask;
    event.data.fd = fd;
    qc_assert(epoll_ctl(_epfd, op, fd, &event) != -1);

    // 将fd添加到监听集合中
    _listen_fds.insert(fd);
}

//删除一个io事件从loop中
void event_loop::del_io_event(int fd) {
    //将事件从_io_evs删除
    _io_evs.erase(fd);

    //将fd从监听集合中删除
    _listen_fds.erase(fd);

    //将fd从epoll堆删除
    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, NULL);
}

//删除一个io事件的EPOLLIN/EPOLLOUT
void event_loop::del_io_event(int fd, int mask) {
    //如果没有该事件，直接返回
    io_event_map_it it = _io_evs.find(fd);
    if (it == _io_evs.end()) {
        return;
    }

    int &o_mask = it->second.mask;
    //修正mask
    o_mask = o_mask & (~mask);

    if (o_mask == 0) {
        //如果修正之后 mask为0，则删除
        this->del_io_event(fd);
    } else {
        //如果修正之后，mask非0，则修改
        struct epoll_event event;
        event.events = o_mask;
        event.data.fd = fd;

        int rt = epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event);
        qc_assert(rt != -1);
    }
}

}  // namespace qc