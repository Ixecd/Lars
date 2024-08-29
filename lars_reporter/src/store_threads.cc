/**
 * @file store_threads.cc
 * @author qc
 * @brief 存储线程池的实现
 * @version 0.1
 * @date 2024-05-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include <proto/lars.pb.h>
#include <lars_reactor/lars_reactor.hpp>
#include <lars_reporter/store_report.hpp>

namespace qc {

struct Args {
    thread_queue<lars::ReportStatusReq> *first;
    StoreReport *second;
};

void thread_report(event_loop *loop, int fd, void *args) {
    // 1.从queue中取出需要report的数据
    thread_queue<lars::ReportStatusReq> *queue = ((Args *)args)->first;
    StoreReport *sr = ((Args *)args)->second;
    std::queue<lars::ReportStatusReq> report_message;
    // 2. 从消息队列中取出全部的消息元素集合
    queue->recv(report_message);
    while (!report_message.empty()) {
        lars::ReportStatusReq msg = report_message.front();
        report_message.pop();

        // 将数据存储到Mysql中
        sr->store(msg);
    }
}

void *store_main(void *args) {
    // 1.得到对应的thread_queue
    thread_queue<lars::ReportStatusReq> *queue =
        (thread_queue<lars::ReportStatusReq> *)args;

    // 2.定义事件触发机制
    event_loop loop;

    // 3.定义一个存储对象
    StoreReport sr;

    // 4.定义io_callback的参数
    Args callback_args;
    callback_args.first = queue;
    callback_args.second = &sr;

    // 5.设置当前消息队列绑定的loop
    queue->set_loop(&loop);

    // 6.设置当前消息队列绑定的回调函数
    queue->set_callback(thread_report, &callback_args);

    // 7.开启事件监听
    loop.event_process();

    // 8.对于比较旧的函数,记得返回值
    return nullptr;
}
}  // namespace qc