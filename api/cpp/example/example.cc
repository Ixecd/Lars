// #include "lars_api.hpp"
#include <lars_api/lars_api.hpp>
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

    // 初始化一次
    int rt;
    rt = api.reg_init(modid, cmdid);

    if (rt != 0) {
        std::cout << "modid " << modid << ", cmdid " << cmdid << " still not exist host, after register, ret = " << rt << std::endl;
    }
    
    route_set route;
    rt = api.get_route(modid, cmdid, route);
    if (rt == 0) {
        std::cout << "get route succ!" << std::endl;
        // 直接把信息输出出来
        for (route_set_it it = route.begin(); it != route.end(); ++it) {
            std::cout << "host ip = " << (*it).first << ", port = " << (*it).second << std::endl;
        }
    }

    // 3. 获取一个host的ip和port
    int cnt = 0;
    rt = api.get_host(modid, cmdid, ip, port);
    if (rt == 0) {
        std::cout << "host is " << ip << " : " << port << std::endl;
        // TODO 上报调用结果
        // 下面就是配置report业务的参数信息
        api.report(modid, cmdid, ip, port, 0);
    }
    return 0;
}


