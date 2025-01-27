#include <string.h>

#include <iostream>

#include <lars_dns/dns_route.hpp>
#include <proto/lars.pb.h>
#include <lars_reactor/lars_reactor.hpp>
using namespace qc;

// 命令行参数
struct Option { 
    Option() : ip(nullptr), port(0) {}
    char *ip;
    short port;
};

Option option;

void Usage() { printf("Usage: ./lars_dns_test -h ip -p port\n"); }

// 解析命令行
void parse_option(int argc, char **argv) {
    for (int i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "-h") == 0)
            option.ip = argv[i + 1];
        else if (strcmp(argv[i], "-p") == 0)
            option.port = atoi(argv[i + 1]);
    }
    if (!option.ip || !option.port) {
        Usage();
        exit(1);
    }
}

void on_connection(net_connection *conn, void *args) {
    // 发送Route请求信息
    lars::GetRouteRequest req;
    req.set_modid(1);
    req.set_cmdid(1);

    std::string requestStirng;
    req.SerializeToString(&requestStirng);
    conn->send_message(requestStirng.c_str(), requestStirng.size(),
                       lars::ID_GetRouteRequest);
}

// 下面是发送完请求服务端返回给客户端的hosts信息,由host来处理
void deal_get_route(const char *data, uint32_t len, int msgid,
                    net_connection *conn, void *args) {
    // 1.解包得到数据
    lars::GetRouteResponse rep;
    rep.ParseFromArray(data, len);
    uint32_t modid = rep.modid(), cmdid = rep.cmdid();
    // 2.打印数据
    std::cout << "modid = " << modid << " cmdid = " << cmdid << std::endl;
    std::cout << "hosts.size() = " << rep.host_size() << std::endl;

    for (int i = 0; i < rep.host_size(); ++i) {
        std::cout << "--> ip = " << (uint32_t)rep.host(i).ip() << std::endl;
        std::cout << "--> port = " << rep.host(i).port() << std::endl;
    }

    // 3.处理完数据,再请求
    // lars::GetRouteRequest req;
    // req.set_modid(rep.modid());
    // req.set_cmdid(rep.cmdid());
    // std::string requestString;
    // req.SerializeToString(&requestString);
    // conn->send_message(requestString.c_str(), requestString.size(),
    //                    lars::ID_GetRouteRequest);
}

int main(int argc, char **argv) {
    // 1. 解析命令行参数
    // parse_option(argc, argv);

    event_loop loop;

    tcp_client *client =
        new tcp_client(&loop, "127.0.0.1", 7775, "lars_dns_client");

    qc_assert(client != nullptr);

    client->set_conn_start(on_connection);

    client->add_msg_router(lars::ID_GetRouteResponse, deal_get_route);

    loop.event_process();

    return 0;
}