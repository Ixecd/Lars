# Lars C++负载均衡远程调度服务系统
**整体框架**
- 一个服务称为一个模块,一个模块由modid + cmdid来标识,一个模块表示一个远程服务,这个远程服务一般部署在多个节点上
- Lars Balance以UDP的方式为业务提供
    1. 节点获取服务
    2. 节点调用结果上报服务
- 业务一 节点获取服务:
    1. 利用modid + cmdid去向LB Agent获取一个可用节点
    2. 向该节点发送消息,完成一次远程调用
    3. 具体获取modid + cmdid下的哪一个节点由LB Agent决定
- 业务二 节点调用结果上报服务:
        首先通过业务一根据LB Agent获取节点,调用结果会汇报给LB Agent,以便LB Agent根据自己的LB算法来感知远程服务节点的状态是空闲还是
        过载,进而控制节点获取时的节点调度.
    
- 整体框架示意图
```
                                                           --------------<--------------     
                                                           |        update route       |
    business one ---> GetNode --->  Thread1    -------> LB Algo                        |
                                UDP Server:8888                \                       |               获取route
                                                                 MQ消息队列 ---> Thread4 Dns service Client <-> Report Service
    business two ---> GetNode --->  Thread2    -------> LB Algo                                        
                                UDP Server:8889                  MQ消息队列 ---> Thread4 Dns service Client <-> Report Service
                                                               /                                        上报状态
    business thr ---> GetNode --->  Thread3    -------> LB Algo 
                                UDP Server:8890
```
- Lars Load—Balance Agent 一共由五个线程组成 一个LB算法构成.

- UDP Server服务,并运行LB算法,对业务提供节点获取和节点调用结果上报服务,为了增大系统吞吐量,使用三个thread独立运行LB算法
        (modid + cmdid) % 3 = i的那些模块的服务与调度,由第i + 1个UDP Server线程负责

- Dns Service Client:Dns Server的客户端线程,向dnsserver获取一个模块的节点集合(或称为获取路由);UDP Server会按需向此线程的MQ写入获取路由
        请求,之后Dns Server 再向UDP Server发送路由信息更新.

- Report Service Client: 是report的客户端线程,负责将每个模块下所有节点在一段时间内的调用结果、过载情况上报到report service中,便于观察、做警报;本身消费MQ数据

**负载均衡**
```
                                        选取队头,并重新放入队尾
      ----------------<----------------- -----> 节点1 节点2 节点3 ... 空闲队列   
      |             return            Y |
      | gethost                         |
    API --> modID/cmdID --> 未超过probe -
      |                                 |
      |             return            N |
      ----------------<----------------- -----> 节点4 节点5 节点6 ... 过载队列
                                        给过载节点一个机会,选取队头,并重新放到队尾
```
1. 基础
- 每个模块modid/cmdid下由若干节点,节点的集合称为此模块的路由,对于每个节点都有两种状态:
    1 idle : 节点可用,可作为API请求的节点使用
    2 overload : 节点暂时不可用
- 在请求节点时,有几个关键属性
- API汇报节点调用的结果
    1. 虚拟成功次数 vsucc
    2. 虚拟失败次数 verr
    3. 连接成功次数 contin_succ
    4. 连接失败次数 contin_err
- 这四个字段,在节点状态改变的时候会导致 idle <---> overload 之间的切换
- 这里每个节点就是一个Host主机信息,就是我们需要被管理的主机信息.
- API相当于Agent模块的客户端,也就是业务端调用的请求主机接口.API发送GetHost,发送给API的server端,AgentServer使用UDPserver处理的API网络请求,将其发送给负载均衡算法
- 当API对某模块发起节点获取时
    1. Load Balance从空闲队列中拿出队列的头节点,作为返回节点,之后将其又重新放到队列的尾部
    2. 关于probe机制: 如果此模块过载队列非空,则每次经过probe_num次节点获取后(默认10),给过载队列中的节点一个机会,让API试探性调用一下
    3. 如果空闲队列为空,也会给机会经过probe_num次获取后,返回过载错误

- 所谓调度就是:从空闲队列轮流选择节点,同时利用probe机制,给过载队列中的节点一个机会.

3. host_info与Load Balance初始化
- host_info:表示一个host主机信息
- load_balance:针对一组modid/cmdid的负载均衡模块
- route_lb:和udp_server数量保持一致,每个route_lb负责管理多个load_balance

