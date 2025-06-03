#pragma once

#include <functional>
#include <list>

#include <lars_reactor/event_loop.hpp>

namespace qc {

// 不要在匿名联合体中使用class
// using task_cb = std::function<void(event_loop*, void*)>;

/// @brief 定义异步任务回调函数类型
typedef void (*task_func)(event_loop *loop, void *args);


struct task_msg {
    enum TASK_TYPE {
        NEW_CONN,  // 新建链接的任务
        NEW_TASK,  // 一般任务 -> 允许服务端去执行某项具体任务,服务端主动发送的业务给客户端
    };

    TASK_TYPE type;

    /// @brief 任务的参数
    union {
        /// @brief 针对NEW_CONN新建链接的任务,需要对应的socket
        int connfd;  // 4

        struct {
            // DO NOT USE std::function<void()>
            // void (*task_cb)(event_loop *loop, void *args);  // 8
            task_func task_cb;
            void *args;                                     // 8
        };
    };
};

}  // namespace qc