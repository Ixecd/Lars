# Lars 服务容器化部署指南

## 一、整体架构

### 服务组成
- **lars_mysql**: MySQL 8.0 数据库容器
- **lars_dns**: DNS 路由服务（端口 7775）
- **lars_reporter**: 状态上报服务（端口 7777）
- **lars_loadbalance_agent**: 负载均衡代理服务（端口 8888-8890）

### 网络架构
```
┌─────────────────────────────────────────┐
│         lars_network (Docker)           │
│                                         │
│  ┌──────────┐    ┌───────────┐          │
│  │ lars_dns │◄──►│lars_mysql │          │
│  │  :7775   │    │  :3306    │          │
│  └────┬─────┘    └─────┬─────┘          │
│       │                │                │
│       │          ┌─────▼───────┐        │
│       └─────────►│lars_reporter│        │
│                  │  :7777      │        │
│                  └─────┬───────┘        │
│                        │                │
│                  ┌─────▼───────────┐    │
│                  │lars_loadbalance │    │
│                  │  :8888-8890     │    │
│                  └─────────────────┘    │
└─────────────────────────────────────────┘
```

## 二、详细部署步骤

### 前置条件：确保使用 OrbStack

**如果使用 OrbStack**，需要确保 Docker 使用 OrbStack 上下文：

```bash
# 检查当前 Docker 上下文
docker context ls

# 切换到 OrbStack 上下文（如果未激活）
docker context use orbstack

# 验证
docker info | grep -i "name\|context"
```

**为什么需要这个？**
- OrbStack 有自己的 Docker 环境
- 使用正确的上下文确保容器在 OrbStack GUI 中可见
- 容器会在 OrbStack 的容器列表中显示

---

### 步骤 1: 创建 Docker 网络

**目的**: 让所有容器在同一个网络中，可以通过容器名称互相访问

```bash
docker network create lars_network
```

**验证**:
```bash
docker network ls | grep lars_network
```

**为什么需要网络？**
- 默认情况下，容器之间无法直接通信
- 创建网络后，容器可以通过容器名称（如 `lars_mysql`）互相访问
- 不需要知道容器的 IP 地址

---

### 步骤 2: 创建并启动 MySQL 容器

**命令**:
```bash
docker run -d \
  --name lars_mysql \
  --network lars_network \
  -e MYSQL_ROOT_PASSWORD=20030329 \
  -e MYSQL_DATABASE=lars_dns \
  -p 3306:3306 \
  mysql:8.0
```

**参数说明**:
- `-d`: 后台运行
- `--name lars_mysql`: 容器名称（其他容器通过此名称访问）
- `--network lars_network`: 加入指定网络
- `-e MYSQL_ROOT_PASSWORD`: 设置 root 密码
- `-e MYSQL_DATABASE`: 自动创建数据库
- `-p 3306:3306`: 端口映射（宿主机:容器）

**等待 MySQL 启动**:
```bash
# MySQL 需要几秒钟初始化
sleep 10

# 验证 MySQL 是否启动成功
docker exec lars_mysql mysql -uroot -p20030329 -e "SELECT 'MySQL 启动成功' AS status;"
```

---

### 步骤 3: 初始化数据库表结构

**导入 SQL 文件**:
```bash
# 方法 1: 使用 docker exec -i（推荐）
docker exec -i lars_mysql mysql -uroot -p20030329 < base/sql/DnsServer.sql

# 方法 2: 先复制文件到容器，再执行
docker cp base/sql/DnsServer.sql lars_mysql:/tmp/
docker exec lars_mysql mysql -uroot -p20030329 < /tmp/DnsServer.sql

# 导入 report 表
docker exec -i lars_mysql mysql -uroot -p20030329 < base/sql/report.sql
```

**验证表结构**:
```bash
docker exec lars_mysql mysql -uroot -p20030329 -e "USE lars_dns; SHOW TABLES;"
```

**应该看到**:
- RouteData
- RouteVersion
- RouteChange
- ServerCallStatus

---

### 步骤 4: 更新配置文件

**修改数据库连接地址**:

1. **lars_dns/conf/lars.conf**:
```ini
[mysql]
db_host = lars_mysql    # 改为容器名称
db_port = 3306
db_user = root
db_passwd = 20030329
db_name = lars_dns
```

