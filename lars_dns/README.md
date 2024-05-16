--- DnsService ---
0. Agent 对 modid, cmdid的请求并返回modid,cmdid下的所有节点,为agent提供路由服务
    cmdid:通常指"Command ID",即命令标识符.在 DNS 服务中,cmdid 可能用于标识执行的特定命令或操作。例如，当使用 DNS 管理工具执行某些操作
    (如添加、删除、修改 DNS 记录等)时,这个 cmdid 可能会指示执行的具体命令类型.cmdid 可能是一个数字、一个字符串或其他类型的标识符。
    modid:通常指"Module ID",即模块标识符.在 DNS 系统中,modid 可能用于标识特定的模块或组件.这个模块或组件可能是 DNS 服务器软件的一个部分
    ,或者是与 DNS 相关的其他系统（如监控系统、管理界面等）中的一个模块.modid 可能也是一个数字、一个字符串或其他类型的标识符。

1. 框架
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

2. 网络模块
    基于Lars_Reactor ---> one loop per thread TCP Servive
    
    主线程中的Accept()负责接收链接
    MQ中的thread 负责处理链接的请求.

3. 双map模型
    使用两个Map来存储路由数据. 
        RouterDataMap ----> key = modid << 32 + cmdid, value = set of ip << 32 + port
        RouterDataMap_A(data_pointer) : 主数据,查询请求在这个Map
        RouterDataMap_B(temp_pointer) : 后台线程周期性重加在路由到此Map,作为最新数据替换掉上一个Map

4. Backend Thread 守护线程
    1. 负责周期性(默认 1s) 检查RouteVersion表的版本号,如果有变化就更新RouteData表的内容
    2. 负责周期性(默认 8s) 重新加载RouteData表内容
    重加在RouteData表的方法:上写锁,swap(temp_pointer, data_pointer)浅拷贝只交换地址

5. 主业务
                  data_pointer
    1.RouteData ---------------> RouterDataMap_A
    2.temp_pointer ------------> RouterDataMap_B = nullptr;         RdLock
    3.Agent send Query for request modid/cmdid -----> Thread Loop ---------> RouterDataMap_A.result()
    4.if !RouterDataMap_A.result() --> agent ip + port + modid/cmdid --> Backend thread loop1  ---> ClientMap
    5.Backend Thread pre 10s clean RouterDataMap_B ---> load RouteData --> RouterDataMap_B ---> swap();

6. Route中关于map数据类型的定义
    这里的Route并非reactor中的路由分发router,这里的Route是把modid/cmdid与需要管理的远程服务器的serverip/serverport的一条对应关系叫
    一个Route.
    我们使用Map来存储这些关系.key是modid/cmdid<uint64_t>的一个二进制偏移量处理,value是一个set<uint64_t>集合,因为一个modid/cmdid可能对应多个host主机的ip和port
    
7. 关于cmdid 和 modid
        cmdid (Command ID)
    cmdid 通常用于标识某个服务接口的具体操作。例如，一个服务可能有多个接口，而每个接口又可能有多个操作，这时候就可以用cmdid来区分这些操作。cmdid 是一个整数值，每个操作都有一个唯一的cmdid。
        modid (Module ID)
    modid 通常用于标识某个具体的服务模块。例如，一个系统中可能有多个不同的服务模块，每个模块提供不同的功能。modid 是一个整数值，每个模块都有一个唯一的modid。

8. 关于MySQL中的mysql_store_result();
    MYSQL_RES *result = mysql_store_result(&_db_conn);
    将服务端的数据存储在客户端的内存中,由result指针指向的结构体控制
    
    需要注意的是，对于大型的查询结果集，一次性将所有数据加载到客户端内存可能会导致内存不足或性能问题。在这种情况下，可以考虑使用 mysql_use_result 函数，该函数允许逐行地从服务器端读取数据，而不是一次性将所有数据加载到客户端内存中。






