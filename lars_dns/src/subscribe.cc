/**
 * @file subscribe.cc
 * @author qc
 * @brief Route 订阅
 * @version 0.1
 * @date 2024-05-15
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "subscribe.hpp"

/// @brief 声明一变量,保证程序执行过程中只有一个实例
extern qc::tcp_server *server;

using namespace qc;
// tcp_server *server;

SubscribeList::SubscribeList() {}

void SubscribeList::subscribe(uint64_t mod, int fd) {
    mutex_book_list.lock();
    _book_list[mod].insert(fd);
    mutex_book_list.unlock();
}

void SubscribeList::unsubscribe(uint64_t mod, int fd) {
    mutex_book_list.lock();
    _book_list[mod].erase(fd);
    if (_book_list[mod].empty() == true) _book_list.erase(mod);
    mutex_book_list.unlock();
}

void SubscribeList::make_publish_map(listen_fd_set &online_fds,
                                     publish_map &need_publish) {
    // 遍历_push_list 找到对应的online_fds,然后放到need_publish中
    mutex_push_list.lock();
    for (auto it = _push_list.begin(); it != _push_list.end(); ++it) {
        // 需要publish的订阅列表在online_fds中找到了
        if (online_fds.find(it->first) != online_fds.end()) {
            need_publish[it->first] = _push_list[it->first];
            // _push_list是一次性的
            _push_list.erase(it);
        }
    }
    mutex_push_list.unlock();
}

void push_change_task(event_loop *loop, void *args) {
    std::cout << "run in push_change_task..." << std::endl;
    if (loop == nullptr)
        std::cout << "cur loop is nullptr" << std::endl;
    else
        std::cout << "cur loop is not nullptr" << std::endl;

    if (args == nullptr)
        std::cout << "cur args is nullptr" << std::endl;
    else
        std::cout << "cur args is not nullptr" << std::endl;

    SubscribeList *subscribe = (SubscribeList *)args;

    // 1. 获取全部的在线客户端fd
    // 在event_loop中一个epoll实例对应一个listen_fd_set 记录监听的文件描述符
    listen_fd_set online_fds;
    loop->get_listen_fds(online_fds);

    publish_map need_publish;
    // 2. 把当前epoll监听的所有文件描述符对应的订阅信息全部放到need_publish中
    subscribe->make_publish_map(online_fds, need_publish);

    // 3. 依次从need_publish去除数据发送给对应客户端连接
    for (auto it = need_publish.begin(); it != need_publish.end(); ++it) {
        int fd = it->first;
        // 遍历当前文件描述符对应的modid/cmdid
        for (auto st = it->second.begin(); st != it->second.end(); ++st) {
            // modid_cmdid uint64_t
            // modid uint32_t
            // cmdid uint32_t
            uint modid = (uint)((*st) >> 32);
            uint cmdid = uint(*st);

            // 组装pb消息,发送给客户

            lars::GetRouteResponse rep;
            rep.set_modid(modid);
            rep.set_cmdid(cmdid);

            // 通过route查询对应的host ip/port信息,进行组装
            host_set hosts = Route::GetInstance()->get_hosts(modid, cmdid);
            for (auto hit = hosts.begin(); hit != hosts.end(); ++hit) {
                uint64_t ip_port = *hit;
                lars::HostInfo host_info;
                host_info.set_ip((uint32_t)(ip_port >> 32));
                host_info.set_port((uint)ip_port);
                rep.add_host()->CopyFrom(host_info);
            }

            std::string responseString;
            rep.SerializeToString(&responseString);

            // 取出链接信息
            net_connection *conn = tcp_server::conns[fd];
            if (conn)
                conn->send_message(responseString.c_str(),
                                   responseString.size(),
                                   lars::ID_GetRouteResponse);
        }
    }
}

/// @brief 当前modid/cmdid被修改了,要通知所有订阅了这些服务的客户端
void SubscribeList::publish(std::vector<uint64_t> &change_mods) {
    // 1.将change_mods已经修改的mod->fd
    // 放到push_list清单中
    mutex_book_list.lock();
    mutex_push_list.lock();

    bool tickle = false;
    for (uint64_t &mod : change_mods) {
        if (_book_list.find(mod) != _book_list.end()) {
            // 将mod下面的fd set拷贝到 _push_list中
            // 遍历mod对应的所有客户
            tickle = true;
            for (auto fds_it = _book_list[mod].begin();
                 fds_it != _book_list[mod].end(); ++fds_it) {
                int fd = *fds_it;
                _push_list[fd].insert(mod);
            }
        }
    }
    mutex_push_list.unlock();
    mutex_book_list.unlock();

    std::cout << "cur tickle = " << tickle << std::endl;

    // 最后通知server的各个线程去执行
    if (tickle) (server->get_thread_pool())->send_task(push_change_task, this);
}

// namespace qc