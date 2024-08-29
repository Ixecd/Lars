/**
 * @file event_base.hpp
 * @author qc
 * @brief IO事件基本封装类
 * @version 0.1
 * @date 2024-04-27
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <functional>
// 集成协程
// #include <co_async/task.hpp>
namespace qc {

class event_loop;

using io_callback = std::function<void(event_loop*, int, void*)>;

/**
 * @details 一个IO事件其属性应该有 
 *          mask --> 事件类型
 *          rcb  --> 读回调函数
 *          rags --> 读回调参数
 *          wcb  --> 写回调函数
 *          wags --> 写回调参数
 */     
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
