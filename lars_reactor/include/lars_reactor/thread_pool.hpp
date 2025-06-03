#pragma once

#include <pthread.h>
#include <lars_reactor/task_msg.hpp>
#include <lars_reactor/thread_queue.hpp>
#include <vector>
#include <thread>
namespace qc {

class thread_pool {
public:
    /// @brief 初始化线程池,创建threads个线程
    /// @details 用thread_pool来为每个thread所绑定的event_pool中去发送task任务
    thread_pool(size_t threads);
    /// @brief 获取一个thread的消息队列,里面有_queue包含任务
    thread_queue<task_msg>* get_thread();

    // ==== 发送一个task给thread_pool里的全部thread ====
    void send_task(task_func func, void *args = nullptr);
private:
    /// @brief 当前所有线程的任务队列
    // std::vector<thread_queue<task_msg>*> _queues;
    std::vector<std::shared_ptr<thread_queue<task_msg>>> _queues;
    // thread_queue<task_msg> **_queues;
    /// @brief 线程池中线程的数量
    int _thread_cnt;
    /// @brief 已经启动的线程的所有编号
    // pthread_t *_tids;
    std::vector<pthread_t> _tids;
    /// @brief 当前选中的线程在_queues中的下标
    int _index;
    /// @brief 线程池
    std::vector<std::thread> _threads;

};



}