4. AGENT-UDP-SERVICE -> port 8888 8889 8890

**Reactor 模型服务器框架**
1. 关于EPOLL的事件驱动模型
- 首先,epoll底层是`RbTree` + `双向队列`
- 其次,与其说epoll监听的是Event,不如说监听的是`文件描述符fd的Event`,重点应该放在fd上
- Epoll可以`同时监听很多EPOLLIN/EPOLLOUT事件`,指的是`多个文件描述的事件`,每个文件描述符,在同一时刻只能有一个EPOLLIN事件/一个EPOLLOUT事件/俩都有一个,这里的意思是指对于每一个文件描述符而言EPOLLIN和EPOLLOUT事件最多都只有一个

*详解EPOLLIN和EPOLLOUT*
- EPOLLIN:读事件,当生成一个socket之后,其对应一个缓冲区(soket_buffer,内核态,数据轮询地从通过DMA技术存放在环形缓冲区Ring Buffer(存放sk_buff)中读取,由内核中的ksofttirqd线程读取到socket_buffer中),如果有其他用户向这个socket发送数据,数据会存储到这个缓冲区中,如果缓冲区中有事件就会触发EPOLLIN
- EPOLLOUT:写事件,当生成一个socket之后,其也对应一个缓冲区(内核态),如果这个缓冲区能写数据,就会触发EPOLLOUT(将用户待发送的数据拷贝到sk_buff内存,之后将其加入到socket的缓冲区中,网络协议栈从缓冲区中取出sk_buff,通过传输层/网络层最后到网络接口层逐层处理,准备好之后放入网卡的发送队列,之后触发软中端,告诉网卡驱动程序,有新的网络包要发送,之后驱动程序会从发送队列中读取sk_buff添加到Ring Buffer中,之后将sk_buff数据映射到网卡可访问的内存DMA区域,最后触发真实的发送),一般当生成这个socket的时候,缓冲区也就初始化好了,所以`EPOLLOUT是立即触发的`,所以EPOLLOUT触发后从epoll_wait退出,首先要从epoll中删除对应的EPOLLOUT事件,之后再执行对应的回调函数
- 对于操作系统提供的IO API,比如read/write,recv/send其实都是向socket_buffer中添加数据,并不会真正发送,首先将用户数据拷贝到sk_buff(内核申请的),之后将其添加到socket_buffer中,之后一步一步封装在sk_buff中,最后添加到Ring Buffer中,将其再映射在网卡可访问的内存DMA区域,最后才真实发送
- 而对于服务器中的EPOLL而言,其只负责socket对应的缓冲区

*详解LT和ET*
- LT(level-trigger) : 水平触发,看的是这个缓冲区,如果之前触发过一次,但是下一次epoll_wait发现对应文件描述符中的缓冲区还有数据,就会立即再次触发一次这个事件
- ET(edge-trigger) : 边沿触发,看的是文件描述符的状态,如果之间触发过一次,但是缓冲区中还有数据,也就是文件描述符的状态没有发生变化,并不会再次触发,适合一次性处理完缓冲区中所有数据的场景

- 对于底层数据包到达通知操作系统的方式也是通过类似于ET的方式触发,DMA-->Ring Buffer(触发硬件中断,暂时屏蔽中断,之后如果还有数据包来,就不会触发硬件中断,开启软中断,恢复屏蔽的中断)-->sk_buff(内核线程ksoftirqd收到软中断之后,就会轮询的处理Ring Buffer中的数据)--> 一层一层拆包 --> socket_buffer

*关于EINPROGRESS*
- 当客户端socket没有设置为非阻塞的情况下,connect()可能会出现EINPROGRESS状态
- 因为太快了,创建文件描述符的背后还要创建缓冲区等一系列数据结构
- 这时候给这个文件描述符添加EPOLLOUT事件,若触发(表明缓冲区建好了),则证明链接成功
- 为了万无一失,在这个写事件对应的回调函数中,使用getsockopt来获取ERROR,再判断一次是否建立成功

