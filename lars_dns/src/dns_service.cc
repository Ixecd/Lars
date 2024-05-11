#include "lars_reactor.hpp"
#include "mysql.h"

using namespace qc;

int main(int argc, char **argv) {

    event_loop loop;

    // 加载配置信息
    config_file::setPath("../conf/lars.conf");
    std::string ip = config_file::GetInstance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::GetInstance()->GetNumber("reactor", "port", 9876);

    tcp_server *server = new tcp_server(&loop, ip.c_str(), port);


    // 测试mysql接口
    MYSQL dbconn;
    mysql_init(&dbconn);

    // 注册路由信息
    printf("lars dns service ...\n");
    loop.event_process();

    return 0;
}
