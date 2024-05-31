#include "lars_reactor.hpp"
#include "main_server.hpp"
#include "route_lb.hpp"

// 与report_client通信的thread_queue消息队列
extern qc::thread_queue<lars::ReportStatusReq> *report_queue;
// 与dns_client通信的thread_queue消息队列
extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;
// route_lb -> 一个管理多个load_balance
extern qc::route_lb *r_lb[3];

namespace qc {

static void get_host_cb(const char *data, uint32_t msglen, int msgid, net_connection *conn, void *user_data) {
    std::cout << "get_host_cb() started ..." << std::endl;

    // 1.解析从客户端发来的包
    lars::GetHostRequest req;
    
    req.ParseFromArray(data, msglen);
    // get modid, cmdid
    int modid = req.modid(), cmdid = req.cmdid();

    // set cb
    lars::GetHostResponse rsp;
    // 这里的seq一致才表示对方收到的是想要的包
    rsp.set_modid(modid), rsp.set_cmdid(cmdid), rsp.set_seq(req.seq());

    // get route_loadbalance ->  route_map
    route_lb *ptr_route_lb = (route_lb *)user_data;

    ptr_route_lb->get_host(modid, cmdid, rsp);

    // 打包回执给api消息
    std::string responseString;
    rsp.SerializeToString(&responseString);
    conn->send_message(responseString.c_str(), responseString.size(), lars::ID_GetHostResponse);

    std::cout << "send to api message.size() = " << responseString.size() << std::endl;
}

// 来接收业务
void *agent_server_main(void *args) {
    int *index = (int *)args;
    // std::cout << "index = " << *index << std::endl;
    unsigned short port = *index + 8888;
    event_loop loop;

    udp_server server(&loop, "0.0.0.0", port);

    // 注册路由
    server.add_msg_router(lars::ID_GetHostRequest, get_host_cb, r_lb[port-8888]);


    std::cout << "agent UDP server: port " << port << " is started...\n";

    loop.event_process();

    return nullptr;
}

void start_UDP_servers(void) {
    // 创建三个线程
    for (int i = 0; i < 3; ++i) {
        pthread_t tid;
        int rt = pthread_create(&tid, nullptr, agent_server_main, (int *)&i);
        qc_assert(rt != -1);
        pthread_detach(tid);
    }
}
}