**DNS Service**
- cmdid:通常指"Command ID",即命令标识符.在 DNS 服务中,cmdid 用于标识执行的特定命令或操作(添加、删除、修改 DNS 记录等)
- modid:通常指"Module ID",即模块标识符.在 DNS 系统中,modid 可能用于标识特定的模块或组件,这个模块或组件可能是 DNS 服务器的一个部分,或者是与 DNS 相关的其他系统（如监控系统等）中的一个模块
1. DNS Service框架示意图
```
    Agent                   MQ --> thread Loop
         \     Main        /                  \                          data_pointer
    Agent --> Accept() -->  MQ --> thread Loop --> 读锁(RdLock) ------------------------------>  RouterDataMap(A)
         /                 \                  /        |                                                ^
    Agent                   MQ --> thread Loop         |                                                |
                                ^                      |First                                           |
                                |send                  |get                                             |
                                |route                 |need                                            |
                                |to                    |subscirbe                                       |    
                                |subscirbe             |                                                |
                                |                      V                                                |  
                        RouteChange()  <----------- SubscribeList                                       |
                            ^                                                                           |
                            |           Get                        Load                  Update         |
    RouteData() <----> RouteVersion() <----- Backend Thread Loop -------> temp_pointer --------> RouterDataMap(B)
```
2. 网络模块
- 基于Lars_Reactor ---> one loop per thread TCP Servive
- 主线程中的Accept()负责接收链接
- MQ中的thread 负责处理链接的请求.

3. 双map模型
- 使用两个Map来存储路由数据. 
- RouterDataMap ----> key = modid << 32 + cmdid, value = set of ip << 32 + port
- RouterDataMap_A(data_pointer) : 主数据,查询请求在这个Map
- RouterDataMap_B(temp_pointer) : 后台线程周期性重加在路由到此Map,作为最新数据替换掉上一个Map

4. Backend Thread 守护线程
    1. 负责周期性(默认 1s) 检查RouteVersion表的版本号,如果有变化就更新RouteData表的内容
    2. 负责周期性(默认 8s) 重新加载RouteData表内容,重加在RouteData表的方法:上写锁,swap(temp_pointer, data_pointer)浅拷贝只交换地址

5. 主业务
```
                  data_pointer
    1.RouteData ---------------> RouterDataMap_A
    2.temp_pointer ------------> RouterDataMap_B = nullptr;         RdLock
    3.Agent send Query for request modid/cmdid -----> Thread Loop ---------> RouterDataMap_A.result()
    4.if !RouterDataMap_A.result() --> agent ip + port + modid/cmdid --> Backend thread loop1  ---> ClientMap
    5.Backend Thread pre 10s clean RouterDataMap_B ---> load RouteData --> RouterDataMap_B ---> swap();
```
6. Route中关于map数据类型的定义
- 这里的Route并非reactor中的路由分发router,这里的Route是把modid/cmdid与需要管理的远程服务器的serverip/serverport的一条对应关系叫
    一个Route
- 使用Map来存储这些关系,key是modid/cmdid<uint64_t>的一个二进制偏移量处理,value是一个set<uint64_t>集合,因为一个modid/cmdid可能对应多个host主机的ip和port
    

7. 关于MySQL中的mysql_store_result();
- MYSQL_RES *result = mysql_store_result(&_db_conn);
- 将服务端的数据存储在客户端的内存中,由result指针指向的结构体控制
- 注意,对于大型的查询结果集,一次性将所有数据加载到客户端内存可能会导致内存不足或性能问题。在这种情况下，可以考虑使用 mysql_use_result 函数，该函数允许逐行地从服务器端读取数据，而不是一次性将所有数据加载到客户端内存中。

**Reporter Service**
- 负责接收各agent对某modid、cmdid下节点的调用状态的上报。
- agent会把代理的host节点的状态上报给Reporter，Reporter负责存储在Mysql数据库中。

架构图如下:
```
    Agent1                                MQ ---> Thread Loop 
           \ 上报请求              Hash   /                    \ write(Mysql)
    Agent2 ---------> Accept() ---------> MQ ---> Thread Loop -------------> 调度状态
           /                             \                    /
    Agent3                                MQ ---> Thread Loop

```

业务:

- Reporter服务模型采用了single thread TCP服务器 + 线程池处理请求

1. 主线程Reporter负责接收agent请求，并根据请求中携带的modid和cmdid，拼接后进行Hash（一致性hash），分配到某个线程的MQ上 
2. Thread 1~N负责处理请求：把MQ上的请求中的数据同步更新到MySQL数据表

