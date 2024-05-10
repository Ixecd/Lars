--- DnsService ---
0. Agent 对 modid, cmdid的请求并返回modid,cmdid下的所有节点,为agent提供路由服务

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






