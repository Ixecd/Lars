#include <time.h>
#include <string.h>
#include "echoMessage.pb.h"
#include "tcp_client.hpp"
#include "qc.hpp"

using namespace qc;

struct Qps {

    Qps() {
        last_time = time(NULL);
        succ_cnt = 0;
    }

    long last_time; //最后一次发包时间ms为单位
    int succ_cnt; //成功收到服务器回显的次数

};

void client_busi(const char *data, int len, int msgid, net_connection *conn, void *user_data) {
    Qps *qps = (Qps*)user_data; // 用户参数

    qps_test::EchoMessage request, response;

    // 1.解析来自服务端的消息
    // 这里使用parseFromArray 还是 parseFromString -> 二进制为前者,文本字符串类型为后者.
    if (response.ParseFromArray(data, len) == false) {
        printf("server call back data error\n");
        return ;
    }

    // 2.判断数据内容是否回显一致
    if (response.content() == "Hello Lars!") {
        qps->succ_cnt++;
    }

    // 3.判断时间
    long current_time = time(nullptr);
    // 如果当前时间比上一次的时间要大于1s
    if (current_time - qps->last_time >= 1) {
        printf("---> qps = %d <---\n", qps->succ_cnt);
        qps->last_time = current_time;
        qps->succ_cnt = 0;
    }

    // 4.给服务端发送新的请求
    request.set_id(response.id() + 1);
    request.set_content(response.content());

    std::string requestString;
    request.SerializeToString(&requestString);

    conn->send_message(requestString.c_str(), requestString.size(), msgid);

}

// Hook
void connection_start(net_connection *conn, void *user_data) {
    qps_test::EchoMessage request;

    request.set_id(1);
    request.set_content("Hello Lars!");

    // 序列化
    std::string requestString;

    request.SerializeToString(&requestString);

    int msgid = 1;

    conn->send_message(requestString.c_str(), requestString.size(), msgid);
}

Qps qps;

void* thread_main(void *args) {
    // 创建好一个线程之后,就在它的线程main函数中创建一个client和server连接
    event_loop loop;

    tcp_client client(&loop, "127.0.0.1", 7777, "qps client");

    

    // 注册路由
    client.add_msg_router(1, client_busi, (void *)&qps);

    client.set_conn_start(connection_start);

    // 开启事件循环
    loop.event_process();

    return nullptr;
}


/// @brief 这里要进行服务器框架的性能测试,所以对应的是一个服务端和N个客户端->线程池实现
int main(int argc, char **argv) {

    if (argc == 1) {
        printf("Usage: ./client [threadNum]\n");
        return 1;
    }

    // 创建N个线程
    int threads = atoi(argv[1]);
    pthread_t *tids = new pthread_t[threads];

    // 线程对应的回调函数类型为void *func(void *args);
    for (int i = 0; i < threads; ++i) {
        pthread_create(&tids[i], nullptr, thread_main, nullptr);
        // 这里detach 是非阻塞的会直接往下执行,main就结束了,主线程退出时,其他线程也会退出
        // pthread_detach(tids[i]);
    }

    for (int i = 0; i < threads; ++i) 
        pthread_join(tids[i], nullptr);

    // while (1);

    return 0;
}