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
#include <iostream>

#include "subscribe.hpp"
#include "dns_route.hpp"
#include "lars.pb.h"
#include "lars_reactor.hpp"
#include "mysql.h"

using namespace qc;

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
            SubscribeList::GetInstance()->unsubscribe(mod, conn->get_fd());
    }

    delete conn->param;
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

    // 3. 根据modid和cmdid获取host ip 和 port 信息
    host_set hosts = Route::GetInstance()->get_hosts(modid, cmdid);

    // 4. 将数据打包成protobuf
    lars::GetRouteResponse rep;
    rep.set_modid(modid), rep.set_cmdid(cmdid);

    // 5. 真正获取host ip port 对应的信息
    for (host_set_iterator it = hosts.begin(); it != hosts.end(); ++it) {
        // -> int32 ip, int32 port
        // -> uint64_t ip_port
        uint64_t ip_port = *it;
        lars::HostInfo host;
        host.set_ip((uint32_t)(ip_port >> 32));
        host.set_port((uint32_t)(ip_port));
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

    // 加载配置信息
    config_file::setPath("../conf/lars.conf");
    std::string ip =
        config_file::GetInstance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::GetInstance()->GetNumber("reactor", "port", 9876);

    tcp_server *server = new tcp_server(&loop, ip.c_str(), port);

    // 创建好server之后直接注册Hook函数
    server->set_conn_start(create_subscribe);
    server->set_conn_close(clear_subscribe);

    server->add_msg_router(lars::ID_GetRouteRequest, get_route);

    Route* r = Route::GetInstance();

    // 测试mysql接口
    // MYSQL dbconn;
    // mysql_init(&dbconn);
    // Route::GetInstance()->connect_db();
    // printf("connect mysql success!\n");
    // sleep(1);
    // Route::GetInstance()->build_maps();
    // printf("build map success!\n");

    // 注册路由信息
    printf("lars dns service ...\n");
    loop.event_process();

    return 0;
}
