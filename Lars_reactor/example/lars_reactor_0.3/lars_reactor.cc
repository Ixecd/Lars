#include "tcp_server.hpp"

using namespace qc;

int main() {

    event_loop loop;
    tcp_server server(&loop, "127.0.0.1", 7777);
    loop.event_process();

    return 0;
}