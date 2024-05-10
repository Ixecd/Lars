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

12. 关于epoll
    底层是红黑树+双向链表
    事件驱动型IO模型 --> 意味着如果对一个文件描述符fd先进行写操作(对应的obuf不为空),之后再epoll_ctl()注册对应的写回调函数会立即执行
                      如果一个文件描述符要read (其有对应的ibuf,如果ibuf不为空),那么之后epoll_ctl()注册的读回调函数会立即执行

13. hook 
    分为外挂式和内侵式,修改代码,本质上根据符号表将系统中的系统调用修改为程序员自己编写的函数(外挂式)
    直接修改内核源码(内侵式)

14. 关于pthread_mutex_t 的初始化
    原型：
        int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr);
        可以通过attr 设置一些锁属性
    锁类型：
        PTHREAD_MUTEX_NORMAL： 这是默认的锁类型，没有死锁检测和错误检测。
        PTHREAD_MUTEX_ERRORCHECK： 这种类型的锁支持错误检测，当同一线程尝试重复加锁时会返回错误。
        PTHREAD_MUTEX_RECURSIVE： 这种类型的锁支持递归加锁，同一线程可以多次加锁，每次加锁后需要相同次数的解锁。
        PTHREAD_MUTEX_DEFAULT： 与 PTHREAD_MUTEX_NORMAL 相同。
        使用场景：
            如果你希望能够在同一线程中多次获取锁而不产生死锁，可以使用 PTHREAD_MUTEX_RECURSIVE。
            如果你希望在调试时能够捕获到重复加锁的错误，可以使用 PTHREAD_MUTEX_ERRORCHECK。
        锁的进程共享属性：
            PTHREAD_PROCESS_PRIVATE： 锁只在创建它的进程内有效，即不跨进程共享。
            PTHREAD_PROCESS_SHARED： 锁可以跨进程共享。
        使用场景：
            如果需要在多个进程间共享锁，可以使用 PTHREAD_PROCESS_SHARED。

    这些特定的锁属性可以通过 pthread_mutexattr_settype()、pthread_mutexattr_setpshared() 等函数进行设置。注意这些函数
    是在调用pthread_mutex_init()之前。
    eg:
        pthread_mutex_t my_mutex;
        pthread_mutexattr_t attr;

        // 初始化属性对象
        pthread_mutexattr_init(&attr);
        // 设置属性为错误检测类型
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
        // 初始化互斥锁并使用属性对象
        pthread_mutex_init(&my_mutex, &attr);

    静态初始化 PTHREAD_MUTEX_INITIALIZER(直接就定死了,属性就是PTHREAD_MUTEX_NORMAL)
    动态初始化 pthread_mutex_t mutex; pthread_mutex_init(&mutex, nullptr);(调用完init就定死了)
    区别：
        时机： 静态初始化是在编译时进行的，而动态初始化是在运行时进行的。
        方式： 静态初始化是通过直接赋值进行的，而动态初始化是通过调用 pthread_mutex_init() 函数进行的。
        适用性： 静态初始化适用于那些在编译时就可以确定初值的情况，而动态初始化适用于在运行时才能确定初值的情况，或者需要动态配置参数的情况。

15. 关于pthread_t 和 tid
    pthread_t：-> 线程标识符(一个class 里面有tid等信息)
        pthread_t 是 POSIX 线程库中用来表示线程标识符的数据类型，通常由线程库创建和管理。它是一个抽象的数据类型，可以用来标识一个线程。
        在 POSIX 线程库中，pthread_t 类型的变量被用来创建、等待、终止等线程操作。
    tid：      -> 线程ID
        tid 通常指的是线程ID，表示一个线程的唯一标识符。在 POSIX 环境下，pthread_t 类型的变量实际上是一个指向线程控制块（Thread Control
        Block，TCB）的指针，而线程控制块中包含了线程的ID等信息。因此，可以通过某种方式来获取线程ID，比如调用 pthread_self() 函数，
        该函数返回当前线程的 pthread_t 类型的标识符。

16. 关于std::function
    本质上是标准库中的一个模板类,不能在一个匿名联合(anonymous union)的匿名结构体中定义具有构造函数、析构函数、拷贝构造函数的成员,因为编译器无法知道什么时候调用这些成员的析构函数.

17. 配置文件格式如下：
    [reactor]
    ip = 127.0.0.1
    port = 7777
    maxConn = 1024
    thredNum = 5

18. 关于sstream
    istringstream ->  将字符串作为输入流
        eg:
            std::string str = "123 456 789"
            std::istringstream ss(str)
            int num1, num2, num3;
            iss >> num1 >> num2 >> num3;
                    123     456     789
    ostringstream -> 将数据写入字符串流,可以将数据格式化为字符串
        eg:
            std::ostringstream oss;
            int num1 = 123, num2 = 456, num3 = 789;
            oss << num1 << " " << num2 << " " << num3;
            std::string str = oss.str(); // str = 123 456 789
    stringstream  -> istringstream 和 ostringstream的组合
        eg:
            std::string str = "123 456 789"
            std::stringstream ss(str);
            可以in 也可以out

19. 关于fstream
    用于简单的读取和写入文件
    std::ifstream 类似于从文件中读取数据,继承自std::istream类
        eg:
            std::ifstream inputFile("input.txt");
            if (!inputFile.is_open()) {
                fprintf(stderr, "input.txt open failed\n");
            }

            std::ofstream outputFile("output.txt");
            if (!outputFile.is_open()) {
                cerr << "output.txt open failed\n";
            }

            // 读取输入文件内容并写入输出文件
            std::string line;
            while (std::getline(inputFile, line)) {
                outputFile << line << std::endl;
            }
    std::ofstream 类似于从文件中写入数据,继承自std::ostream类

20. 在string或者vector中
    使用at比[]更加安全,前者会检查越界报错,后者越界的话会UB(未定义的行为)
    
21. Google Protocal Buffer -> Protobuf 结构化数据存储格式
        很适合用做数据存储和作为不同应用，不同语言之间相互通信的数据交换格式,只要实现相同的协议格式,即同一proto文件被编译成不同的语言版本,
    这样不同语言就可以解析其他语言通过protobuf序列化的数据.
        平台无关、语言无关、可扩展
    所谓序列化就是把复杂的结构体数据按照一定的规则编码成一个字节切片

    A.常用的数据交换格式 json、xml、protobuf(二进制数据格式、需要编码和解码)
