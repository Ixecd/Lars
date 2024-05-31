#include "route_lb.hpp"

#include "lars.pb.h"

namespace qc {

route_lb::route_lb(int id) : _id(id) {}

// agent 获取一个host主机,将返回的主机结果放到rsp中
int route_lb::get_host(int modid, int cmdid, lars::GetHostResponse &rsp) {
    int rt = lars::RET_SUCC;
    uint64_t key = ((uint64_t)modid << 32) + cmdid;

    _mutex.lock();
    // key 在 _route_lb_map中
    if (_route_lb_map.find(key) != _route_lb_map.end()) {
        // 取出load_balance
        load_balance *lb = _route_lb_map[key];
        // 如果lb里面host为空,说明正在Pull(),没从dns_service返回来
        if (lb->empty() == true) {
            qc_assert(lb->status == load_balance::PULLING);
            rsp.set_retcode(lars::RET_NOEXIST);
        } else {
            rt = lb->choice_one_host(rsp);
            rsp.set_retcode(rt);
            //TODO
        }
    } else { // 当前key不存在_route_lb_map中
        load_balance *lb = new load_balance(modid, cmdid);
        qc_assert(lb != nullptr);

        _route_lb_map[key] = lb;
        // 从dns service中拉取具体的host信息
        lb->pull();
        rsp.set_retcode(lars::RET_NOEXIST);
        rt = lars::RET_NOEXIST;
    }
    return rt;
}

int route_lb::update_host(int modid, int cmdid, lars::GetRouteResponse &rsp) {
    // 更新自己的_route_lb_map
    uint64_t key = ((uint64_t)modid << 32) + cmdid;
    _mutex.lock();

    // 在route_map中找到对应的key
    if (_route_lb_map.find(key) != _route_lb_map.end()) {
        load_balance *lb = _route_lb_map[key];

        if (rsp.host_size() == 0) {
            // 可以删除这个load信息
            delete lb;
            _route_lb_map.erase(key);
        } else {
            // 更新信息,这里要确保rsp中host的信息不为空
            lb->update(rsp);
        }
    }
    return 0;
}

}  // namespace qc