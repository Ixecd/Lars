# Lars C++负载均衡远程调度服务系统
--- Reactor 模型服务器框架 ---
eventLoop/thread Tcp Server Model

1. 关于#include <netinet/in.h> 
    其通常在Unix-Like系统中使用,包含了struct sockaddr_in、AF_INET、PORT_ANY、IPPROTO_TCP、inet_addr()、htonl()、htons()、ntohl()、ntohs()
    类似的头文件还有:
        #include <sys/socket.h> -> soket相关
        #include <arpa/inet.h>  -> inet_addr()、inet_ntoa()
        #include <netdb.h> -> DNS
        #include <netinet/tcp.h> -> TCP
        #include <netinet/ip.h> -> IP

2. 关于函数参数中使用const char* 还是 string的讨论
    如果函数需要与 C 语言进行交互、对性能要求较高、或者是对字符串的操作比较简单和局限，那么选择 const char* 更合适。而如果你更关注安全性、
    方便性和代码的可读性，或者需要进行复杂的字符串操作，那么选择 std::string 更为合适。std::string中自带相关成员函数,能方便得出size()等,
    但string在底层是动态对内存分配,会产生额外开销

3. 关于SIGNAL
    SIGPIPE: 如果客户端关闭,服务端再次write就会产生
    SIGHUP:如果终端关闭,就会给当前进程发送该信号

4. 注重防御性编程
    编写自己的assert qc_assert(),见#include <qc.hpp>

5. 关于网络相关的一些函数
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

6. 关于端口复用
    一般来说,一个端口释放后会等待(TIME_WAIT时间)两分钟之后才能被再次使用.
    允许多个套接字在同一台主机上同时绑定到相同的IP地址和端口.
    作用:
        提供相同服务多个实例
        实现快速切换服务
    SO_REUSEADDR:   允许在绑定套接字时重用处于TIME_WAIT状态的地址和端口
                    允许同一端口启动同一服务器的多个实例,只要每个实例捆绑
                    一个不同的本地IP地址即可.但对于TCP而言,不可能启动捆
                    绑相同IP地址和相同该端口号的多个服务器.
    SO_REUSEPORT: 允许多个套接字绑定到相同IP地址和端口,after Linux3.9

    Q:编写 TCP/SOCK_STREAM 服务程序时，SO_REUSEADDR到底什么意思？
    A:这个套接字选项通知内核，如果端口忙，但TCP状态位于 TIME_WAIT ，可以重
    用端口。如果端口忙，而TCP状态位于其他状态，重用端口时依旧得到一个错误信
    息，指明"地址已经使用中"。如果你的服务程序停止后想立即重启，而新套接字依旧
    使用同一端口，此时SO_REUSEADDR 选项非常有用。必须意识到，此时任何非期望
    数据到达，都可能导致服务程序反应混乱，不过这只是一种可能，事实上很不可能。

7. 关于客户端连接
    偷懒方法 nc 127.0.0.1 7777

8. 关于Makefile
    OBJS = $(addsuffix.o, $(basename $(wildcard *.cc)))
    wildcard函数 会返回当前目录下所有以.cc结尾的文件列表
    basename函数 用于去掉文件名中的扩展名(把.cc去掉)
    addsuffix .o函数 用于在每个文件名后面添加后缀. 

9. 解决TCP粘包问题
    我们要将我们所发送的数据做一个规定,采用TLV的格式
    DataLen Id   Data    DataLen Id  Data
    |--head--| |-body-|  |-head---| |-body-| 

10. 关于TCP连接
    设置TCP_NODELAY 禁止做读写缓存,降低小包延迟
    int op = 1;
    setsockopt(_connfd, IPPROTO_TCP, TCP_NODELAY, &op, sizeof(op));

11. 关于TCP-conn中客户端socket
    当客户端socket没有设置为非阻塞的情况下,connect()可能会出现EINPROGRESS状态,
	
    客户端测试程序时，由于出现很多客户端，经过connect成功后，代码卡在recv系统调用中，后来发现可能是由于socket默认是阻塞模式，所以会令很多客户端链接处于链接却不能传输数据状态。

	后来修改socket为非阻塞模式，但在connect的时候，发现返回值为-1，刚开始以为是connect出现错误，但在服务器上看到了链接是ESTABLISED状态。证明链接是成功的

	但为什么会出现返回值是-1呢？ 经过查询资料，以及看stevens的APUE，也发现有这么一说。

	当connect在非阻塞模式下，会出现返回-1值，错误码是EINPROGRESS，但如何判断connect是联通的呢？stevens书中说明要在connect后，继续判断该socket是否可写？

	若可写，则证明链接成功。

	给epoll立即添加写事件,如果可写说明连接建立成功
