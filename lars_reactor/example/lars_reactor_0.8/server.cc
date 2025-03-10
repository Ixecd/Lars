#include <string.h>

#include <string>

#include "config_file.hpp"
#include "udp_server.hpp"

using namespace qc;

//回显业务的回调函数
void callback_busi(const char *data, uint32_t len, int msgid,
                   net_connection *conn, void *user_data) {
    printf("callback_busi ...\n");
    //直接回显
    conn->send_message(data, len, msgid);
}

int main() {
    event_loop loop;

    //加载配置文件
    config_file::setPath("./serv.conf");
    std::string ip =
        config_file_instance::GetInstance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file_instance::GetInstance()->GetNumber("reactor", "port", 8888);

    printf("ip = %s, port = %d\n", ip.c_str(), port);

    udp_server server(&loop, ip.c_str(), port);

    //注册消息业务路由
    server.add_msg_router(1, callback_busi);

    loop.event_process();

    return 0;
}