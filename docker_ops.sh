#!/bin/bash
# Lars 服务 Docker 运维脚本

set -e

SERVICES=("lars_mysql" "lars_dns" "lars_reporter" "lars_loadbalance_agent")

show_help() {
    cat << EOF
Lars 服务 Docker 运维脚本

用法: ./docker_ops.sh <command> [service_name]

命令:
  status       - 查看所有服务状态
  logs         - 查看服务日志（实时）
  logs-tail    - 查看服务日志（最后N行，默认50行）
  shell        - 进入容器终端
  restart      - 重启服务
  stop         - 停止服务
  start        - 启动服务
  exec         - 在容器中执行命令
  ps           - 查看容器内进程
  netstat      - 查看端口监听状态
  mysql        - 进入 MySQL 客户端
  test         - 测试服务连接

示例:
  ./docker_ops.sh status
  ./docker_ops.sh logs lars_dns
  ./docker_ops.sh shell lars_dns
  ./docker_ops.sh exec lars_dns "ps aux | grep lars"
  ./docker_ops.sh mysql

EOF
}

check_service() {
    local service=$1
    if ! docker ps -a --format '{{.Names}}' | grep -q "^${service}$"; then
        echo "错误: 容器 ${service} 不存在"
        exit 1
    fi
}

status() {
    echo "=== 所有服务状态 ==="
    echo ""
    docker ps --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}" | grep -E "NAME|lars"
    echo ""
    echo "=== 服务进程状态 ==="
    for service in "${SERVICES[@]}"; do
        if docker ps --format '{{.Names}}' | grep -q "^${service}$"; then
            echo ""
            echo "[${service}]"
            case $service in
                lars_mysql)
                    docker exec $service ps aux | grep -E "mysqld|mysql" | grep -v grep | head -2 || echo "  未运行"
                    ;;
                lars_dns)
                    docker exec $service ps aux | grep "[l]ars_dns" || echo "  未运行"
                    ;;
                lars_reporter)
                    docker exec $service ps aux | grep "[l]ars_reporter" || echo "  未运行"
                    ;;
                lars_loadbalance_agent)
                    docker exec $service ps aux | grep "[l]ars_loadbalance_agent" || echo "  未运行"
                    ;;
            esac
        fi
    done
}

logs() {
    local service=${1:-""}
    if [ -z "$service" ]; then
        echo "用法: ./docker_ops.sh logs <service_name>"
        echo "可用服务: ${SERVICES[*]}"
        exit 1
    fi
    check_service $service
    echo "=== ${service} 日志（实时，按 Ctrl+C 退出）==="
    docker logs -f $service
}

logs_tail() {
    local service=${1:-""}
    local lines=${2:-50}
    if [ -z "$service" ]; then
        echo "用法: ./docker_ops.sh logs-tail <service_name> [lines]"
        echo "可用服务: ${SERVICES[*]}"
        exit 1
    fi
    check_service $service
    echo "=== ${service} 日志（最后 ${lines} 行）==="
    docker logs --tail $lines $service
}

shell() {
    local service=${1:-""}
    if [ -z "$service" ]; then
        echo "用法: ./docker_ops.sh shell <service_name>"
        echo "可用服务: ${SERVICES[*]}"
        exit 1
    fi
    check_service $service
    echo "进入 ${service} 容器终端..."
    echo "提示: 退出输入 exit"
    docker exec -it $service bash
}

restart() {
    local service=${1:-""}
    if [ -z "$service" ]; then
        echo "重启所有服务..."
        for service in "${SERVICES[@]}"; do
            if docker ps -a --format '{{.Names}}' | grep -q "^${service}$"; then
                echo "重启 ${service}..."
                docker restart $service
                # 等待容器启动
                sleep 2
                # 启动服务进程
                start_service_process $service
            fi
        done
    else
        check_service $service
        echo "重启 ${service}..."
        docker restart $service
        # 等待容器启动
        sleep 2
        # 启动服务进程
        start_service_process $service
    fi
}

