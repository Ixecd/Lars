#include "lars_api.hpp"
#include <iostream>

using namespace qc;

void usage() {
    std::cout << "usage: ./example [modid] [cmdid]\n";
}

int main(int argc, char **argv) {
    if (argc != 3) {
        usage();
        return 1;
    }

    int modid = atoi(argv[1]);
    int cmdid = atoi(argv[2]);

    std::string ip;
    int port;

    lars_client api;
    
    route_set route;
    int rt = api.get_route(modid, cmdid, route);
    if (rt == 0) {
        std::cout << "get route succ!" << std::endl;
        // 直接把信息输出出来
        for (route_set_it it = route.begin(); it != route.end(); ++it) {
            std::cout << "host ip = " << (*it).first << ", port = " << (*it).first << std::endl;
        }
    }

    rt = api.get_host(modid, cmdid, ip, port);
    if (rt == 0) {
        std::cout << "host is " << ip << " : " << port << std::endl;
        // TODO 上报调用结果
        // 下面就是配置report业务的参数信息
        api.report(modid, cmdid, ip, port, rt);
    }
    return 0;
}


