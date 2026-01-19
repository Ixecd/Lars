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
            fi
        done
    else
        check_service $service
        echo "重启 ${service}..."
        docker restart $service
    fi
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
            fi
        done
    else
        check_service $service
        echo "启动 ${service}..."
        docker start $service
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
    docker exec $service ss -tlnp 2>/dev/null || docker exec $service netstat -tlnp 2>/dev/null || echo "无法获取端口信息"
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
    echo "测试 lars_dns (7775):"
    docker exec lars_dns bash -c "timeout 2 bash -c '</dev/tcp/localhost/7775' 2>/dev/null && echo '  ✓ 端口 7775 可连接' || echo '  ✗ 端口 7775 不可连接'"
    
    echo "测试 lars_reporter (7777):"
    docker exec lars_reporter bash -c "timeout 2 bash -c '</dev/tcp/localhost/7777' 2>/dev/null && echo '  ✓ 端口 7777 可连接' || echo '  ✗ 端口 7777 不可连接'"
    
    echo "测试 lars_loadbalance_agent (8888):"
    docker exec lars_loadbalance_agent bash -c "timeout 2 bash -c '</dev/tcp/localhost/8888' 2>/dev/null && echo '  ✓ 端口 8888 可连接' || echo '  ✗ 端口 8888 不可连接'"
    
    echo "测试 MySQL (3306):"
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
