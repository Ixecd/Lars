#!/bin/bash
# OrbStack 快速设置脚本

echo "=== OrbStack Docker 环境设置 ==="
echo ""

# 0. 检查 Docker Desktop 是否在运行
echo "0. 检查 Docker Desktop 状态..."
if ps aux | grep -i "docker desktop" | grep -v grep > /dev/null; then
    echo "   ⚠️  检测到 Docker Desktop 正在运行"
    echo "   建议：关闭 Docker Desktop，只使用 OrbStack"
    echo "   原因：两者不能同时运行，会冲突"
    read -p "   是否继续？(y/n) " -n 1 -r
    echo
    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        echo "   已取消"
        exit 1
    fi
else
    echo "   ✓ Docker Desktop 未运行（正常）"
fi

# 1. 检查并切换到 OrbStack 上下文
echo ""
echo "1. 检查 Docker 上下文..."
CURRENT_CTX=$(docker context show)
echo "   当前上下文: $CURRENT_CTX"

if [ "$CURRENT_CTX" != "orbstack" ]; then
    echo "   切换到 OrbStack 上下文..."
    docker context use orbstack
else
    echo "   ✓ 已使用 OrbStack 上下文"
fi

# 2. 验证
echo ""
echo "2. 验证 Docker 环境..."
docker info | grep -E "Context|Name" | head -2

# 3. 提示
echo ""
echo "=== 设置完成 ==="
echo ""
echo "✓ 现在所有 docker 命令创建的容器都会在 OrbStack GUI 中显示"
echo "✓ 在 OrbStack 应用中查看 Containers 列表即可看到所有容器"
echo ""
echo "重要提示："
echo "  - 使用 OrbStack 时，不需要打开 Docker Desktop"
echo "  - OrbStack 和 Docker Desktop 不能同时运行"
echo "  - 所有 docker 命令都会通过 OrbStack 执行"
echo ""
echo "如果容器不显示，请："
echo "  1. 确保 OrbStack 应用已启动"
echo "  2. 重启 OrbStack 应用"
echo "  3. 在 OrbStack GUI 中点击刷新"
echo "  4. 运行: docker ps -a 验证容器是否存在"