# 启动服务进程（容器启动后需要手动启动服务进程）
start_service_process() {
    local service=$1
    case $service in
        lars_dns)
            echo "  启动 lars_dns 服务进程..."
            docker exec -d $service bash -c "cd /lars && /lars/lars_dns/bin/lars_dns /lars/lars_dns/conf/lars.conf" 2>/dev/null || true
            sleep 1
            ;;
        lars_reporter)
            echo "  启动 lars_reporter 服务进程..."
            docker exec -d $service bash -c "cd /lars && /lars/lars_reporter/bin/lars_reporter /lars/lars_reporter/config/lars_reporter.conf" 2>/dev/null || true
            sleep 1
            ;;
        lars_loadbalance_agent)
            echo "  启动 lars_loadbalance_agent 服务进程..."
            docker exec -d $service bash -c "cd /lars && /lars/lars_loadbalance_agent/bin/lars_loadbalance_agent" 2>/dev/null || true
            sleep 1
            ;;
        lars_mysql)
            # MySQL 服务由容器自动启动，无需手动启动
            ;;
        *)
            echo "  未知服务: $service"
            ;;
    esac
}

stop() {
    local service=${1:-""}
    if [ -z "$service" ]; then
        echo "停止所有服务..."
        docker stop "${SERVICES[@]}"
    else
        check_service $service
        echo "停止 ${service}..."
        docker stop $service
    fi
}

start() {
    local service=${1:-""}
    if [ -z "$service" ]; then
        echo "启动所有服务..."
        for service in "${SERVICES[@]}"; do
            if docker ps -a --format '{{.Names}}' | grep -q "^${service}$"; then
                echo "启动 ${service}..."
                docker start $service
                # 等待容器启动
                sleep 2
                # 启动服务进程
                start_service_process $service
            fi
        done
    else
        check_service $service
        echo "启动 ${service}..."
        docker start $service
        # 等待容器启动
        sleep 2
        # 启动服务进程
        start_service_process $service
    fi
}

exec_cmd() {
    local service=${1:-""}
    local cmd=${2:-""}
    if [ -z "$service" ] || [ -z "$cmd" ]; then
        echo "用法: ./docker_ops.sh exec <service_name> \"<command>\""
        echo "示例: ./docker_ops.sh exec lars_dns \"ps aux | grep lars\""
        exit 1
    fi
    check_service $service
    docker exec $service bash -c "$cmd"
}

ps_cmd() {
    local service=${1:-""}
    if [ -z "$service" ]; then
        echo "用法: ./docker_ops.sh ps <service_name>"
        echo "可用服务: ${SERVICES[*]}"
        exit 1
    fi
    check_service $service
    echo "=== ${service} 进程列表 ==="
    docker exec $service ps aux
}

netstat() {
    local service=${1:-""}
    if [ -z "$service" ]; then
        echo "用法: ./docker_ops.sh netstat <service_name>"
        echo "可用服务: ${SERVICES[*]}"
        exit 1
    fi
    check_service $service
    echo "=== ${service} 端口监听状态 ==="
    echo ""
    
    # 尝试使用 ss 或 netstat
    local has_ss=$(docker exec $service which ss 2>/dev/null)
    local has_netstat=$(docker exec $service which netstat 2>/dev/null)
    
    if [ -n "$has_ss" ]; then
        echo "TCP 端口:"
        docker exec $service ss -tlnp 2>/dev/null || echo "  无 TCP 端口监听"
        echo ""
        echo "UDP 端口:"
        docker exec $service ss -ulnp 2>/dev/null || echo "  无 UDP 端口监听"
    elif [ -n "$has_netstat" ]; then
        echo "TCP 端口:"
        docker exec $service netstat -tlnp 2>/dev/null || echo "  无 TCP 端口监听"
        echo ""
        echo "UDP 端口:"
        docker exec $service netstat -ulnp 2>/dev/null || echo "  无 UDP 端口监听"
    else
        echo "提示: 容器中未安装 ss 或 netstat 工具"
        echo ""
        echo "服务进程:"
        docker exec $service ps aux | grep -E "lars|mysql" | grep -v grep | head -5
        echo ""
        echo "如需查看端口详情，可以："
        echo "  1. 安装工具: docker exec $service apt-get update && apt-get install -y net-tools iproute2"
        echo "  2. 或使用: ./docker_ops.sh ps $service 查看进程"
    fi
}

mysql() {
    check_service lars_mysql
    echo "进入 MySQL 客户端..."
    echo "提示: 退出输入 exit 或按 Ctrl+D"
    docker exec -it lars_mysql mysql -uroot -p20030329
}

