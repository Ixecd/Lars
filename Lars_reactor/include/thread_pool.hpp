/**
 * @file thread_pool.hpp
 * @author qc
 * @brief 线程池封装
 * @version 0.1
 * @date 2024-05-06
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

#include <pthread.h>
#include <task_msg.hpp>
#include <thread_queue.hpp>
#include <vector>
namespace qc {

class thread_pool {
public:
    /// @brief 初始化线程池,创建threads个线程
    thread_pool(size_t threads);
    /// @brief 获取一个thread的消息队列
    thread_queue<task_msg>* get_thread();

private:
    /// @brief 当前所有线程的任务队列
    // std::vector<thread_queue<task_msg>*> _queues;
    thread_queue<task_msg> **_queues;
    /// @brief 线程池中线程的数量
    int _thread_cnt;
    /// @brief 已经启动的线程的所有编号
    pthread_t *_tids;
    /// @brief 当前选中的线程在_queues中的下标
    int _index;

};



}