2. **lars_reporter/config/lars_reporter.conf**:
```ini
[mysql]
db_host = lars_mysql    # 改为容器名称
db_port = 3306
db_user = root
db_passwd = 20030329
db_name = lars_dns
```

3. **lars_loadbalance_agent/conf/lars_lb_agent.conf**:
```ini
[reporter]
ip = lars_reporter      # 改为容器名称
port = 7777

[dnsserver]
ip = lars_dns           # 改为容器名称
port = 7775
```

**关键点**: 
- 在 Docker 网络中，容器可以通过容器名称互相访问
- 不需要使用 `localhost` 或 `127.0.0.1`
- 不需要使用 `host.docker.internal`（那是访问宿主机用的）

---

### 步骤 5: 创建服务容器

#### 5.1 创建 lars_dns 容器

```bash
docker run -d \
  --name lars_dns \
  --network lars_network \
  --platform linux/amd64 \
  -v /Users/qc/Lars:/lars \
  -w /lars \
  -p 7775:7775 \
  ubuntu:24.04 \
  bash -c "apt-get update -qq && apt-get install -y -qq libmysqlclient-dev libprotobuf-dev && tail -f /dev/null"
```

**参数说明**:
- `--platform linux/amd64`: 指定平台（如果宿主机是 ARM Mac）
- `-v /Users/qc/Lars:/lars`: 挂载项目目录到容器
- `-w /lars`: 设置工作目录
- `-p 7775:7775`: 端口映射

#### 5.2 创建 lars_reporter 容器

```bash
docker run -d \
  --name lars_reporter \
  --network lars_network \
  --platform linux/amd64 \
  -v /Users/qc/Lars:/lars \
  -w /lars \
  -p 7777:7777 \
  ubuntu:24.04 \
  bash -c "apt-get update -qq && apt-get install -y -qq libmysqlclient-dev libprotobuf-dev && tail -f /dev/null"
```

#### 5.3 创建 lars_loadbalance_agent 容器

```bash
docker run -d \
  --name lars_loadbalance_agent \
  --network lars_network \
  --platform linux/amd64 \
  -v /Users/qc/Lars:/lars \
  -w /lars \
  -p 8888:8888 -p 8889:8889 -p 8890:8890 \
  ubuntu:24.04 \
  bash -c "apt-get update -qq && apt-get install -y -qq libprotobuf-dev && tail -f /dev/null"
```

---

### 步骤 6: 安装运行时依赖库

#### 6.1 解决 MySQL 客户端库版本问题

**问题**: 编译的二进制需要 `libmysqlclient.so.21`，但 Ubuntu 24.04 默认只有 MariaDB 库

**解决方案**: 创建符号链接（如果使用 MariaDB）或安装 MySQL 客户端库

```bash
# 在 lars_dns 容器中
docker exec lars_dns bash -c "
  cd /usr/lib/x86_64-linux-gnu
  # 如果使用 MariaDB
  ln -sf libmariadb.so.3 libmysqlclient.so.21
  ln -sf libmysqlclient.so.21 libmysqlclient.so.18
  # 或者安装 MySQL 官方客户端库
"

# 在 lars_reporter 容器中（同样操作）
docker exec lars_reporter bash -c "
  cd /usr/lib/x86_64-linux-gnu
  ln -sf libmariadb.so.3 libmysqlclient.so.21
  ln -sf libmysqlclient.so.21 libmysqlclient.so.18
"
```

#### 6.2 解决 Protobuf 库版本问题

```bash
# 创建 protobuf 符号链接
docker exec lars_dns bash -c "
  cd /usr/lib/x86_64-linux-gnu
  ln -sf libprotobuf.so.32 libprotobuf.so.30
"

docker exec lars_reporter bash -c "
  cd /usr/lib/x86_64-linux-gnu
  ln -sf libprotobuf.so.32 libprotobuf.so.30
"

docker exec lars_loadbalance_agent bash -c "
  cd /usr/lib/x86_64-linux-gnu
  ln -sf libprotobuf.so.32 libprotobuf.so.30
"
```

---

### 步骤 7: 启动服务

#### 7.1 启动 lars_dns

```bash
docker exec -d lars_dns bash -c "
  cd /lars
  /lars/lars_dns/bin/lars_dns /lars/lars_dns/conf/lars.conf > /tmp/lars_dns.log 2>&1
"

# 检查日志
docker exec lars_dns tail -20 /tmp/lars_dns.log

# 检查进程
docker exec lars_dns ps aux | grep lars_dns
```