- 由于agent上报给Reporter的信息是携带时间的，且仅作为前台展示方便查看服务的过载情况，故通信仅有请求没有响应

- 于是Reporter服务只要可以高效读取请求即可，后端写数据库的实时性能要求不高。

**配置文件格式**
```
[reactor]
ip = 127.0.0.1
port = 7777
maxConn = 1024
thredNum = 5
```


**一些知识点**

1. 关于函数参数中使用`const char* `还是 `string`的讨论
- 如果函数需要与 C 语言进行交互、对性能要求较高、或者是对字符串的操作比较简单和局限，那么选择 const char* 更合适
- 更关注安全性、方便性和代码的可读性，或者需要进行复杂的字符串操作，那么选择 std::string 更为合适,但string在底层是动态对内存分配,会产生额外开销
- 如果作为map的key,必须使用string,因为key之间的比较是值(const char *的话是指针)比较,string进行了一次封装, string::operator < 判断的是指向的mapped_value

2. 关于SIGNAL
    SIGPIPE: 如果客户端关闭,服务端再次write就会产生
    SIGHUP:如果终端关闭,就会给当前进程发送该信号

3. 注重编程安全
    封装一些marco,见`#include<qc.hpp>`

4. 关于网络相关的一些函数
- 通常关于网络的一些属性都是基于大端存储(更适合人类阅读),但ip地址是一个例外,其以小端字节序存储
- 判断的方法:`char ch[] {'1','2','3','4'}; std::cout << ch;`输出4表小端,输出1表大端
```cpp
    inet_aton(const char* cp, struct in_addr* inp);
        param[in] cp -> 表示要转换为点分十进制表示的IPv4地址
        param[in] inp -> 表示存储转换后的二进制形式的IP地址
        aton -> address to net(考虑字节序问题)
    htons(uint16_t hostshort);
        host to network short -> 主机序列转换为网络序列
    htonl(uint32_t hostlong);
        host to network long 
    ntohl(uint32_t netlong);
        network to host long -> 网络序列转换为主机序列
    ntohs(uint16_t netshort);
```
5. 关于端口复用
- 一般来说,一个端口释放后会等待(TIME_WAIT时间)两分钟之后才能被再次使用.
- 允许多个套接字在同一台主机上同时绑定到相同的IP地址和端口.
- 作用:
        提供相同服务多个实例
        实现快速切换服务
- `SO_REUSEADDR`:   允许在绑定套接字时重用处于TIME_WAIT状态的地址和端口
                    允许同一端口启动同一服务器的多个实例,只要每个实例捆绑
                    一个不同的本地IP地址即可.但对于TCP而言,不可能启动捆
                    绑相同IP地址和相同该端口号的多个服务器.
- `SO_REUSEPORT`: 允许多个套接字绑定到相同IP地址和端口,after Linux3.9

6. 关于客户端连接
- 偷懒方法 nc 127.0.0.1:7777

7. 关于Makefile(已经不用了,CMake更方便也更高效)
```makefile
OBJS = $(addsuffix.o, $(basename $(wildcard *.cc)))
```
- wildcard函数 会返回当前目录下所有以.cc结尾的文件列表
- basename函数 用于去掉文件名中的扩展名(把.cc去掉)
- addsuffix .o函数 用于在每个文件名后面添加后缀. 

8. 解决TCP粘包问题
- 我们要将我们所发送的数据做一个规定,采用TLV的格式
- 本质就是自己实现一个缓冲区,read/recv,write/send是先向缓冲区读/写数据,保证数据一致性
```
    DataLen Id   Data    DataLen Id  Data
    |--head--| |-body-|  |-head---| |-body-| 
```
9. 关于TCP连接
- 设置TCP_NODELAY 禁止做读写缓存,降低小包延迟
```cpp
int op = 1;
setsockopt(_connfd, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));
```

10. Hook 
- 分为外挂式和内侵式
- 主流的都是外挂式
- 一种方式是根据符号表将系统中的系统调用修改
- 另一种是使用函数重载,直接调用重载后的就可以

11. 关于pthread_t 和 tid
- pthread_t:->线程标识符(一个class 里面有tid等信息)
- tid:->线程ID,表示一个线程的唯一标识符在,POSIX下,pthread_t 类型的变量实际上是一个指向线程控制块(Thread Control Block,TCB)的指针,而线程控制块中包含了线程的ID等信息

