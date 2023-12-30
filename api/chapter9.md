# 9.1 select系统调用

I/O复用允许服务器同时监听多个文件描述符，对提高服务器的性能至关重要，以下场景需要使用：

- 客户端程序需要同时处理多个socket（非阻塞connect）；
- 客户端程序要同时处理用户输入和网络连接（聊天室程序）；
- TCP服务器同时处理监听socket和连接socket；
- 服务器同时处理TCP请求和UDP请求（回射服务器）；
- 服务器同时监听多个端口或提供多种服务（xinetd服务器）；

## 9.1.1 select api

select系统调用用来监听一段时间内，用户感兴趣的socket上的可读、可写、异常事件。

`select` 是一种用于实现多路复用的系统调用或库函数。它允许程序监视一组文件描述符（通常是套接字），并在其中任何一个文件描述符准备好进行 I/O 操作时通知程序。这种机制允许在一个单独的线程或进程中处理多个 I/O 操作，从而提高程序的效率。

```
#include <sys/select.h>

int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
```

- `nfds` 是待监视的文件描述符的最大值加 1。（文件描述符从0开始计数的）
- `readfds`、`writefds`、`exceptfds` 分别是可读、可写、异常的文件描述符集合。
- `timeout` 是一个结构体，表示 `select` 的超时时间。如果为 `NULL`，表示阻塞直到有文件描述符就绪。

`select` 的使用方式是在调用它后，它会检查所监视的文件描述符集合的状态，然后返回就绪的文件描述符数目。程序可以通过检查每个文件描述符在集合中的状态来确定哪些文件描述符是可用的。

`fd_set` 是一个用于表示文件描述符集合的数据结构，通常用于多路复用函数（如 `select`、`pselect`、`FD_SET` 等）。

在 POSIX 系统中，`fd_set` 通常被定义为一个位图（bitmap），用于表示一组文件描述符的状态。它的实现可能因系统而异，但通常是一个数组，数组中的每个元素都对应一个文件描述符。

基本的操作包括向 `fd_set` 中添加和删除文件描述符，以及测试某个文件描述符是否在集合中。这些操作通过宏来完成，主要有：

- `FD_ZERO(fd_set *fdset)`：将 `fd_set` 清零，即将其所有位都设为 0。
- `FD_SET(int fd, fd_set *fdset)`：将文件描述符 `fd` 加入到 `fd_set` 中。
- `FD_CLR(int fd, fd_set *fdset)`：将文件描述符 `fd` 从 `fd_set` 中移除。
- `FD_ISSET(int fd, fd_set *fdset)`：测试文件描述符 `fd` 是否在 `fd_set` 中。

`fd_set` 的大小通常是通过宏 `FD_SETSIZE` 来定义的，表示最大的文件描述符值。在使用 `fd_set` 时，需要保证所使用的文件描述符不超过这个范围。

```
#include <typesizes.h>

#define _FD_SETSIZE 1024
#include <sys/select.h>

typedef long int __fd_mask;

// 数组中的元素是__fd_mask，得到__fd_mask的字节数后*8得到位数
#undef __NFDBITS
#define __NFDBITS (8 * (int)sizeof(__fd_mask))

typedef struct {
#ifdef _USE_XOPEN
	// 求需要多少个数组元素可以包含1024位，并开辟数组
    __fd_mask fds_bits[_FD_SETSIZE / __NFDBITS];
#define __FDS_BITS(set) ((set)->fds_bits)
#else
    __fd_mask __fds_bits[_FD_SETSIZE / __NFDBITS];
#define __FDS_BITS(set) ((set)->__fds_bits)
#endif
} fd_set;
```

根据上面的代码可以看到，`fd_set`每一位对应一个文件描述符，最大可以同时监听的文件描述符数量由`_FD_SETSIZE`来决定，因此限制了同时处理文件描述符的数量。

```
struct timeval {
	long tv_sec;	// 秒数
	long tv_usec;	// 微秒
};
```

`select`返回值：

- 成功返回：
  - 就绪事件不为0，返回就绪事件的总数；
  - 就绪事件为0，返回0；
- 返回失败：返回-1，并设置errno；
- 被信号中断：立即返回-1，设置errno值为EINTR；

`select`的问题是：

- 位图默认是1024，最大可同时监听数量有限；
- 每次开始前都要将位图清零；
- 需要将位图在内核空间和用户空间复制；
- 需要频繁遍历访问位图。

