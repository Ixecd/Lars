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
#include <lars_reactor/lars_reactor.hpp>
#include <lars_dns/dns_route.hpp>
#include <lars_dns/subscribe.hpp>
#include <unordered_set>
#include <proto/lars.pb.h>

#include "mysql.h"

using namespace qc;
// using namespace co_async;
// server 不在 namespace 中
tcp_server *server;
using client_sub_list = std::unordered_set<uint64_t>;

void create_subscribe(net_connection *conn, void *args) {
    conn->param = new client_sub_list;
}

void clear_subscribe(net_connection *conn, void *args) {
    client_sub_list::iterator it;
    for (it = ((client_sub_list *)(conn->param))->begin();
         it != ((client_sub_list *)(conn->param))->end(); ++it) {
        // 下面退订阅
        uint64_t mod = *it;
        // SubscribeList::GetInstance()->unsubscribe(mod, conn->get_fd());
        GetInstance<SubscribeList>()->unsubscribe(mod, conn->get_fd());
    }
    client_sub_list *sub_list = (client_sub_list *)conn->param;
    delete sub_list;
    // delete conn->param;
    conn->param = nullptr;
}


void get_route(const char *data, uint32_t len, int msgid, net_connection *conn,
               void *user_data) {
    // 1. 解析proto文件
    lars::GetRouteRequest req;

    req.ParseFromArray(data, len);

    // 2. 得到modid和cmdid
    int modid = req.modid(), cmdid = req.cmdid();

    printf("modid = %u , cmdid = %u\n", modid, cmdid);

    // 2.5 如果之前没有订阅过这个modid/cmdid 订阅
    // 一个conn对应一个client_sub_list->存储了客户端对应订阅的modid和cmdid信息,也就是说订阅信息存储在客户端
    uint64_t mod = ((uint64_t)modid << 32) + cmdid;
    client_sub_list *sub_list = (client_sub_list *)conn->param;


    if (sub_list == nullptr)
        std::cout << "cur client_sub_list is nullptr" << std::endl;
    if (sub_list->find(mod) == sub_list->end()) {
        sub_list->insert(mod);
        // !!TM的问题在这里
        // 这里只是简单上锁解锁啊??
        // ok 问题进一步缩小,出现在subscribe上
        int fd = conn->get_fd();
        // SubscribeList::GetInstance()->subscribe(mod, fd);
        // ptr->subscribe(mod, fd);  // err
        GetInstance<SubscribeList>()->subscribe(mod, fd); // ok
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
        host.set_ip((uint32_t)(ip_port >> 32));
        host.set_port((uint32_t)(ip_port));
        std::cout << (uint32_t)(ip_port >> 32) << ' ' << (uint32_t)ip_port
                  << std::endl;
        rep.add_host()->CopyFrom(host);
    }

    // 6.发送给客户端
    std::string responseString;
    rep.SerializeToString(&responseString);

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
    std::cout << "dns_server ip = " << ip.c_str() << " port = " << port
              << std::endl;

    // 创建好server之后直接注册Hook函数
    server->set_conn_start(create_subscribe);
    server->set_conn_close(clear_subscribe);

    // 注册路由信息
    server->add_msg_router(lars::ID_GetRouteRequest, get_route);

    // -------- 开启 Backend Thread 实现周期性更新RouteData --------
    // 这里是单独开辟一个线程来实现
    // pthread_t tid;
    // int rt = pthread_create(&tid, nullptr, check_route_change, nullptr);
    // qc_assert(rt != -1);

    // // 设置线程分离
    // pthread_detach(tid);

    GetInstance<SubscribeList>();
    Route::GetInstance();

    // 测试mysql接口
    // MYSQL dbconn;
    // mysql_init(&dbconn);
    // Route::GetInstance()->connect_db();
    // printf("connect mysql success!\n");
    // sleep(1);
    // Route::GetInstance()->build_maps();
    // printf("build map success!\n");

    printf("lars dns service ...\n");
    loop.event_process();

    return 0;
}
