#include <iostream>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>
#include <algorithm>
#include <iomanip>
#include <fstream>

using namespace std;
using namespace std::chrono;

// 节点状态结构体
struct Node {
    string ip;
    int port;
    int capacity;           // 最大负载能力
    int current_load{0}; // 当前负载
    double response_time{0.0}; // 平均响应时间(ms)
    double error_rate{0.0};    // 错误率(%)

    Node(string ip, int port, int cap) 
        : ip(ip), port(port), capacity(cap) {}

    bool process_request() {
        // 模拟处理时间 = 基础时间 + 负载惩罚
        double base_time = 5 + rand() % 10; // 5-15ms
        double load_penalty = static_cast<double>(current_load) / capacity * 50;
        double processing_time = base_time + load_penalty;
        
        // 模拟实际处理延迟
        this_thread::sleep_for(microseconds(static_cast<int>(processing_time * 1000)));
        
        // 更新节点状态
        current_load++;
        
        // 模拟错误（过载时错误率升高）
        bool is_error = (rand() % 100) < (1 + min(20, current_load * 50 / capacity));
        
        // 更新指标
        response_time = response_time * 0.9 + processing_time * 0.1;
        if (is_error) {
            error_rate = error_rate * 0.9 + 10;
        } else {
            error_rate *= 0.9;
        }
        
        current_load--;
        return !is_error;
    }
};

// 负载均衡器基类
class LoadBalancer {
public:
    vector<Node> nodes;
    
    LoadBalancer(const vector<Node>& nodes) : nodes(nodes) {}
    virtual ~LoadBalancer() = default;
    
    virtual Node& select_node() = 0;
    virtual void report_result(Node& node, bool success, double response_time) {}
};

// Round-Robin负载均衡器
class RoundRobinLB : public LoadBalancer {
    atomic<int> index{0};
    mutex mtx;
    
public:
    using LoadBalancer::LoadBalancer;
    
    Node& select_node() override {
        lock_guard<mutex> lock(mtx);
        int i = index++ % nodes.size();
        return nodes[i];
    }
};

class WeightedRoundRobinLB : public LoadBalancer {
    vector<int> weights;
    atomic<int> current_index{0};
    atomic<int> current_weight{0};
    int max_weight;
    int gcd_weight;
    mutex mtx;

    // 计算最大公约数
    int gcd(int a, int b) {
        if (b == 0) return a;
        return gcd(b, a % b);
    }

    // 计算最大权重
    void calculate_weights() {
        max_weight = 0;
        for (const auto& node : nodes) {
            weights.push_back(node.capacity);
            if (node.capacity > max_weight) {
                max_weight = node.capacity;
            }
        }

        // 计算所有权重的最大公约数
        gcd_weight = weights[0];
        for (size_t i = 1; i < weights.size(); i++) {
            gcd_weight = gcd(gcd_weight, weights[i]);
        }
    }

public:
    WeightedRoundRobinLB(const vector<Node>& nodes) : LoadBalancer(nodes) {
        calculate_weights();
    }

    Node& select_node() override {
        lock_guard<mutex> lock(mtx);
        while (true) {
            current_index.store((current_index + 1) % nodes.size());
            if (current_index == 0) {
                current_weight = current_weight - gcd_weight;
                if (current_weight <= 0) {
                    current_weight = max_weight;
                    if (current_weight == 0) {
                        return nodes[0];
                    }
                }
            }

            if (weights[current_index] >= current_weight) {
                return nodes[current_index];
            }
        }
    }
};

// 自定义负载均衡器
class CustomLB : public LoadBalancer {
    deque<Node*> idle_queue;
    deque<Node*> overload_queue;
    const double threshold = 150.0; // 响应时间阈值
    mutex mtx;
    int probe;
    
public:
    CustomLB(const vector<Node>& nodes) : LoadBalancer(nodes) {
        for (auto& node : this->nodes) {
            idle_queue.push_back(&node);
        }
    }
    
    Node& select_node() override {
        lock_guard<mutex> lock(mtx);
        
        if (probe >= 10) {
            probe = 0;
            if (!overload_queue.empty()) return *overload_queue.front();
        }

        // 优先选择空闲队列
        if (!idle_queue.empty()) {
            probe++;
            return *idle_queue.front();
        }
        
        // 没有空闲节点时选择过载队列
        if (!overload_queue.empty()) {
            probe++;
            return *overload_queue.front();
        }
        
        probe++;
        return nodes[rand() % nodes.size()];
    }
    
