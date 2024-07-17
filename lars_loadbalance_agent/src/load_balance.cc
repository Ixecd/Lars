#include <lars_loadbalance_agent/load_balance.hpp>
#include <lars_loadbalance_agent/main_server.hpp>

#define PROBE_NUM 10
#define INIT_SUCC_CNT 128

// 与dns_client通信的thread_queue消息队列
extern qc::thread_queue<lars::GetRouteRequest> *dns_queue;
// 与report_client通信的消息队列
extern qc::thread_queue<lars::ReportStatusReq> *report_queue;
// load_balance配置文件
extern struct load_balance_config lb_config;
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

/// @brief 获取当前lb下所有的host信息
void load_balance::get_all_hosts(std::vector<host_info *> &vec) {
    for (host_map_it it = _host_map.begin(); it != _host_map.end(); ++it) {
        vec.emplace_back(it->second);
    }
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
    return lars::RET_SUCC;
}

/// @brief 向自己的idle_list中添加主机信息
/// @param rsp
void load_balance::update(lars::GetRouteResponse &rsp) {
    qc_assert(rsp.host_size() != 0);
    long current_time = time(nullptr);

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
        else
            _idle_list.remove(hi);
        delete hi;
    }
    // 更新完毕,重新设置状态
    /// @details load_balance
    /// 每次调用一次update就更新一次last_update_time,并标记为NEW表示
    ///          当前modid/cmdid,表示当前节点并不是PULLING状态,可以更新
    last_update_time = current_time;
    status = NEW;
}

/// @brief 上报当前host主机调用情况给远端repoter service
/// @version 2 添加过期窗口和过载超时
void load_balance::report(int ip, int port, int retcode) {
    // 定义当前时间
    long current_time = time(nullptr);

    uint64_t key = ((uint64_t)ip << 32) + port;

    if (_host_map.find(key) == _host_map.end()) {
        return;
    }

    // 更新

    // 1 计数统计
    host_info *hi = _host_map[key];
    if (retcode == lars::RET_SUCC) {
        // 更新虚拟成功,真实成功
        hi->vsucc++;
        hi->rsucc++;

        hi->contin_succ++;
        hi->contin_err = 0;
    } else {
        // 更新虚拟失败,真实失败
        hi->verr++;
        hi->rerr++;

        hi->contin_err++;
        hi->contin_succ = 0;
    }

    // 2.检查节点状态
    /// @details 关于idle节点只有返回值不为SUCC才去判断是否更新为overload状态
    if (hi->overload == false && retcode != lars::RET_SUCC) {
        bool overload = false;

        double err_rate = hi->verr * 1.0 / (hi->verr + hi->vsucc);

        if (err_rate > lb_config.err_rate) overload = true;

        if (overload == false &&
            hi->contin_err >= (uint32_t)lb_config.contin_err_limits)
            overload = true;

        if (overload == true) {
            // 输出一下过载信息
            struct in_addr saddr;
            saddr.s_addr = htonl(hi->ip);

            printf("[%d, %d] host %s : %d change overload, succ %u, err %u\n",
                   _modid, _cmdid, inet_ntoa(saddr), hi->port, hi->vsucc,
                   hi->verr);

            hi->set_overload();

            _idle_list.remove(hi);
            _overload_list.push_back(hi);
            return;
        }
    }  // 如果当前节点的状态是overload那只有调用成功才会有可能重新设置为idle
    else if (hi->overload == true && retcode == lars::RET_SUCC) {
        bool idle = false;

        double succ_rate = hi->vsucc * 1.0 / (hi->vsucc + hi->verr);
        if (succ_rate >= lb_config.succ_rate) idle = true;

        if (hi->contin_succ >= (uint32_t)lb_config.contin_succ_limits)
            idle = true;

        if (idle == true) {
            // 输出一下信息
            struct in_addr saddr;
            saddr.s_addr = htonl(hi->ip);

            printf("[%d, %d] host %s : %d change idle, succ %u, err %u\n",
                   _modid, _cmdid, inet_ntoa(saddr), hi->port, hi->vsucc,
                   hi->verr);

            hi->set_idle();

            // 更改列表
            _overload_list.remove(hi);
            _idle_list.push_back(hi);
            return;
        }
    }

    // TODO 窗口检查和超时机制
    if (hi->overload == false) {
        if (current_time - hi->idle_ts >= lb_config.idle_timeout) {
            // 时间窗口到达,需要对idle系欸但清理负载均衡数据
            if (hi->check_window() == true) {
                // 设置当前节点状态为overload
                struct in_addr saddr;
                saddr.s_addr = htonl(hi->ip);

                printf(
                    "[%d, %d] host %s : %d change overload, succ %u, err %u\n",
                    _modid, _cmdid, inet_ntoa(saddr), hi->port, hi->vsucc,
                    hi->verr);

                hi->set_overload();

                _idle_list.remove(hi);
                _overload_list.push_back(hi);
            } else {
                // 重置窗口,回复负载默认信息
                hi->set_idle();
            }
        }
    } else {
        // 节点为overload状态
        // 那么处于overload状态的节点状态时间是否已经超时
        if (current_time - hi->overload_ts >= lb_config.overload_timeout) {
            // 输出一下信息
            struct in_addr saddr;
            saddr.s_addr = htonl(hi->ip);

            printf("[%d, %d] host %s : %d reset idle, succ %u, err %u\n",
                   _modid, _cmdid, inet_ntoa(saddr), hi->port, hi->vsucc,
                   hi->verr);

            hi->set_idle();

            // 更改列表
            _overload_list.remove(hi);
            _idle_list.push_back(hi);
        }
    }
}

// 上报结果
void load_balance::commit() {
    if (this->empty()) return;
    // 1.封装请求消息
    lars::ReportStatusReq req;
    req.set_modid(_modid);
    req.set_cmdid(_cmdid);
    req.set_caller(lb_config.local_ip);
    req.set_ts(time(nullptr));

    // 2. 从idle_list取值
    for (host_list_it it = _idle_list.begin(); it != _idle_list.end(); it++) {
        host_info *hi = *it;
        lars::HostCallResult call_res;
        call_res.set_ip(hi->ip);
        call_res.set_port(hi->port);
        call_res.set_succ(hi->rsucc);
        call_res.set_err(hi->rerr);
        call_res.set_overload(false);

        req.add_results()->CopyFrom(call_res);
    }

    // 3. 从over_list取值
    for (host_list_it it = _overload_list.begin(); it != _overload_list.end();
         it++) {
        host_info *hi = *it;
        lars::HostCallResult call_res;
        call_res.set_ip(hi->ip);
        call_res.set_port(hi->port);
        call_res.set_succ(hi->rsucc);
        call_res.set_err(hi->rerr);
        call_res.set_overload(true);

        req.add_results()->CopyFrom(call_res);
    }

    // 4 发送给report_client 的消息队列
    report_queue->send(req);
}

}  // namespace qc