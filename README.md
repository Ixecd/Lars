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