12. 关于std::function
- 本质上是标准库中的一个模板类,不能在一个匿名联合(anonymous union)的匿名结构体中定义具有构造函数、析构函数、拷贝构造函数的成员,因为编译器无法知道什么时候调用这些成员的析构函数,除非给匿名联合体加上名字并且在里面自己实现构造析构函数

13. 配置文件格式如下
```
[reactor]
ip = 127.0.0.1
port = 7777
maxConn = 1024
thredNum = 5
```
14. 关于sstream
- istringstream ->  将字符串作为输入流
```cpp
std::string str = "123 456 789"
std::istringstream ss(str)
int num1, num2, num3;
iss >> num1 >> num2 >> num3; //123 456 789
```
- ostringstream -> 将数据写入字符串流,可以将数据格式化为字符串
```cpp
std::ostringstream oss;
int num1 = 123, num2 = 456, num3 = 789;
oss << num1 << " " << num2 << " " << num3;
std::string str = oss.str(); // str = 123 456 789
```
- stringstream  -> istringstream 和 ostringstream的组合
```cpp
std::string str = "123 456 789"
std::stringstream ss(str);
```

15. 在string或者vector或者map中
- 使用at比[]更加安全,前者会检查越界报错,后者越界的话会UB(未定义的行为)
- 尤其对于map而言,[]表示的是写入,at()表示查找,因为[]只有非const版本,at()const和非const都有,使用[]如果误写了,会try_emplate生成一个KV,返回0

16. Google Protocal Buffer -> Protobuf 结构化数据存储格式
- 很适合用做数据存储和作为不同应用，不同语言之间相互通信的数据交换格式,只要实现相同的协议格式,即同一proto文件被编译成不同的语言版本,这样不同语言就可以解析其他语言通过protobuf序列化的数据,平台无关、语言无关、可扩展
- 所谓序列化就是把复杂的结构体数据按照一定的规则编码成一个字节切片
- 常用的数据交换格式 json、xml、protobuf(二进制数据格式、需要编码和解码)


17. 关于重复定义
- 在lars_loadbalance_agent遇到了重复定义的问题,解决方法如下:
- 第一种方法将全局变量定义在源文件中,不要放在名字空间中,在其他文件中使用extern包含即可
- 第二种方法使用inline将这个全局变量定义在一个头文件中,使用inline修饰,不要放在名字空间中,其他文件也不需要extern

18. 关于git 
- 由于开发环境是 主机Window + 虚拟机(Ubuntu24.04),所以主机上有梯子能登陆github也没用,你得虚拟机要能登上github
- 解决方法:
- 1.给虚拟机也整上梯子
- 2.安装双系统
- 3.使用代理
- 4.在 /etc/hosts中将github域名对应的ip保存下来,之后输入下面的bash
```bash
git remote -v
git remote set-url origin https://yourusername@yourrepositoryurl.git
```

19. 关于protobuf
- 一般对于Cpp而言,在接收数据包的时候使用ParseFromArray
- 本质上 ParseFromArray 和 ParseFromString 的处理结果是一致的.
- ParseFromString多用于Python,而ParseFromArray多用于C++
- 在message中的reapted属性,如果要一个一个获取,其类型为const.

20. 关于RAII
- 本项目中规定只在创建的时候make_shared一次,后面都用get,裸指针
- 因为一般没人手动delete,是为了防止内存泄漏,并且同时兼顾了内存开销,同时还保留了Cpp指针特色
- 良好的Cpp程序是不能出现 new 和 delete 的
- 相较于std::shared_ptr<T> ptr(new T()) 更加推荐 std::shared_ptr<T> ptr = std::make_shared<T>();
- 后者比前者少一次申请内存

21. 切勿滥用unordered_map
- 在dns_service中,要集成subscribe,如果使用`unordered_map<uint64_t, unordered_set<int>>`话
- 在对容器进行操作之后,这个内存是不保证安全的,有可能会覆盖掉之前的一些变量,因为unordered_map底层实现是哈希,value被映射到哪里是透明的,出现了这个问题说明,标准库的实现还是有问题的,后面对conn->的所有操作都会报段错误
- 改成map和set后问题就可以解决