#pragma once

namespace qc {

void thread_report(event_loop *loop, int fd, void *args);

void *store_main(void *args);


}