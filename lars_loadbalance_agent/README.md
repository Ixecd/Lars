# Part 30 负载均衡模块基础实现

1. 基础

    每个模块modid/cmdid下由若干节点,节点的集合称为此模块的路由,对于每个节点都有两种状态:
    idle : 节点可用,可作为API请求的节点使用
    overload : 节点暂时不可用

    在请求节点时,有几个关键属性
    虚拟成功次数 vsucc : API汇报节点调用的结果
    虚拟失败次数 verr  :
    连接成功次数 contin_succ :
    连接失败次数 contin_err  :

    这四个字段,在节点状态改变的时候会导致 idle <-----> overload 之间的切换

2. 调度方式(见Lars下的README.md文档 22)

    下面是LoadBalance Algorithm 
                                        选取队头,并重新放入队尾
      ----------------<----------------- -----> 节点1 节点2 节点3 ... 空闲队列   
      |             return            Y |
      | gethost                         |
    API --> modID/cmdID --> 未超过probe -
      |                                 |
      |             return            N |
      ----------------<----------------- -----> 节点4 节点5 节点6 ... 过载队列
                                        给过载节点一个机会,选取队头,并重新放到队尾


    这里每个节点就是一个Host主机信息,就是我们需要被管理的主机信息.
    API相当于Agent模块的客户端,也就是业务端调用的请求主机接口.API发送GetHost,发送给API的server端,AgentServer使用UDPserver处理的API网络
    请求,将其发送给一个负载均衡算法.

    当API对某模块发起节点获取时:
        Load Balance从空闲队列中拿出队列的头节点,作为返回节点,之后将其又重新放到队列的尾部.
        关于probe机制: 如果此模块过载队列非空,则每次经过probe_num次节点获取后(默认10),给过载队列中的节点一个机会,让API试探性调用一下.
        如果空闲队列为空,也会给机会经过probe_num次获取后,返回过载错误.

    调度就是:从空闲队列轮流选择节点,同时利用probe机制,给过载队列中的节点一个机会.

3. host_info与Load Balance初始化

    host_info:表示一个host主机信息
    load_balance:针对一组modid/cmdid的负载均衡模块
    route_lb:和udp_server数量保持一致,每个route_lb负责管理多个load_balance

4. 关于protobuf
    一般对于Cpp而言,在接收数据包的时候使用ParseFromArray
    本质上 ParseFromArray 和 ParseFromString 的处理结果是一致的.
    ParseFromString多用于Python,而ParseFromArray多用于C++

    在message中的reapted属性,如果要一个一个获取,其类型为const.

    