test_connection() {
    echo "=== 测试服务连接 ==="
    echo ""
    echo "测试 lars_dns (7775 - TCP):"
    # 检查进程是否运行（使用更灵活的匹配）
    local dns_process=$(docker exec lars_dns ps aux 2>/dev/null | grep -E "[l]ars_dns|bin/lars_dns" | wc -l | tr -d ' ')
    if [ "$dns_process" -gt 0 ]; then
        # 从宿主机测试端口映射（使用 nc，兼容 macOS）
        if nc -z -w 2 localhost 7775 2>/dev/null; then
            echo "  ✓ 端口 7775 可连接（从宿主机）"
        else
            echo "  ⚠ 服务运行中（$dns_process 个进程），但端口 7775 无法从宿主机连接（可能服务正在启动或绑定问题）"
        fi
    else
        echo "  ✗ 服务未运行"
    fi
    
    echo "测试 lars_reporter (7777 - TCP):"
    # 检查进程是否运行（使用更灵活的匹配）
    local reporter_process=$(docker exec lars_reporter ps aux 2>/dev/null | grep -E "[l]ars_reporter|bin/lars_reporter" | wc -l | tr -d ' ')
    if [ "$reporter_process" -gt 0 ]; then
        # 从宿主机测试端口映射（使用 nc，兼容 macOS）
        if nc -z -w 2 localhost 7777 2>/dev/null; then
            echo "  ✓ 端口 7777 可连接（从宿主机）"
        else
            echo "  ⚠ 服务运行中（$reporter_process 个进程），但端口 7777 无法从宿主机连接（可能服务正在启动或绑定问题）"
        fi
    else
        echo "  ✗ 服务未运行"
    fi
    
    echo "测试 lars_loadbalance_agent:"
    # 检查进程是否运行
    if docker exec lars_loadbalance_agent ps aux | grep -q "[l]ars_loadbalance_agent"; then
        local process_count=$(docker exec lars_loadbalance_agent ps aux | grep -c "[l]ars_loadbalance_agent")
        echo "  ✓ 服务运行中（$process_count 个进程）"
        
        # 检查 UDP 端口监听（8888, 8889, 8890）
        echo "  检查 UDP 端口（8888, 8889, 8890）："
        local udp_ports=(8888 8889 8890)
        local udp_ok=0
        for port in "${udp_ports[@]}"; do
            # 检查 /proc/net/udp 中的端口（十六进制）
            local hex_port=$(printf "%04X" $port)
            if docker exec lars_loadbalance_agent bash -c "cat /proc/net/udp 2>/dev/null | grep -qi $hex_port"; then
                echo "    ✓ UDP 端口 $port 已监听"
                udp_ok=$((udp_ok + 1))
            else
                echo "    ⚠ UDP 端口 $port 未检测到监听（可能正在启动）"
            fi
        done
        
        if [ "$udp_ok" -eq 3 ]; then
            echo "  ✓ 所有 UDP 端口（8888, 8889, 8890）已监听"
        elif [ "$udp_ok" -gt 0 ]; then
            echo "  ⚠ 部分 UDP 端口已监听（$udp_ok/3）"
        fi
        
        # 注意：lars_loadbalance_agent 使用 TCP 客户端连接到 lars_reporter 和 lars_dns
        # 但不提供 TCP 服务器端口，只有 3 个 UDP 服务器端口
    else
        echo "  ✗ 服务未运行"
    fi
    
    echo "测试 MySQL (3306 - TCP):"
    docker exec lars_mysql mysql -uroot -p20030329 -e "SELECT 'MySQL 连接成功' AS status;" 2>/dev/null && echo "  ✓ MySQL 可连接" || echo "  ✗ MySQL 不可连接"
}

# 主逻辑
case "${1:-}" in
    status)
        status
        ;;
    logs)
        logs "$2"
        ;;
    logs-tail)
        logs_tail "$2" "$3"
        ;;
    shell)
        shell "$2"
        ;;
    restart)
        restart "$2"
        ;;
    stop)
        stop "$2"
        ;;
    start)
        start "$2"
        ;;
    exec)
        exec_cmd "$2" "$3"
        ;;
    ps)
        ps_cmd "$2"
        ;;
    netstat)
        netstat "$2"
        ;;
    mysql)
        mysql
        ;;
    test)
        test_connection
        ;;
    help|--help|-h|"")
        show_help
        ;;
    *)
        echo "未知命令: $1"
        echo ""
        show_help
        exit 1
        ;;
esac