### 9.1.2 文件描述符就绪条件

**socket可读就绪：**

- socket内核缓冲区的字节数大于等于缓冲区的低水位标记：`SO_RCVLOWAT`，此时可以无阻塞的读取数据，读操作返回>0的值；
- socket对方关闭连接，此时读操作返回0；
- 监听socket上有新的连接请求；
- socket上有未处理的错误，此时可以用`getsockopt`读取并清除该错误；

**socket可写就绪：**

- socket内核发送缓冲区中的可用字节数大于等于低水位标记：`SO_SNDLOWAT`，此时可以无阻塞的写数据，写操作返回>0的值；
- socket的写操作被关闭，对写操作被关闭的socket执行写操作将触发一个`SIGPIPE`信号；
- socket使用非阻塞`connect`连接成功或失败后；
- socket上有未处理的错误，此时使用`getsockopt`读取并清除该错误；

### 9.1.3 处理带外数据

`select`无论接收到普通数据还是带外数据都会返回，但socket处于不同的状态，前者处于就绪状态，后者属于异常状态。因此需要针对不同的情况进行不同的处理。

# 9.2 poll系统调用

```
#include <poll.h>

int poll(struct pollfd *fds, nfds_t nfds, int timeout);
```

- `fds` 是一个指向 `struct pollfd` 结构体数组的指针，每个结构体描述一个待监视的文件描述符以及关心的事件。
- `nfds` 表示数组中元素的数量。
- `timeout` 表示超时时间，以毫秒为单位。如果设置为负值，`poll` 将一直等待直到有事件发生。如果设置为 0，`poll` 将立即返回而不等待。

```
struct pollfd {
    int   fd;         /* 文件描述符 */
    short events;     /* 等待的事件 */
    short revents;    /* 实际发生的事件 */
};
```

- `fd` 是文件描述符。

- `event`

   是等待的事件，可以是以下几个常量的按位或：

  - `POLLIN`：有数据可读。
  - `POLLOUT`：写不会导致阻塞。
  - `POLLERR`：发生错误。
  - `POLLHUP`：对方关闭连接。
  - `POLLNVAL`：文件描述符不是一个打开的文件。

`revents` 字段在 `poll` 函数返回时表示实际发生的事件。

`poll` 系统调用的使用方式类似于 `select`，但相对更灵活，可以同时监视更多的文件描述符。在高性能网络编程中，`poll` 通常被用来实现多路复用。

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/9_1.png)

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/9_2.png)

`poll`在`select`基础上解决了最大监听数量受限的问题。

# 9.3 epoll系列系统调用

## 9.3.1 内核事件表

`epoll`是Linux特有的的I/O复用函数，实现上和`select`、`poll`有所不同。首先，`epoll`使用一组函数来完成任务，而不是单个函数；其次，`epoll`把用户关心的文件描述符上的事件放到内核的一个事件表中，无需在每次调用时都要重复传入文件描述符或者事件集。但是，`epoll`需要额外的文件描述符来唯一的标识这个内核事件表。

`epoll_create` 函数

```
#include <sys/epoll.h>

int epoll_create(int size);
```

- 用途：创建一个 epoll 实例。
- 参数：
  - `size`：表示 epoll 实例的大小，这个参数在 Linux 2.6.8 之后已经被忽略，但是仍然需要传递一个大于 0 的值。

返回值：

- 成功：返回一个文件描述符，用于引用创建的 epoll 实例。
- 失败：返回 -1，并设置 `errno` 来指示错误原因。

`epoll_ctl` 函数

```
#include <sys/epoll.h>

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
```

- 用途：对 epoll 实例进行控制操作，比如注册、修改、删除事件。
- 参数：
  - `epfd`：epoll 实例的文件描述符。
  - `op`：操作类型，可以是 `EPOLL_CTL_ADD`、`EPOLL_CTL_MOD` 或 `EPOLL_CTL_DEL`。
  - `fd`：待操作的文件描述符。
  - `event`：指向 `struct epoll_event` 结构体的指针，描述待注册的事件。

返回值：

- 成功：返回 0。
- 失败：返回 -1，并设置 `errno` 来指示错误原因。

`struct epoll_event` 结构体