#### 7.2 启动 lars_reporter

```bash
docker exec -d lars_reporter bash -c "
  cd /lars
  /lars/lars_reporter/bin/lars_reporter /lars/lars_reporter/config/lars_reporter.conf > /tmp/lars_reporter.log 2>&1
"

# 检查日志
docker exec lars_reporter tail -20 /tmp/lars_reporter.log
```

#### 7.3 启动 lars_loadbalance_agent

```bash
docker exec -d lars_loadbalance_agent bash -c "
  cd /lars
  /lars/lars_loadbalance_agent/bin/lars_loadbalance_agent /lars/lars_loadbalance_agent/conf/lars_lb_agent.conf > /tmp/lars_lb.log 2>&1
"

# 检查日志
docker exec lars_loadbalance_agent tail -20 /tmp/lars_lb.log
```

---

### 步骤 8: 验证服务状态

```bash
# 查看所有容器状态
docker ps --format "table {{.Names}}\t{{.Status}}\t{{.Ports}}" | grep lars

# 检查服务进程
docker exec lars_dns ps aux | grep lars_dns
docker exec lars_reporter ps aux | grep lars_reporter
docker exec lars_loadbalance_agent ps aux | grep lars_loadbalance

# 检查端口监听
docker exec lars_dns ss -tlnp | grep 7775
docker exec lars_reporter ss -tlnp | grep 7777
docker exec lars_loadbalance_agent ss -tlnp | grep -E '8888|8889|8890'

# 测试服务连接
docker exec lars_dns curl -v telnet://lars_mysql:3306
docker exec lars_loadbalance_agent ping -c 1 lars_dns
```

---

## 三、常见问题及解决方案

### 问题 1: 容器无法启动或立即退出

**症状**: `docker ps -a` 显示容器状态为 `Exited`

**可能原因**:
1. 启动命令执行失败
2. 容器内进程崩溃

**解决方案**:
```bash
# 查看容器日志
docker logs lars_dns
docker logs lars_reporter

# 查看退出状态码
docker inspect lars_dns | grep -A 10 "State"

# 交互式进入容器调试
docker exec -it lars_dns bash
```

---

### 问题 2: 服务无法连接 MySQL

**症状**: 日志显示 "Can't connect to MySQL server"

**可能原因**:
1. MySQL 容器未启动
2. 配置文件中的主机名错误
3. 网络配置问题

**解决方案**:
```bash
# 1. 检查 MySQL 容器状态
docker ps | grep lars_mysql

# 2. 测试网络连通性
docker exec lars_dns ping -c 2 lars_mysql

# 3. 测试 MySQL 连接
docker exec lars_dns mysql -uroot -p20030329 -hlars_mysql -e "SELECT 1"

# 4. 检查配置文件
docker exec lars_dns cat /lars/lars_dns/conf/lars.conf | grep db_host

# 5. 检查 MySQL 是否允许远程连接
docker exec lars_mysql mysql -uroot -p20030329 -e "SELECT host, user FROM mysql.user;"
```

---

### 问题 3: 缺少共享库文件

**症状**: `error while loading shared libraries: libmysqlclient.so.21: cannot open shared object file`

**可能原因**:
1. 库文件未安装
2. 库文件路径不正确
3. 符号链接未创建

**解决方案**:
```bash
# 1. 检查库文件是否存在
docker exec lars_dns find /usr -name "*mysqlclient*.so*"

# 2. 检查符号链接
docker exec lars_dns ls -la /usr/lib/x86_64-linux-gnu/libmysqlclient.so*

# 3. 重新创建符号链接
docker exec lars_dns bash -c "
  cd /usr/lib/x86_64-linux-gnu
  rm -f libmysqlclient.so*
  ln -sf libmariadb.so.3 libmysqlclient.so.21
  ln -sf libmysqlclient.so.21 libmysqlclient.so.18
"

# 4. 检查库依赖
docker exec lars_dns ldd /lars/lars_dns/bin/lars_dns | grep mysql
```

---

### 问题 4: 库版本符号不匹配

**症状**: `version 'libmysqlclient_21.0' not found`

**可能原因**:
- MariaDB 库与 MySQL 库的符号不兼容

