#include "lars_reactor.hpp"
#include "main_server.hpp"
#include <pthread.h>
namespace qc {

void *dns_client(void *args) {
    int *index = (int *)args;
    std::cout << "dns_client thread running...\n";
    return nullptr;
}   

void start_dns_client(void) {
    // 创建一个个线程
    pthread_t tid;
    int rt = pthread_create(&tid, nullptr, dns_client, nullptr);
    qc_assert(rt != -1);

    pthread_detach(tid);
}


}