# myProtocal

### TCP

可靠数据传输：使用超时技术处理丢包问题（超过一定时间未收到接收方的返回值，则重发分组）

报文：

1.  序号（seq），占4个字节，TCP连接中传送的字节流中的每个字节都按顺序编号。
2.  确认号（ack），占4个字节，是期望收到对方下一个报文的第一个数据字节的序号。
3.  同步SYN，在连接建立时用来同步序号。当SYN=1，ACK=0，表明是连接请求报文，若同意连接，则响应报文中应该使SYN=1，ACK=1；

三次握手：

![img](https://imgconvert.csdnimg.cn/aHR0cDovL2ltZy5ibG9nLmNzZG4ubmV0LzIwMTcwNjA1MTEwNDA1NjY2?x-oss-process=image/format,png)

1.   服务器建立TCB，进入监听状态
2.   客户端创建TCB，发送链接请求报文（第一次握手），不能携带数据，且需要消耗一个序号
3.   服务器接收请求报文，返回确认报文（第二次握手）
4.   客户端发送确认报文（第三次握手）
     1.   主要防止已经失效的连接请求报文突然又传送到了服务器，从而产生错误。
     2.   若客户端发送的第一个请求连接在关闭连接之后传达到服务器，服务器会返回确认报文，但由于客户端不会再发送确认报文，所以不会再次建立连接导致资源浪费

四次挥手：

![image-20230312152708344](C:\Users\24022\AppData\Roaming\Typora\typora-user-images\image-20230312152708344.png)

1.   客户端发送连接释放报文，并停止发送数据（**第一次挥手**）
2.   服务器发送确认报文（**第二次挥手**），进入关闭等待的状态（客户端需要接收服务器发送的数据）
3.   客户端接收确认报文，等待服务器发送连接释放报文
4.   服务器发送链接释放报文（**第三次挥手**），等待客户端的最终确认
5.   客户端发送确认请求（**第四次挥手**），再经过2MSL（最长报文段寿命）的时间后，当客户端撤销TCB之后才能关闭
     1.   等待2MSL：若服务器没有接收到第四次挥手发送的报文，则会重新发送第三次挥手时发送的报文，而客户端需要等待一段时间，在这段时间如果没有接收到服务器发送的报文，则说明服务器收到了第四次挥手的报文，会进行关闭
     2.   2MSL：第四次挥手的报文存活时间+服务器重传第三次挥手的报文的存活时间 <= 2MSL
6.   服务器接收确认报文后关闭，撤销TCB

### Socket编程

#### Socket creation

`int sockfd = socket(domain, type, protocol)`

-   **sockfd:** socket descriptor, an integer (like a file-handle)
-   **domain:** integer, specifies communication domain. 
    -   AF_INET————IPV4, AF_INET6————IPV6
-   **type:** communication type
    -   SOCK_STREAM: TCP, SOCK_DGRAM: UDP
-   **protocol:** 0, Protocol value for Internet Protocol(IP), appears on protocol field in the IP header of a packet.

`int bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)`

`int listen(int sockfd, int backlog)`

`int new_socket = accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)`

`int connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)`

#### 参数转换

`inet_addr()`，`htons()`，将给定的数字或字符串转换为端口所需的格式

`inet_ntoa()`，`ntohs()`，将端口的格式转化为普通格式（可以输出、操作）