```
struct epoll_event {
    uint32_t events;   /* epoll 事件 */
    epoll_data_t data;  /* 用户数据变量，可以是指针或者文件描述符 */
};

typedef union epoll_data {
    void *ptr;
    int fd;
    uint32_t u32;
    uint64_t u64;
} epoll_data_t;
```

- `events`：表示关心的事件，可以是以下几个常量的按位或：
  - `EPOLLIN`：有数据可读。
  - `EPOLLOUT`：写不会导致阻塞。
  - `EPOLLERR`：发生错误。
  - `EPOLLHUP`：对方关闭连接。
  - `EPOLLRDHUP`：对方关闭连接或者半关闭的状态。
  - `EPOLLONESHOT`：注册的文件描述符只能触发一次事件。
  - `EPOLLET`：设置为边缘触发模式。
- `ptr`：指定与fd相关的用户数据；
- `data`：用户数据变量，可以是指针或者文件描述符。在注册事件时，通过这个字段传递用户需要的数据。

**注意：**由于`epoll_data`是一个联合体，因此无法同时访问`fd`和`ptr`，如果要将文件描述符和用户数据关联起来以实现快速访问，只能使用其他手段，比如放弃使用`fd`成员，在`ptr`成员指向的用户数据中包含`fd`。

## 9.3.2 epoll_wait函数

`epoll_wait` 函数是 Linux 中用于等待事件的系统调用。它可以用于等待一个 epoll 实例上的事件，并且可以设置一个超时时间。

**函数原型**

```
#include <sys/epoll.h>

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout);
```

**参数**

- `epfd`：epoll 实例的文件描述符。
- `events`：用于存放事件的数组，是一个指向 `struct epoll_event` 结构体数组的指针。
- `maxevents`：`events` 数组的大小，表示最多等待多少个事件。
- `timeout`：等待的超时时间，单位为毫秒。如果为负数，表示永远等待；如果为零，表示立即返回；如果大于零，表示等待指定的毫秒数。

**返回值**

- 成功：返回就绪的事件数量。
- 失败：返回 -1，并设置 `errno` 来指示错误原因。

函数成功返回就绪事件的过程：在`epfd`中将所有就绪的文件描述符复制到`events`数组中，只输出已经就绪的事件，而不用既存放就绪事件还要存放用户注册事件，极大的提高了索引就绪文件的速度。

`poll`和`epoll`处理就绪事件的不同：

```
int ret = poll(fds, MAX_EVENT_NUMBER, -1);
for(int i = 0; i < MAX_EVENT_NUMBER; ++i) {
	if(fds[i].revents & POLLIN) {
		int sockfd = fds[i];
		// 处理sockfd
	}
}

int ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
for(int i = 0; i < ret; ++i) {
	int sockfd = epollfd[i].data.fd;
	// 处理socket
}
```

## 9.3.3 IT和ET模式

`epoll`对文件描述符的操作方式有两种，LT-LEVEL TRIGGER，也就是电平触发；ET-EDGE-TRIGGER，也就是边缘触发。其中LT是`epoll`默认的触发模式。

- LT
  - 此模式相当于效率较高的poll；
  - 当`epoll_wait`发现有事件发生并返回给应用程序时，应用程序可以不立即处理，等到下一次调用`epoll_wait`时依然会触发事件并返回给应用程序，一直到该事件被处理。
- ET
  - 此模式是高效率的`epoll`；
  - 当`epoll_wait`发现有事件发生并返回给应用程序时，应用程序必须马上处理，因为后续的`epoll_wait`调用不再向程序通知这一事件；

## 9.3.4 epolloneshot事件

即使使用了ET模式，socket也有可能被多次触发，这就会导致一种情况：当前已经有工作线程处理触发的事件，此时又来了新的数据，然后唤醒另一个工作线程来处理新的数据，这就出现了两个工作线程同时处理一个`socket`。这当然不是我们所期望的，我们期望的是任一时刻同一个`socket`只被一个工作线程处理，这一点可以通过`EPOLLONESHOT`来完成。

注册了`EPOLLONESHOT`的`socket`，操作系统最多出发其上注册的一个可读、可写、异常事件，并且只触发一次，从而保证同一时刻只有一个工作线程处理`socket`，但是返过来，当一个工作线程处理完后，需要重置这个`EPOLLONESHOT`，从而保证下一次可读时，能够有工作线程处理其`EPOLLIN`事件，让其他线程能够处理这个`socket`。

