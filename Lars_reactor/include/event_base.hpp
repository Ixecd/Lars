/**
 * @file event_base.hpp
 * @author qc
 * @brief IO事件封装类
 * @version 0.1
 * @date 2024-04-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef __EVENT_BASE_HPP__
#define __EVENT_BASE_HPP__

#include <functional>

namespace qc {

class event_loop;

using io_callback = std::function<void(event_loop*, int, void*)>;

class io_event {
public:
    io_event():read_callback(nullptr), write_callback(nullptr), rcb_args(nullptr),
    wcb_args(nullptr) {}
    int mask;                   // EPOLLIN EPOLLOUT
    io_callback read_callback;  // EPOLLIN
    io_callback write_callback; // EPOLLOUT
    void *rcb_args;             // read_callback args
    void *wcb_args;             // write_callbck args
};

}


#endif