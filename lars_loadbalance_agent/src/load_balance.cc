#include "load_balance.hpp"
#include "main_server.hpp"
#define PROBE_NUM 10
#define INIT_SUCC_CNT 128

// 与dns_client通信的thread_queue消息队列
extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;

namespace qc {

static void get_host_from_list(lars::GetHostResponse &rsp, host_list &l) {
    host_info *host = l.front();

    // HostInfo自定义类型,proto3并没提供set方法,通过mutable_返回HostInfo指针
    lars::HostInfo *hip = rsp.mutable_host();
    hip->set_ip(host->ip);
    hip->set_port(host->port);

    // 添加到队列尾部
    l.pop_front();
    l.push_back(host);
}

// 从两个队列中获取一个host给上层
int load_balance::choice_one_host(lars::GetHostResponse &rsp) {
    // 1. 判断空闲list是否为空
    if (_idle_list.empty()) {
        // 1.1 判断过载list 是否为空
        if (!_overload_list.empty()) {
            // 1.2 判断_access_cnt是否超过probe_num
            if (_access_cnt >= PROBE_NUM) {
                _access_cnt = 0;
                // 1.3 从过载队列中取一个返回
                get_host_from_list(rsp, _overload_list);
            } else {
                // 1.4 返回错误
                ++_access_cnt;
                std::cout << "CUR SYSTEM OVERLOAD\n";
                return lars::RET_OVERLOAD;
            }
        } else {
            std::cout << "CUR SYSTEM ERR\n";
            return lars::RET_SYSTEM_ERR;
        }
    } else {
        // 2. 空闲队列不为空
        // 判断过载列表是否为空
        if (_overload_list.empty()) {
            // 当前所有节点都正常
            get_host_from_list(rsp, _idle_list);
        } else {
            // 有部分过载的节点
            if (_access_cnt >= PROBE_NUM) {
                _access_cnt = 0;
                get_host_from_list(rsp, _overload_list);
            } else {
                ++_access_cnt;
                get_host_from_list(rsp, _idle_list);
            }
            // 目前不给机会
            get_host_from_list(rsp, _idle_list);
        }

    }
    return lars::RET_SUCC;
}

// 如果list中没有host信息,就需要从远程DNS Service发送GetRouteHost请求申请
int load_balance::pull() {
    lars::GetRouteRequest route_req;
    route_req.set_modid(_modid);
    route_req.set_cmdid(_cmdid);

    // 通过DNS client的thread queue发送请求
    dns_queue->send(route_req);

    status = PULLING;
}

/// @brief 向自己的idle_list中添加主机信息
/// @param rsp 
void load_balance::update(lars::GetRouteResponse &rsp) {
    qc_assert(rsp.host_size() != 0);

    std::set<uint64_t> remote_hosts;
    std::set<uint64_t> need_delete;

    for (int i = 0; i < rsp.host_size(); ++i) {
        // get cur host
        const lars::HostInfo &HI = rsp.host(i);
        
        uint64_t key = ((uint64_t)HI.ip() << 32) + HI.port(); 

        remote_hosts.insert(key);

        if (_host_map.find(key) == _host_map.end()) {
            // 新增
            host_info *hi = new host_info(HI.ip(), HI.port(), INIT_SUCC_CNT);
            qc_assert(hi != nullptr);
            _host_map[key] = hi;

            _idle_list.push_back(hi);
        }
    }

    // 得到的所有结点都要删除
    for (auto it = _host_map.begin(); it != _host_map.end(); ++it) {
        if (remote_hosts.find(it->first) == remote_hosts.end()) 
            need_delete.insert(it->first);
    }

    // 开始真正删除
    for (auto it = need_delete.begin(); it != need_delete.end(); ++it) {
        uint64_t key = *it;
        host_info *hi = _host_map[key];
        if (hi->overload) 
            _overload_list.remove(hi);
        else _idle_list.remove(hi);
        delete hi;
    }
    

}

}