# 9.4 三组I/O复用函数的比较

前面的三组系统调用，都可以同时监听多个文件描述符，将等待由timeout指定的超时时间，直到一个或多个描述符发生事件，返回就绪的文件描述符数量。现在从事件集、最大支持文件描述符、工作模式和具体实现来进一步比较异同：

**`**select`：****

- `fd_set`是一个集合，没有和具体事件绑定，因此需要提供三个这种类型的参数来区分是输入、输出和异常情况，一方面不好处理更多的事件类型，另一方面由于内核对`fd_set`的修改，下一次使用`select`系统调用时必须重置这三个参数；
- 但是由于每次调用返回整个用户注册的事件集合（包括就绪和未就绪），应用程序索引就绪文件描述符的时间复杂度为O(n)；
- 支持最多监听的文件描述符通常是1024，可以修改但可能导致不可预测的结果。

**`poll`：**

- 文件描述符和事件定义在一起，任何事件被统一处理，编程接口更简洁；
- 内核每次修改的是`revents`成员而不是`events`，因此下次调用poll时不需要重置`pollfd`；
- 最多监听的文件描述符可以达到系统最大文件描述符65535；
- 但是由于每次调用返回整个用户注册的事件集合（包括就绪和未就绪），应用程序索引就绪文件描述符的时间复杂度为O(n)。

**`epoll`：**

- 在内核中维护一个事件表，提供系统调用`epoll_ctl`来控制向其中添加、删除、修改事件，每次调用`epoll_wait`直接从内核事件表中取得用户注册的事件，无需反复的从用户空间读入这些数据；
- 最多监听的文件描述符可以达到系统最大文件描述符65535；
- `epoll_wait`仅返回就绪事件，使得应用程序索引就绪文件描述符的时间复杂度为O(n)。

`select`和`poll`都工作在低效的LT模式，而`epoll`可以工作在高效的ET模式，并且`epoll`支持`EPOLLONESHOT`，可进一步减小可读、可写、异常事件触发次数。

从原理上来说：`select`和`poll`采用的都是轮询方式，每次调用都要扫描整个注册文件描述符集合，将就绪的文件描述符返回给用户程序，因此检测就绪事件算法的时间复杂度是O(n)。`epoll_wait`则不同，采用回调方式，讷河检测到就绪的文件描述符，触发回调函数，回调函数将文件描述符上对应的事件插入内核就绪事件队列，内核最后在适当实际将就绪事件队列中的内容拷贝到用户空间，其算法时间复杂度是O(1)。

但是要注意：当活动连接比较多的时候，`epoll_wait`的效率未必比`select`和`poll`高，因为此时回调函数被触发的过于频繁，因此`epoll_wait`适用于连接数量多，但活动连接少的情况。

1. **`epoll_wait` 的优势：** `epoll` 主要在处理大量连接中的少量活跃连接时表现优异。`epoll` 使用事件通知机制，只在状态变化时通知应用程序，避免了轮询所有连接的开销，因此在连接数量较大但活动连接较少的情况下，`epoll` 效率更高。
2. **回调函数触发频率：** 当活动连接较多时，`epoll` 可能会导致回调函数的频繁触发，因为每个活动事件都会导致回调。这可能会引入一些开销，因为频繁调用回调函数可能会导致系统切换上下文的开销。
3. **`select` 和 `poll` 的适用场景：** `select` 和 `poll` 在处理大量连接的场景中，可能会比 `epoll` 更高效，因为它们不使用事件通知机制，而是在调用时遍历所有连接并检查状态。当连接数量较大但活动连接较多时，这种轮询的方式可能更合适。

总的来说，选择使用 `epoll`、`select` 还是 `poll` 取决于具体的应用场景和性能需求。在连接数量多但活动连接较少的情况下，`epoll` 是一个高效的选择。在其他情况下，`select` 和 `poll` 也可能是合适的。

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/9_3.png)

# 9.5 I/O复用的高级应用一：非阻塞connect

对非阻塞I/O调用`connect`，而连接没有立即建立好，这种情况下可以调用`select`或者`poll`来监听这个连接失败的`socket`上的读写事件，当`select`或者`poll`返回后，利用`getsockopt`来获取并消除该错误码，如果错误码是0则标识连接已建立，否则连接失败。因此，我们可以同时发起多个连接并一起等待。

