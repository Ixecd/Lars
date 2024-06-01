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

    int rt = api.get_host(modid, cmdid, ip, port);
    if (rt == 0) {
        std::cout << "host is " << ip << " : " << port << std::endl;
        // TODO 上报调用结果
        // 下面就是配置report业务的参数信息
        api.report(modid, cmdid, ip, port, rt);
    }
    return 0;
}


