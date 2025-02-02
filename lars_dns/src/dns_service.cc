/**
 * @file dns_service.cc
 * @author qc
 * @brief 实现针对ID_GetRouteRequest信息指令的业务
 * @version 0.3
 * @date 2024-05-14
 *
 * @copyright Copyright (c) 2024
 *
 */
#include <proto/lars.pb.h>

#include <cstring>
#include <lars_dns/dns_route.hpp>
#include <lars_dns/subscribe.hpp>
#include <lars_reactor/lars_reactor.hpp>
#include <set>
#include <string>

#include "mysql.h"

using namespace qc;

tcp_server *server;

using client_sub_list = std::set<uint64_t>;

void create_subscribe(net_connection *conn, void *args) {
    conn->param = (void *)new client_sub_list;
}

void clear_subscribe(net_connection *conn, void *args) {
    client_sub_list::iterator it;
    client_sub_list *sub_list = (client_sub_list *)conn->param;

    for (it = sub_list->begin(); it != sub_list->end(); it++) {
        uint64_t mod = *it;
        SubscribeList::GetInstance()->unsubscribe(mod, conn->get_fd());
    }

    delete sub_list;
}

void get_route(const char *data, uint32_t len, int msgid, net_connection *conn,
               void *user_data) {
    // 1. 解析proto文件
    lars::GetRouteRequest req;

    req.ParseFromArray(data, len);

    // 2. 得到modid和cmdid
    uint modid = req.modid(), cmdid = req.cmdid();

    std::cout << "[GetRouteRequest] modid = " << modid << ", cmdid = " << cmdid
              << std::endl;

    // 2.5 如果之前没有订阅过这个modid/cmdid 订阅
    // 一个conn对应一个client_sub_list->存储了客户端对应订阅的modid和cmdid信息
    uint64_t mod = ((uint64_t)modid << 32) + cmdid;

    client_sub_list *sub_list = (client_sub_list *)(conn->param);
    qc_assert(sub_list != nullptr);
    if (sub_list->find(mod) == sub_list->end()) {
        sub_list->insert(mod);

        // std::cout << "==============" << std::endl;

        // std::cout << "[subList size] " << sub_list->size() << std::endl; // 1

        // std::cout << "==============" << std::endl;

        // !!TM的问题在这里
        // 这里只是简单上锁解锁啊??
        // ok 问题进一步缩小,出现在subscribe上
        SubscribeList::GetInstance()->subscribe(mod, conn->get_fd());
        // ptr->subscribe(mod, fd);  // err
        // GetInstance<SubscribeList>()->subscribe(mod, fd); // ok
    }

    // 3. 根据modid和cmdid获取host ip 和 port 信息
    host_set hosts = Route::GetInstance()->get_hosts(modid, cmdid);

    // 4. 将数据打包成protobuf
    lars::GetRouteResponse rep;
    rep.set_modid(modid);
    rep.set_cmdid(cmdid);

    // 5. 真正获取host ip port 对应的信息
    for (host_set_iterator it = hosts.begin(); it != hosts.end(); ++it) {
        // -> int32 ip, int32 port
        // -> uint64_t ip_port
        uint64_t ip_port = *it;
        lars::HostInfo host;
        host.set_ip((uint)(ip_port >> 32));
        host.set_port((uint)(ip_port));
        std::cout << "[GetRouteResponse] ip = " << (uint)(ip_port >> 32)
                  << ", port = " << (uint)(ip_port) << std::endl;
        rep.add_host()->CopyFrom(host);
    }

    // 6.发送给客户端
    std::string responseString;
    rep.SerializeToString(&responseString);

    // 取出链接信息
    // net_connection *_conn = tcp_server::conns[fd];
    // if (_conn)
    //     _conn->send_message(responseString.c_str(),
    //                                responseString.size(),
    //                                lars::ID_GetRouteResponse);

    std::cout << "[get_route()]" << conn << " " << conn->param << std::endl;

    conn->send_message(responseString.c_str(), responseString.size(),
                       lars::ID_GetRouteResponse);
}

int main(int argc, char **argv) {
    event_loop loop;

    config_file::setPath("/home/qc/Lars/lars_dns/conf/lars.conf");
    // 输出所有配置信息
    // config_file::GetInstance()->get_all_info(config_file::GetInstance());

    std::string ip = config_file_instance::GetInstance()->GetString(
        "reactor", "ip", "0.0.0.0");
    short port =
        config_file_instance::GetInstance()->GetNumber("reactor", "port", 9876);
    // 创建好server之后会自动创建5个线程
    // server只负责accept,执行到accept函数的时候已经建立好链接了,只不过要封装成tcp_conn,之后将相应的文件描述符注册到线程池中的一个消息队列的事件循环上即可,之后通信都是在消息队列上
    server = new tcp_server(&loop, ip.c_str(), port);

    std::cout << "[dns_server]" << " ip = " << ip << " port = " << port
              << std::endl;

    // 创建好server之后直接注册Hook函数
    server->set_conn_start(create_subscribe);
    server->set_conn_close(clear_subscribe);

    // 注册路由信息
    server->add_msg_router(lars::ID_GetRouteRequest, get_route);

    // -------- 开启 Backend Thread 实现周期性更新RouteData --------
    // 这里是单独开辟一个线程来实现
    pthread_t tid;
    int rt = pthread_create(&tid, nullptr, check_route_change, nullptr);
    qc_assert(rt != -1);

    // 设置线程分离
    pthread_detach(tid);

    std::cout << "[dns_server] start..." << std::endl;

    loop.event_process();

    return 0;
}
