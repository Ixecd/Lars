#include <string>
#include <string.h>
#include "config_file.hpp"
#include "tcp_server.hpp"
#include "echoMessage.pb.h"

using namespace qc;

// 会先业务的回调函数
void call_backbusi(const char *data, int len, int msgid, net_connection *conn, void *usr_data) {
    qps_test::EchoMessage request, response;

    // 解包,解包成功之后data数据就放在request中.
    request.ParseFromArray(data, len);

    response.set_id(request.id());

    response.set_content(request.content());

    std::string responseString;
    response.SerializeToString(&responseString);

    conn->send_message(responseString.c_str(), responseString.size(), msgid);
}


int main() {
    event_loop loop;
    
    // 加载配置文件
    config_file::GetInstance()->setPath("./serv.conf");
    std::string ip = config_file::GetInstance()->GetString("reactor", "ip", "0.0.0.0");
    short port = config_file::GetInstance()->GetNumber("reactor", "port", 8888);

    printf("ip = %s, port = %d\n", ip.c_str(), port);

    tcp_server server(&loop, ip.c_str(), port);

    // 注册消息业务路由
    server.add_msg_router(1, call_backbusi);

    loop.event_process();

    return 0;
}