**解决方案**:
```bash
# 方案 1: 安装 MySQL 官方客户端库（推荐）
docker exec lars_dns bash -c "
  apt-get update
  apt-get install -y -qq wget
  wget https://dev.mysql.com/get/mysql-apt-config_0.8.24-1_all.deb
  dpkg -i mysql-apt-config_0.8.24-1_all.deb
  apt-get update
  apt-get install -y -qq libmysqlclient-dev
"

# 方案 2: 重新编译服务以链接 MariaDB 库
# 修改 CMakeLists.txt，使用 libmariadb 而不是 libmysqlclient
```

---

### 问题 5: 服务间无法通信

**症状**: 服务无法连接到其他服务容器

**可能原因**:
1. 容器不在同一网络
2. 配置文件中的服务地址错误
3. 服务未启动

**解决方案**:
```bash
# 1. 检查网络配置
docker network inspect lars_network

# 2. 检查容器是否在同一网络
docker inspect lars_dns | grep -A 10 "Networks"
docker inspect lars_reporter | grep -A 10 "Networks"

# 3. 测试容器间连通性
docker exec lars_loadbalance_agent ping -c 2 lars_dns
docker exec lars_loadbalance_agent ping -c 2 lars_reporter

# 4. 检查配置文件中的服务地址
docker exec lars_loadbalance_agent cat /lars/lars_loadbalance_agent/conf/lars_lb_agent.conf | grep -E 'ip|port'
```

---

### 问题 6: 端口冲突

**症状**: `bind: address already in use`

**可能原因**:
- 宿主机端口已被占用

**解决方案**:
```bash
# 1. 检查端口占用
lsof -i :7775
lsof -i :7777
lsof -i :8888

# 2. 修改端口映射
docker run -p 7776:7775 ...  # 使用不同的宿主机端口

# 3. 停止占用端口的进程
kill -9 <PID>
```

---

### 问题 7: 配置文件路径硬编码

**症状**: 服务无法找到配置文件

**可能原因**:
- 代码中硬编码了配置文件路径（如 `/home/qc/Lars/...`）

**解决方案**:
```cpp
// 修改 main 函数，使用命令行参数
int main(int argc, char **argv) {
    std::string config_path = (argc > 1) ? argv[1] : "默认路径";
    config_file::setPath(config_path);
    // ...
}
```

---

### 问题 8: 容器重启后服务未自动启动

**症状**: 容器重启后，服务进程消失

**可能原因**:
- 服务是在容器启动后手动启动的，不是容器启动命令的一部分

**解决方案**:
```bash
# 方案 1: 使用启动脚本
# 创建启动脚本 start_services.sh
cat > start_services.sh << 'EOF'
#!/bin/bash
cd /lars
/lars/lars_dns/bin/lars_dns /lars/lars_dns/conf/lars.conf &
/lars/lars_reporter/bin/lars_reporter /lars/lars_reporter/config/lars_reporter.conf &
/lars/lars_loadbalance_agent/bin/lars_loadbalance_agent /lars/lars_loadbalance_agent/conf/lars_lb_agent.conf &
wait
EOF

# 修改容器启动命令
docker run -d --name lars_dns ... ubuntu:24.04 /lars/start_services.sh

# 方案 2: 使用 systemd 或 supervisor（更复杂但更可靠）
```

---

### 问题 9: 数据持久化

**症状**: 容器删除后，MySQL 数据丢失

**解决方案**:
```bash
# 使用数据卷持久化 MySQL 数据
docker run -d \
  --name lars_mysql \
  -v lars_mysql_data:/var/lib/mysql \
  mysql:8.0

# 查看数据卷
docker volume ls
docker volume inspect lars_mysql_data
```

---

### 问题 10: 资源限制

**症状**: 容器占用过多资源

**解决方案**:
```bash
# 限制容器资源使用
docker run -d \
  --name lars_dns \
  --memory="512m" \
  --cpus="1.0" \
  ...
```

---

## 四、最佳实践

### 1. 使用 Docker Compose（推荐）

