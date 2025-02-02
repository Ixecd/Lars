#include <unistd.h>
#include <memory>
#include <lars_reactor/thread_pool.hpp>
#include <lars_reactor/event_loop.hpp>
#include <lars_reactor/tcp_conn.hpp>

// 考虑到RAII
// 规定只在创建的时候make_shared一次,后面都用get,裸指针
// 因为一般没人手动delete,是为了防止内存泄漏,并且同时兼顾了内存开销,同时还保留了Cpp指针特色
// 良好的Cpp程序是不能出现 new 和 delete 的

namespace qc {
/**
 * @brief 一旦有task任务过来,就会执行这个函数,这个函数时处理task消息业务的主流程
 *        只要有人调用thread_queue::send()方法就会触发这个函数
 * @details 添加NEW_TASKS之后在这里应该处理一下
 */
void deal_task_message(event_loop *loop, int fd, void *args) {
    thread_queue<task_msg>* queue = (thread_queue<task_msg>*) args;

    std::queue<task_msg> tasks;

    queue->recv(tasks);

    while (!tasks.empty()) {
        // 这里直接依赖默认的拷贝构造函数就行
        task_msg task = tasks.front();
        tasks.pop();

        if (task.type == task_msg::NEW_CONN) {
            // 此时已经将对应的connfd绑定到消息队列上的事件循环
            // MD!
            // std::shared_ptr<tcp_conn> conn = std::make_shared<tcp_conn>(task.connfd, loop);
            tcp_conn *conn = new tcp_conn(task.connfd, loop);
            qc_assert(conn != nullptr);
            printf("[thread] : get new connection succ!\n");
        } else if (task.type == task_msg::NEW_TASK) {
            // TODO
            // 直接将这个task加入到epoll中即可
            loop->add_task(task.task_cb, task.args);    
        } else {
            fprintf(stderr, "unknow task!\n");
        }
    }
}

// 一个线程的主业务main函数
void *thread_main(void *args) {
    thread_queue<task_msg> *queue = (thread_queue<task_msg> *)args;

    // 每个线程都应该有一个event_loop来监控客户端连接的读写事件
    // event_loop *loop = new event_loop();
    std::shared_ptr<event_loop> loop = std::make_shared<event_loop>();
    // 不推荐下面的写法,相比于上面要多申请一次内存
    // ! std::shared_ptr<event_loop> loop(new event_loop());
    qc_assert(loop != nullptr);

    queue->set_loop(loop.get());
    // 后面的参数时任务队列
    queue->set_callback(deal_task_message, queue);
    loop->event_process();
    return nullptr;
}

// ===== 处理异步任务 =====
void thread_pool::send_task(task_func func, void *args) {
    task_msg task;

    // 给当前每个thread都发送当前任务
    for (int i = 0; i < _thread_cnt; ++i) {
        task.type = task_msg::NEW_TASK;
        task.task_cb = func;
        task.args = args;
        // 一个thread对应一个消息队列,这里由于要发送给所有线程
        // 要拿到所有消息队列
        thread_queue<task_msg> *queue = _queues[i].get();
        queue->send(task);
    }
}

thread_pool::thread_pool(size_t threads) {
    qc_assert(threads > 0);
    _index = 0;
    _queues.resize(threads);
    _thread_cnt = threads;
    _threads.resize(threads);

    // 任务队列的个数要和线程池中线程的数量一致
    // printf("cur threads = %d \n", threads);
    
    // _queues = new thread_queue<task_msg>*[threads];
    // _tids = new pthread_t[threads];
    _tids.resize(threads);
    int rt;
    for (size_t i = 0; i < threads; ++i) {
        // 创建一个线程
        printf("create %ld thread\n", i);
        // 给当前线程创建一个任务消息队列
        // 一个thread_queue<task_msg>中单独有一个evfd和loop
        _queues[i] = std::make_shared<thread_queue<task_msg>>();
        // _queues[i] = new thread_queue<task_msg>();
        qc_assert(_queues[i] != nullptr);

        rt = pthread_create(&_tids[i], nullptr, thread_main, _queues[i].get());
        // _threads.emplace_back(std::thread(std::bind(thread_main, _queues[i].get())));


        // 线程脱离
        pthread_detach(_tids[i]);
        // _threads[i].detach();
    }
}

thread_queue<task_msg>* thread_pool::get_thread() {
	// 这里修改为_index轮询发送消息
    if (_index == _thread_cnt) _index = 0;
    else _index++;
    return _queues[_index].get();
}

}
