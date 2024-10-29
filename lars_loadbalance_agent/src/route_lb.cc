#include <lars_loadbalance_agent/route_lb.hpp>
#include <proto/lars.pb.h>

extern struct load_balance_config lb_config;

namespace qc {

// id 从 1开始
route_lb::route_lb(int id) : _id(id) {}

// agent 获取一个host主机,将返回的主机结果放到rsp中
int route_lb::get_host(int modid, int cmdid, lars::GetHostResponse &rsp) {
    
    int rt = lars::RET_SUCC;
    uint64_t key = ((uint64_t)modid << 32) + cmdid;

    MutexType::Lock lock(_mutex);
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
            // ------ 超时重新拉取路由信息 ------
            if (lb->status == load_balance::NEW && time(nullptr) - lb->last_update_time > lb_config.update_timeout) {
                lb->pull();
            }
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

    MutexType::Lock lock(_mutex);
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

// 由当前route上报host的调用结果
/// @brief 所谓上报其实就是向对应的哈希表更新数据
void route_lb::report_host(lars::ReportRequest req) {
    int modid = req.modid();
    int cmdid = req.cmdid();
    int retcode = req.retcode();
    int ip = req.host().ip();
    int port = req.host().port();

    uint64_t key = ((uint64_t)modid << 32) + cmdid;
    
    MutexType::Lock lock(_mutex);
    if (_route_lb_map.find(key) != _route_lb_map.end()) {
        load_balance *lb = _route_lb_map[key];
        
        lb->report(ip, port, retcode);
        // 上报信息给远程reporter服务器
        lb->commit();
    } 
}

void route_lb::reset_lb_status() {
    // 这里用一下范围锁
    MutexType::Lock lock(_mutex);
    for (auto it = _route_lb_map.begin(); it != _route_lb_map.end(); ++it) {
        load_balance *lb = it->second;
        if (lb->status == load_balance::PULLING) 
            lb->status = load_balance::NEW;
    }
}

// API层GetRoute 具体实现
// agent获取某个modid/cmdid的全部主机,放到rsp传出参数中
int route_lb::get_route(int modid, int cmdid, lars::GetRouteResponse &rsp) {
    std::cout << "route_lb::get_route() start..." << std::endl;
    int rt = lars::RET_SUCC;
    uint64_t key = ((uint64_t)modid << 32) + cmdid;
    // 范围锁
    // 锁是在多线程上加的,架构改为多进程之后,这里是否还需要加锁?
    // MutexType::Lock lock(_mutex);
    // std::cout << "get lock" << std::endl;
    if (_route_lb_map.find(key) == _route_lb_map.end()) {
        // 当前route_lb_map中没有对应key,那就创建一个
        load_balance *lb = new load_balance(modid, cmdid);
        qc_assert(lb != nullptr);

        // 将新建的load_balance添加到map中
        _route_lb_map[key] = lb;
        // _route_lb_map.insert({key, lb});
        
        // 新建好之后从dns service服务拉取具体的modid/cmdid信息
        lb->pull();
        rt = lars::RET_SUCC;
    } else {
        // 本来这个lb就在map中,直接取出来用
        load_balance *lb = _route_lb_map[key];
        
        // 那就直接从lb中获取所有数据返回
        std::vector<host_info*> vec;
        lb->get_all_hosts(vec);

        for (int i = 0; i < vec.size(); ++i) {
            lars::HostInfo host;
            host.set_ip(vec[i]->ip);
            host.set_port(vec[i]->port);
            // 添加到rsp中
            rsp.add_host()->CopyFrom(host);
        }

        // 超时重拉路由
        /// 如果路由没有处于PULLING状态,并且当前时间超过了有效期,那就重新拉取
        if (lb->status == load_balance::NEW && time(nullptr) - lb->last_update_time >= lb_config.update_timeout) {
            lb->pull();
        }
    }
    return rt;
}

}  // namespace qc