创建 `docker-compose.yml`:
```yaml
version: '3.8'

services:
  mysql:
    image: mysql:8.0
    container_name: lars_mysql
    environment:
      MYSQL_ROOT_PASSWORD: 20030329
      MYSQL_DATABASE: lars_dns
    volumes:
      - mysql_data:/var/lib/mysql
      - ./base/sql:/docker-entrypoint-initdb.d
    ports:
      - "3306:3306"
    networks:
      - lars_network

  dns:
    build: ./lars_dns
    container_name: lars_dns
    depends_on:
      - mysql
    volumes:
      - ./lars_dns:/lars
    ports:
      - "7775:7775"
    networks:
      - lars_network

  reporter:
    build: ./lars_reporter
    container_name: lars_reporter
    depends_on:
      - mysql
    volumes:
      - ./lars_reporter:/lars
    ports:
      - "7777:7777"
    networks:
      - lars_network

  loadbalance:
    build: ./lars_loadbalance_agent
    container_name: lars_loadbalance_agent
    depends_on:
      - dns
      - reporter
    volumes:
      - ./lars_loadbalance_agent:/lars
    ports:
      - "8888-8890:8888-8890"
    networks:
      - lars_network

networks:
  lars_network:
    driver: bridge

volumes:
  mysql_data:
```

启动所有服务:
```bash
docker-compose up -d
```

### 2. 健康检查

```yaml
healthcheck:
  test: ["CMD", "mysql", "-uroot", "-p20030329", "-e", "SELECT 1"]
  interval: 10s
  timeout: 5s
  retries: 3
```

### 3. 日志管理

```bash
# 配置日志驱动
docker run --log-driver json-file --log-opt max-size=10m --log-opt max-file=3 ...
```

---

## 五、OrbStack 集成说明

### 确保容器在 OrbStack GUI 中显示

**问题**: 使用 `docker` 命令创建的容器不显示在 OrbStack GUI 中

**解决方案**:

1. **确保使用 OrbStack 上下文**:
```bash
# 检查当前上下文
docker context ls

# 切换到 OrbStack（如果未激活）
docker context use orbstack

# 验证
docker info | grep -i "name\|context"
# 应该显示: Context: orbstack, Name: orbstack
```

2. **创建容器时使用标准 Docker 命令**:
   - 所有使用 `docker run`、`docker-compose` 等命令创建的容器
   - 都会自动出现在 OrbStack GUI 的 Containers 列表中
   - 无需特殊配置

3. **如果容器仍然不显示**:
   - 重启 OrbStack 应用
   - 在 OrbStack GUI 中点击刷新按钮
   - 检查容器是否真的在运行: `docker ps -a`

4. **在 OrbStack GUI 中管理容器**:
   - 查看容器日志
   - 进入容器终端
   - 启动/停止/删除容器
   - 查看资源使用情况

**注意**: 
- OrbStack 使用标准的 Docker API
- 所有 Docker 命令都会在 OrbStack 环境中执行
- 容器、镜像、网络、卷等都会在 OrbStack GUI 中显示

---

## 六、常用命令速查

```bash
# 查看所有容器
docker ps -a | grep lars

# 查看容器日志
docker logs -f lars_dns

# 进入容器
docker exec -it lars_dns bash

# 重启容器
docker restart lars_dns

# 停止所有服务
docker stop lars_dns lars_reporter lars_loadbalance_agent lars_mysql

# 删除所有容器
docker rm -f lars_dns lars_reporter lars_loadbalance_agent lars_mysql

# 删除网络
docker network rm lars_network

# 查看网络详情
docker network inspect lars_network

# 查看容器资源使用
docker stats lars_dns lars_reporter lars_loadbalance_agent
```

---

## 六、故障排查流程

1. **检查容器状态**: `docker ps -a`
2. **查看容器日志**: `docker logs <container_name>`
3. **进入容器调试**: `docker exec -it <container_name> bash`
4. **检查网络连通性**: `ping`, `telnet`, `nc`
5. **检查服务进程**: `ps aux | grep <service>`
6. **检查端口监听**: `ss -tlnp` 或 `netstat -tlnp`
7. **检查配置文件**: `cat <config_file>`
8. **检查库依赖**: `ldd <binary>`

---

## 七、总结

容器化部署多服务的核心要点：

1. **网络隔离**: 使用 Docker 网络让服务互相通信
2. **服务发现**: 通过容器名称进行服务发现
3. **配置管理**: 修改配置文件使用容器名称而非 localhost
4. **依赖管理**: 确保每个容器都有必要的运行时库
5. **数据持久化**: 使用数据卷保存重要数据
6. **监控和日志**: 建立完善的日志和监控机制

通过以上步骤，可以成功将多个服务部署到独立的容器中，实现微服务架构的容器化部署。
