/**
 * @file store_threads.hpp
 * @author qc
 * @brief 为了避免服务器调用io操作浪费CPU资源,创建相应线程池调度io事件
 * @version 0.1
 * @date 2024-05-19
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#pragma once

namespace qc {

void thread_report(event_loop *loop, int fd, void *args);

void *store_main(void *args);


}