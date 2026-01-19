# 头文件 Include 路径指南

## 一、.clangd 配置的路径映射

`.clangd` 中配置的 `-I` 路径决定了如何在源文件中 include 头文件。

### 配置的路径：

```yaml
-I/Users/qc/Lars/base                    # base 目录
-I/Users/qc/Lars/lars_reactor/include    # lars_reactor/include 目录
-I/Users/qc/Lars/lars_reporter/include   # lars_reporter/include 目录
-I/Users/qc/Lars/lars_dns/include        # lars_dns/include 目录
-I/Users/qc/Lars/lars_loadbalance_agent/include  # lars_loadbalance_agent/include 目录
-I/Users/qc/Lars/base/proto              # base/proto 目录
-I/opt/homebrew/include                  # 系统库（protobuf 等）
```

## 二、Include 方式说明

### 规则：
- `-I/Users/qc/Lars/base` 意味着：`base/` 目录下的文件可以直接通过相对路径访问
- `-I/Users/qc/Lars/lars_reactor/include` 意味着：`include/` 目录下的文件可以直接访问

### 实际映射：

#### 1. Proto 文件
```
实际路径: /Users/qc/Lars/base/proto/lars.pb.h
Include 方式: #include <proto/lars.pb.h>
原因: -I/Users/qc/Lars/base，所以 base/proto/lars.pb.h → <proto/lars.pb.h>
```

#### 2. lars_reactor 头文件
```
实际路径: /Users/qc/Lars/lars_reactor/include/lars_reactor/lars_reactor.hpp
Include 方式: #include <lars_reactor/lars_reactor.hpp>
原因: -I/Users/qc/Lars/lars_reactor/include，所以 include/lars_reactor/xxx.hpp → <lars_reactor/xxx.hpp>
```

#### 3. lars_reporter 头文件
```
实际路径: /Users/qc/Lars/lars_reporter/include/lars_reporter/store_report.hpp
Include 方式: #include <lars_reporter/store_report.hpp>
原因: -I/Users/qc/Lars/lars_reporter/include
```

#### 4. lars_dns 头文件
```
实际路径: /Users/qc/Lars/lars_dns/include/lars_dns/dns_route.hpp
Include 方式: #include <lars_dns/dns_route.hpp>
原因: -I/Users/qc/Lars/lars_dns/include
```

#### 5. lars_loadbalance_agent 头文件
```
实际路径: /Users/qc/Lars/lars_loadbalance_agent/include/lars_loadbalance_agent/load_balance.hpp
Include 方式: #include <lars_loadbalance_agent/load_balance.hpp>
原因: -I/Users/qc/Lars/lars_loadbalance_agent/include
```

#### 6. 系统库（protobuf）
```
实际路径: /opt/homebrew/include/google/protobuf/message.h
Include 方式: #include <google/protobuf/message.h>
原因: -I/opt/homebrew/include
```

## 三、Include 规则总结

### 使用尖括号 `<>` 的情况：
- 项目内的头文件（通过 -I 配置的路径）
- 系统库头文件

### 使用双引号 `""` 的情况：
- 相对路径的头文件（如 `#include "mysql.h"`）
- 同一目录下的头文件

### 示例：

```cpp
// ✅ 正确 - 使用尖括号，因为路径在 -I 配置中
#include <proto/lars.pb.h>
#include <lars_reactor/lars_reactor.hpp>
#include <lars_dns/dns_route.hpp>
#include <lars_reporter/store_report.hpp>
#include <lars_loadbalance_agent/load_balance.hpp>
#include <google/protobuf/message.h>

// ✅ 也可以 - 使用双引号（如果知道相对路径）
#include "mysql.h"  // 如果 mysql.h 在系统路径或当前目录

// ❌ 错误 - 不要这样写
#include "proto/lars.pb.h"  // 虽然可能工作，但不推荐
#include "/Users/qc/Lars/base/proto/lars.pb.h"  // 绝对路径，不推荐
```

## 四、路径查找顺序

当使用 `#include <xxx>` 时，编译器会按以下顺序查找：

1. 首先查找 `-I` 配置的路径
2. 然后查找系统标准路径（如 `/usr/include`）
3. 最后查找编译器默认路径

## 五、常见问题

### Q: 为什么 `#include <proto/lars.pb.h>` 可以工作？
A: 因为 `.clangd` 配置了 `-I/Users/qc/Lars/base`，所以 `base/proto/lars.pb.h` 可以通过 `<proto/lars.pb.h>` 访问。

### Q: 为什么 `#include <lars_reactor/lars_reactor.hpp>` 可以工作？
A: 因为配置了 `-I/Users/qc/Lars/lars_reactor/include`，所以 `include/lars_reactor/lars_reactor.hpp` 可以通过 `<lars_reactor/lars_reactor.hpp>` 访问。

### Q: 可以修改 include 方式吗？
A: 可以，但需要保持一致性。推荐使用尖括号 `<>` 来 include 项目内的头文件，这样更清晰。

## 六、快速参考

| 头文件类型 | Include 方式 | 示例 |
|-----------|-------------|------|
| proto 文件 | `<proto/文件名>` | `#include <proto/lars.pb.h>` |
| lars_reactor | `<lars_reactor/文件名>` | `#include <lars_reactor/lars_reactor.hpp>` |
| lars_dns | `<lars_dns/文件名>` | `#include <lars_dns/dns_route.hpp>` |
| lars_reporter | `<lars_reporter/文件名>` | `#include <lars_reporter/store_report.hpp>` |
| lars_loadbalance_agent | `<lars_loadbalance_agent/文件名>` | `#include <lars_loadbalance_agent/load_balance.hpp>` |
| 系统库 | `<库名/文件名>` | `#include <google/protobuf/message.h>` |