    void report_result(Node& node, bool success, double response_time) override {
        lock_guard<mutex> lock(mtx);
        
        // 动态队列调整
        auto find_and_move = [&](deque<Node*>& from, deque<Node*>& to) {
            auto it = find(from.begin(), from.end(), &node);
            if (it != from.end()) {
                to.push_back(*it);
                from.erase(it);
            }
        };
        
        if (response_time > threshold || !success) {
            find_and_move(idle_queue, overload_queue);
        } else {
            find_and_move(overload_queue, idle_queue);
        }
    }
};

// 性能统计结构
struct Stats {
    int total_requests{0};
    int successful_requests{0};
    double total_response_time{0.0};
    vector<int> node_counts;
    
    Stats(int node_count) : node_counts(node_count, 0) {}
};

// 测试工作线程
void worker_thread(LoadBalancer* lb, Stats& stats, atomic<bool>& running) {
    random_device rd;
    mt19937 gen(rd());
    
    while (running) {
        auto start = high_resolution_clock::now();
        
        Node& node = lb->select_node();
        bool success = node.process_request();
        
        auto end = high_resolution_clock::now();
        double duration = duration_cast<microseconds>(end - start).count() / 1000.0;
        
        // 上报结果
        lb->report_result(node, success, duration);
        
        // 更新统计
        stats.total_requests++;
        if (success) {
            stats.successful_requests++;
            stats.total_response_time += duration;
        }
        
        // 记录节点选择
        for (size_t i = 0; i < lb->nodes.size(); ++i) {
            if (&node == &lb->nodes[i]) {
                stats.node_counts[i]++;
                break;
            }
        }
    }
}

// 运行负载测试
Stats run_load_test(LoadBalancer* lb, int thread_count, int duration_sec) {
    atomic<bool> running{true};
    Stats stats(lb->nodes.size());
    
    // 启动工作线程
    vector<thread> threads;
    for (int i = 0; i < thread_count; ++i) {
        threads.emplace_back(worker_thread, lb, ref(stats), ref(running));
    }
    
    // 运行指定时间
    this_thread::sleep_for(seconds(duration_sec));
    running = false;
    
    // 等待线程结束
    for (auto& t : threads) {
        t.join();
    }
    
    return stats;
}

// 打印测试结果
void print_results(const Stats& stats, const string& algo_name, int duration) {
    cout << "\n===== " << algo_name << " 测试结果 =====" << endl;
    cout << "测试时长: " << duration << " 秒" << endl;
    cout << "总请求数: " << stats.total_requests << endl;
    cout << "成功请求数: " << stats.successful_requests << endl;
    cout << "成功率: " 
         << fixed << setprecision(2) 
         << (static_cast<double>(stats.successful_requests) / stats.total_requests * 100)
         << "%" << endl;
    cout << "平均响应时间: " 
         << (stats.total_response_time / stats.successful_requests) 
         << " ms" << endl;
}

int main() {
    // 创建测试节点 (5个节点)
    vector<Node> nodes = {
        Node("192.168.1.101", 8080, 100),
        Node("192.168.1.102", 8080, 150),
        Node("192.168.1.103", 8080, 80),
        Node("192.168.1.104", 8080, 70),
        Node("192.168.1.105", 8080, 100)
    };

    // 测试参数
    const int THREAD_COUNT = 50;    // 并发线程数
    const int TEST_DURATION = 10;   // 测试时长(秒)

    // 测试Round-Robin算法
    cout << "测试Round-Robin算法..." << endl;
    RoundRobinLB rr_lb(nodes);
    Stats rr_stats = run_load_test(&rr_lb, THREAD_COUNT, TEST_DURATION);
    print_results(rr_stats, "RoundRobin", TEST_DURATION);

    // 测试Weighted Round-Robin算法
    cout << "\n测试Weighted Round-Robin算法..." << endl;
    WeightedRoundRobinLB wrr_lb(nodes);
    Stats wrr_stats = run_load_test(&wrr_lb, THREAD_COUNT, TEST_DURATION);
    print_results(wrr_stats, "WeightedRoundRobin", TEST_DURATION);

    // 测试自定义算法
    cout << "\n测试自定义负载均衡算法..." << endl;
    CustomLB custom_lb(nodes);
    Stats custom_stats = run_load_test(&custom_lb, THREAD_COUNT, TEST_DURATION);
    print_results(custom_stats, "CustomLB", TEST_DURATION);

    return 0;
}