非阻塞`connect`的返回值有下面几种情况：

- 返回结果0表示连接建立成功；
- 返回结果小于0且`errno`不是`EINPROGRESS`，表示连接出错；
- 返回结果小于0且`errno`等于`EINPROGRESS`，表示正在建立连接，设置等待时间并使用select或者poll监听sockfd是否可读可写（以select举例））：
  - 如果select返回结果小于等于0，表示超时时间内未能建立连接或者出错，通知应用程序并返回；
  - 如果select返回结果大于0，表示检测到socket可读或可写，通过getsockopt来判断：
    - 如果错误值为0，则连接建立成功；
    - 如果错误值不为0，则连接失败。

socket可写但不可读：连接成功建立，写缓冲区空闲，因此可写；

socket可读可写且通过getsockopt检测socket没有错误则连接成功；（socket出错，socket存在未决错误，socket设置可读可写状态；当socket连接成功，socket也变成可读可写状态，因此需要通过getsockopt检测来进一步区分。）

遗憾的是，上述方法存在移植性问题：

- 非阻塞`socket`可能导致`connect`始终失败；
- `select`对处于`EINPROGRESS`状态下的`socket`可能不起作用；
- 对于出错的`socket`，`getsockopt`在某些系统上返回-1（Linux），而在有些系统返回0（UNIX）。

# 9.6 I/O复用的高级应用二：聊天室程序

本节以poll为例，实现一个简单的聊天室程序，以阐述如何使用I/O复用技术同时处理网络连接和用户输入。允许所有用户同时在线聊天，分为客户端和服务器两个部分。客户端有两个功能，一是从标准输入终端读入用户数据，并发送给服务器；二是从服务器读取数据并显式在终端。服务器的功能是接收数据，并把数据发送到每个登录到该服务器上的客户端。

## 9.6.1 客户端

客户端使用poll同时监听用户输入和网络连接，利用splice函数将用户输入内容直接定向到网络连接上以发送，从而实现零拷贝，提高程序执行效率。

此处代码见`chapter9`。

## 9.6.2 服务端

此处代码见`chapter9`。

# 9.7 I/O复用的高级应用三：同时处理TCP和UDP

目前所有实现的代码都是一个服务器监听一个端口，但实际上有些服务器可以同时监听多个端口。从bing系统调用来看，一个socket只能绑定一个socket地址，因此如果要监听多个端口就需要创建多个socket，并绑定不同的端口号，这样服务器就需要同时管理多个监听文件描述符，I/O复用技术有了用武之地。

此外，即使是同一个端口，处理TCP和UDP也需要建立两个socket分别进行处理（绑定到同一个端口上）。

在测试功能时，对于TCP的测试可以使用telnet，对于UDP的测试可以使用net cat，

```
echo -n "message" | nc -u -w1 ip port
注释：向指定IP和端口发送一个UDP报文，报文的内容是message
-n：取消结尾的换行符；
nc：netcat命令；
-u：发送UDP报文（-t是TCP）；
-w1：等待时间为1秒；
```

# 9.8 超级服务xinetd

超级服务管理多个自服务，即监听多个端口。

## 9.8.1 xinetd配置文件

超级服务的配置文件包含两部分：主配置文件：`/etc/xinetd.conf`和`/etc/xinetd.d`目录下的所有子配置文件。主配置文件包含通用选项，这些选项被子配置文件继承，子配置文件可以覆盖这些选项，类似于C++的继承机制。每一个子配置文件用于设置一个子服务的配置。以`telnet`为例：

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/9_4.png)

## 9.8.2 xinetd工作流程

`xinetd`管理的子服务有两种，第一种是标准服务，比如`discard`、`echo`、`daytime`，`xinetd`服务器在内部直接处理这些服务；还有一种需要调用外部服务程序来处理，`xinetd`通过调用`fork`、`exec`函数加载这些服务程序，比如`ftp`、`telnet`。

以`telnet`为例，`xinetd`创建新的进程执行`telnet`服务，关闭`telnet`的标准输入、标准输出、标准错误，并将`telnet`服务器程序将网络连接上的输入当作标准输入；将标准输出定向到同一个网络连接上。同时`xinetd`一直监听`telnet`连接请求，因此`in.telnetd`子进程只处理连接`socket`而不处理监听`socket`。

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/9_5.png)