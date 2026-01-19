# Lars 服务快速运维指南

## 为什么进入容器是新终端？

**问题**: 使用 `docker exec -it lars_dns bash` 进入容器后，看不到服务运行的终端输出。

**原因**:
- 服务使用 `docker exec -d` 以后台方式启动
- 后台进程的输出不会显示在新终端中
- 每个 `docker exec` 都会创建新的终端会话

**解决方案**:

### 1. 查看服务日志（推荐方式）

```bash
# 实时查看日志（相当于在服务运行的终端中查看）
docker logs -f lars_dns

# 或使用运维脚本
./docker_ops.sh logs lars_dns
```

### 2. 在容器中执行命令

```bash
# 查看服务进程
docker exec lars_dns ps aux | grep lars_dns

# 查看端口监听
docker exec lars_dns ss -tlnp

# 或使用运维脚本
./docker_ops.sh exec lars_dns "ps aux | grep lars"
```

### 3. 以前台方式运行（用于调试）

如果需要看到服务的实时输出，可以停止后台服务，以前台方式启动：

```bash
# 进入容器
docker exec -it lars_dns bash

# 在容器内停止后台服务
pkill lars_dns

# 以前台方式启动（可以看到所有输出）
cd /lars/lars_dns
./bin/lars_dns conf/lars.conf
# 现在可以看到服务的所有输出
# 按 Ctrl+C 可以停止服务
```

## 日常运维命令

### 使用运维脚本（推荐）

```bash
# 查看所有服务状态
./docker_ops.sh status

# 查看服务日志（实时）
./docker_ops.sh logs lars_dns

# 查看服务日志（最后N行）
./docker_ops.sh logs-tail lars_dns 100

# 进入容器终端
./docker_ops.sh shell lars_dns

# 在容器中执行命令
./docker_ops.sh exec lars_dns "ps aux | grep lars"

# 查看端口监听
./docker_ops.sh netstat lars_dns

# 重启服务
./docker_ops.sh restart lars_dns

# 进入 MySQL
./docker_ops.sh mysql

# 测试服务连接
./docker_ops.sh test
```

### 直接使用 Docker 命令

```bash
# 查看所有容器
docker ps | grep lars

# 查看服务日志
docker logs -f lars_dns

# 进入容器
docker exec -it lars_dns bash

# 在容器中执行命令
docker exec lars_dns ps aux | grep lars_dns

# 重启服务
docker restart lars_dns
```

## 常见运维场景

### 场景1: 查看服务是否正常运行

```bash
# 方法1: 使用运维脚本
./docker_ops.sh status

# 方法2: 直接查看
docker ps | grep lars
docker exec lars_dns ps aux | grep lars_dns
```

### 场景2: 查看服务输出/日志

```bash
# 实时查看（推荐）
./docker_ops.sh logs lars_dns

# 查看最后100行
./docker_ops.sh logs-tail lars_dns 100
```

### 场景3: 调试服务问题

```bash
# 1. 查看日志
./docker_ops.sh logs lars_dns

# 2. 进入容器
./docker_ops.sh shell lars_dns

# 3. 在容器内执行调试命令
ps aux | grep lars
ss -tlnp
cat /lars/lars_dns/conf/lars.conf
```

### 场景4: 重启服务

```bash
# 方法1: 重启容器（会重启所有进程）
./docker_ops.sh restart lars_dns

# 方法2: 只重启服务进程（不重启容器）
docker exec lars_dns pkill lars_dns
docker exec -d lars_dns bash -c "cd /lars/lars_dns && ./bin/lars_dns conf/lars.conf"
```

### 场景5: 查看 MySQL 数据

```bash
# 进入 MySQL 客户端
./docker_ops.sh mysql

# 或直接执行 SQL
docker exec lars_mysql mysql -uroot -p20030329 -e "USE lars_dns; SELECT * FROM RouteData LIMIT 5;"
```

## 提示

- **查看日志** = 在服务运行的"终端"中查看输出
- **进入容器** = 进入新的终端，可以执行命令，但看不到服务输出
- **使用运维脚本** = 更方便，统一管理所有服务
