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

#include <lars_reactor/event_loop.hpp>
#include <lars_reactor/qc.hpp>
// 集成协程 + 定时器
#include <chrono>
#include <optional>
// #include <co_async/timerLoop.hpp>

namespace qc {

event_loop::event_loop() {
    _epfd = epoll_create1(0);
    qc_assert(_epfd != -1);
}

// 阻塞循环处理函数
// read事件一直监听,write事件触发一次删除一次
void event_loop::event_process() {
    while (true) {

        // // 先判断定时器中是否有任务, 每个loop都有单独的一个定时器
        // int time_out = 100; // 默认等待100ms
        // if (co_async::getTimerLoop().hasTimer()) {
        //     std::optional<std::chrono::system_clock::duration> opt = co_async::getTimerLoop().getNext();
        //     if (opt.has_value()) {
        //         auto t = opt.value();
        //         time_out = t.count();
        //           //std::chrono::duration_cast<std::chrono::duration<int, std::milli>(t).count();
        //     }
        // }

        io_event_map_it ev_it;
        int nfds = epoll_wait(_epfd, _fired_evs, MAXEVENTS, 1000);

        // // 先处理定时器,里面所有任务都是协程,会把超时的都执行完,之后返回下一个定时器的时间/如果没有返回nullopt;
        // co_async::getTimerLoop().run();

        for (int i = 0; i < nfds; i++) {
            // 通过判断data.ptr来判断是否是一个协程句柄
            // 普通IO事件不会设置data.ptr
            // if (_fired_evs[i].data.ptr != nullptr) {
            //     // 当前监听的事件对应的体是一个协程句柄
            //     auto &event = _fired_evs[i];
            //     auto &promise = *(EventFilePromise *)_fired_evs[i].data.ptr;
            //     //checkError(epoll_ctl(_epfd, EPOLL_CTL_DEL, promise.mFd, nullptr));
            //     // 下面promise 对应的协程句柄执行完就会析构掉promise,也就会析构掉事件
            //     // 这个promise就剩下一个省略的co_return了
            //     // std::coroutine_handle<EventFilePromise>::from_promise(promise).resume();
            //     // 先开香槟,后触发,再返回
            //     promise.mAwaiter->mResumeEvents = event.events;
            //     // resume结束之后就会调用EventFilePromise对应的析构函数,删除事件
            //     std::coroutine_handle<EventFilePromise>::from_promise(promise).resume();
            //     // 不走下面
            //     continue;
            // }
            //通过触发的fd找到对应的绑定事件
            ev_it = _io_evs.find(_fired_evs[i].data.fd);
            qc_assert(ev_it != _io_evs.end());

            io_event *ev = &(ev_it->second);

            if (_fired_evs[i].events & EPOLLIN) {
                //读事件，读回调函数
                void *args = ev->rcb_args;
                ev->read_callback(this, _fired_evs[i].data.fd, args);
            } 
            if (_fired_evs[i].events & EPOLLOUT) {
                //写事件，掉写回调函数
                void *args = ev->wcb_args;
                ev->write_callback(this, _fired_evs[i].data.fd, args);
            }  
            if (_fired_evs[i].events & (EPOLLHUP | EPOLLERR)) {
                //水平触发未处理，可能会出现HUP事件，正常处理读写，没有则清空
                //HUP也有可能是由于对端关闭导致的
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

        // 执行任务函数在每次event_loop执行完一组fd之后触发一次额外的处理任务函数
        this->execute_ready_tasks();
    }
}

/// @brief 添加一个io事件到loop中,一次只能添加一个类型,如果要两个,那就调用两次
void event_loop::add_io_event(int fd, io_callback proc, int mask, void *args) {
    //  找到当前fd是否已经有事件
    io_event_map_it it = _io_evs.find(fd);
    int final_mask;
    int op = it == _io_evs.end()
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
    int rt = epoll_ctl(_epfd, op, fd, &event);
    qc_assert(rt != -1);

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
    epoll_ctl(_epfd, EPOLL_CTL_DEL, fd, nullptr);
}

//删除一个io事件的EPOLLIN/EPOLLOUT
void event_loop::del_io_event(int fd, int mask) {
    //如果没有该事件，直接返回
    io_event_map_it it = _io_evs.find(fd);
    if (it == _io_evs.end()) return;

    int &io_mask = it->second.mask;
    //修正mask
    io_mask = io_mask & (~mask);

    if (io_mask == 0) {
        //如果修正之后 mask为0，则删除
        this->del_io_event(fd);
    } else {
        //如果修正之后，mask非0，则修改
        struct epoll_event event;
        event.events = io_mask;
        event.data.fd = fd;

        int rt = epoll_ctl(_epfd, EPOLL_CTL_MOD, fd, &event);
        qc_assert(rt != -1);
    }
}

// ---------- 一般任务处理 ----------
void event_loop::add_task(task_func func, void *args) {
    task_func_pair func_pair(func, args);
    _ready_tasks.push_back(func_pair);
}

void event_loop::execute_ready_tasks() {
    // std::vector<task_func_pair>::iterator it;
    for (auto it = _ready_tasks.begin(); it != _ready_tasks.end(); ++it) {
        task_func func = it->first;
        void *args = it->second;
        func(this, args);
    }
    _ready_tasks.clear();
}

// ---------- 处理协程 ------------
// inline
// EventFilePromise::~EventFilePromise() {
//     mAwaiter->mLoop.removeListener(mAwaiter->mFd);
// }

// inline bool 
// event_loop::addListener(EventFilePromise &promise, int op) {
//     struct epoll_event event;
//     // EPOLLONSHOT 只触发一次 -> 避免对同一个fd重复添加相同event
//     event.events = promise.mAwaiter->mEvents;
//     event.data.ptr = &promise;
//     // 服务器一旦运行起来,如果你下面直接抛出异常了,那还服务啥??
//     // checkError(epoll_ctl(_epfd, EPOLL_CTL_ADD, promise.mAwaiter->mFd, &event));
//     int rt = epoll_ctl(_epfd, op, promise.mAwaiter->mFd.fileNo(), &event);
//     if (rt == -1) return false;
//     if (op == EPOLL_CTL_ADD) _count++;
//     return true;
// }

// inline bool 
// event_loop::tryRun(std::optional<std::chrono::system_clock::duration> timeout) {
//     return true;
// }
}  // namespace qc
