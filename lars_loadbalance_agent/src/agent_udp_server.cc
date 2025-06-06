#include <lars_loadbalance_agent/main_server.hpp>
#include <lars_loadbalance_agent/route_lb.hpp>
#include <lars_reactor/lars_reactor.hpp>



/// @brief 多进程下,每个进程都要有两个消息队列

// 与report_client通信的thread_queue消息队列
// extern qc::thread_queue<lars::ReportStatusReq> *report_queue;
qc::thread_queue<lars::ReportStatusRequest> *report_queue;
// 与dns_client通信的thread_queue消息队列
// extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;
qc::thread_queue<lars::GetRouteRequest> *dns_queue;

// route_lb -> 一个管理多个load_balance
extern qc::route_lb *r_lb[3];

namespace qc {

static void get_host_cb(const char *data, uint32_t msglen, int msgid,
                        net_connection *conn, void *user_data) {
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
    conn->send_message(responseString.c_str(), responseString.size(),
                       lars::ID_GetHostResponse);

    std::cout << "send to api message.size() = " << responseString.size()
              << std::endl;
}

/// @brief 由agent服务端才处理api层的get_route请求
static void get_route_cb(const char *data, uint32_t msglen, int msgid,
                         net_connection *conn, void *user_data) {
    std::cout << "agent server(UDP) get_route_cb running..." << std::endl;
    // 1. 解析来自API的数据包
    lars::GetRouteRequest req;
    req.ParseFromArray(data, msglen);
    int modid = req.modid();
    int cmdid = req.cmdid();

    std::cout << "modid = " << modid  << ", cmdid = " << cmdid << std::endl;
    if (user_data == nullptr) {
        std::cout << "user_data is nullptr, return..." << std::endl;
    }

    // 设置回执信息
    lars::GetRouteResponse rsp;
    rsp.set_modid(modid);
    rsp.set_cmdid(cmdid);

    route_lb *ptr_route_lb = (route_lb *)user_data;

    // 调用route_lb 的获取host方法,得到rsp返回结果
    qc_assert(ptr_route_lb!= nullptr);

    ptr_route_lb->get_route(modid, cmdid, rsp);

    // 得到结果之后返回给API层
    std::string responseString;
    rsp.SerializeToString(&responseString);
    conn->send_message(responseString.c_str(), responseString.size(),
                       lars::ID_API_GetRouteRequest);
}

static void report_cb(const char *data, uint32_t msglen, int msgid,
                      net_connection *conn, void *user_data) {
    // 接收请求
    lars::ReportRequest req;

    req.ParseFromArray(data, msglen);

    route_lb *ptr_route_lb = (route_lb *)user_data;
    ptr_route_lb->report_host(req);
}

// 来接收业务
void *agent_server_main(void *args) {
    long index = (long)args;
    // int index = (int)args;

    short port = index + 8888;

    event_loop loop;

    udp_server server(&loop, "0.0.0.0", port);

    // 注册路由,处理HOST REQUEST请求
    server.add_msg_router(lars::ID_GetHostRequest, get_host_cb,
                          r_lb[port - 8888]);

    // 注册路由,处理ReportRequest请求
    server.add_msg_router(lars::ID_ReportRequest, report_cb, r_lb[port - 8888]);

    // 注册路由,支持API层的getRoute
    /// @details 段错误出现在这里
    server.add_msg_router(lars::ID_API_GetRouteRequest, get_route_cb,
                          r_lb[port - 8888]);

    std::cout << "[Agent UDP Server]" << " port : " << port << " is started."
              << std::endl;

    // 3. 启动report client
    report_queue = new thread_queue<lars::ReportStatusRequest>();
    qc_assert(report_queue != nullptr);
    start_report_client(report_queue);

    // 4. 启动dns
    dns_queue = new thread_queue<lars::GetRouteRequest>();
    qc_assert(dns_queue != nullptr);
    start_dns_client(dns_queue);


    loop.event_process();

    return nullptr;
}

// TODO:
// 为了考虑系统的安全和可靠,这里将单进程多线程的方式改为多进程模式,每个进程负责一个agent_server_main线程

// 光荣退休!
#if 0
void start_UDP_servers() {
    // 创建三个线程
    long index = 0;
    for (int i = 0; i < 3; ++i) {
        pthread_t tid;
        index = i;
        int rt = pthread_create(&tid, nullptr, agent_server_main, (void
        *)index); qc_assert(rt != -1); pthread_detach(tid);
    }
}

#else 
void start_UDP_servers() {
    // 创建三个进程
    long index = 0;
    for (int i = 0; i < 3; ++i) {
        // 父进程返回子进程的pid
        // 子进程返回0
        // 这里一共有四个进程,一个main进程,三个agent_server_main进程
        pid_t pid = fork();
        if (pid == 0) {
            index = i;
            agent_server_main((void *)index);
            // 下不来的,必须在angent_server_main中初始化环境
            exit(0);
        }
    }
}
#endif

}  // namespace qc
