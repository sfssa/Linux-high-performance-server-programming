# 12.1 I/O框架库概述

I/O框架库以库函数的形式，封装了较为底层的系统调用，给应用程序提供了易于使用的接口。通常比程序员自己实现的功能的函数更合理、更高效、更健壮。因为经受住了真实网络环境下的高压测试以及时间的考验。基于Reactor模式的框架库包含下面四个组件：

- 句柄-Handle：句柄是I/O事件的符号代表，可以通过句柄找到事件源，包括I/O事件、定时事件、信号，信号的句柄就是信号值；I/O事件对应的句柄是文件描述符。
- 事件多路分发器-EventDemultiplexer：负责事件循环监听，注册事件、删除事件。

![image-20240108104436922](C:\Users\16645\Documents\atpdxy\functions\static\12_1.png)

- 事件处理器和具体事件处理器：通常包含事件源的处理函数。事件处理器提供接口，一般被声明为虚函数，具体的处理函数在具体事件处理器实现，方便扩展。
- Reactor-执行事件循环、调用事件多路分发器中的方法来注册或删除事件。

![image-20240108104449315](C:\Users\16645\Documents\atpdxy\functions\static\12_2.png)

# 12.2 libevent源码分析

libevent库需要手动安装，提供下面两种方法，第一种比较简单：

```
$ sudo apt-get install libevent-dev
```

第二种需要手动下载源文件并编译安装，官网：https://libevent.org/

下载后缀名为`tar.gz`的文件，通过下面命令进行解压：

```undefined
tar -zxvf libevent-2.1.11-stable.tar.gz 
```

配置安装路径：

```
cd libevent-2.1.11-stable
./configure
```

编译安装：

```
make
make install
```

测试安装是否成功：

```bash
ls -al /usr/local/lib | grep libevent
```

## 12.2.1 一个实例

1. **`struct event_base`：**

   - `struct event_base` 是 Libevent 中的事件基础结构体。它代表一个事件循环的实例，用于管理事件的分发和调度。
   - 事件基础结构维护了事件处理的上下文，包括已注册的事件、活跃的事件、定时器、信号等。
   - 使用 `event_base_new` 函数可以创建一个新的事件基础结构体。通常，程序在处理事件之前需要先创建一个事件基础结构体。

2. **`struct timeval`：**

   - `struct timeval` 是一个表示时间间隔的结构体，它包含两个成员：`tv_sec` 表示秒数，`tv_usec` 表示微秒数（1秒 = 1,000,000 微秒）。
   - 用于表示一段时间的延迟或超时，例如在 `event_base_loopexit` 函数中，用于指定在多长时间后退出事件循环。

3. **`event_base_loopexit` 函数：**

   - ```
     event_base_loopexit
     ```

      函数用于通知事件循环退出。函数原型如下：

     ```
     int event_base_loopexit(struct event_base *base, const struct timeval *tv);
     ```

   - `base` 是事件循环的基础结构体，`tv` 是一个 `struct timeval` 结构体，表示延迟多久后退出。如果 `tv` 为 NULL，表示立即退出而不等待。

   - 函数返回 0 表示成功，-1 表示失败。

   - 通过调用该函数，程序可以优雅地通知事件循环退出，而不是强行中断。这对于确保在退出时执行清理工作或等待一段时间后再退出是很有用的。在示例中，`event_base_loopexit(base, &delay);` 表示在两秒后退出事件循环。

4. **`event_init` 函数：**

   - `event_init` 函数用于初始化 Libevent 库。在使用其他 Libevent 函数之前，通常需要调用此函数初始化整个事件库。

   - 函数原型如下：

     ```
     struct event_base *event_init(void);
     ```

   - 返回一个指向 `struct event_base` 的指针，该结构体代表一个事件循环的实例。

5. **`evsignal_new` 函数：**

   - `evsignal_new` 用于创建一个信号事件。信号事件允许程序响应系统信号。

   - 函数原型如下：

     ```
     struct event *evsignal_new(struct event_base *base, int signum, void (*cb)(evutil_socket_t, short, void *), void *arg);
     ```

   - `base` 是事件循环的基础结构体，`signum` 是信号的编号，`cb` 是事件发生时的回调函数，`arg` 是传递给回调函数的参数。

   - 返回一个指向 `struct event` 的指针，代表创建的信号事件。

6. **`evtimer_new` 函数：**

   - `evtimer_new` 用于创建一个定时器事件，定时器事件在指定的时间间隔后触发。

   - 函数原型如下：

     ```
     struct event *evtimer_new(struct event_base *base, void (*cb)(evutil_socket_t, short, void *), void *arg);
     ```

   - `base` 是事件循环的基础结构体，`cb` 是定时器事件触发时的回调函数，`arg` 是传递给回调函数的参数。

   - 返回一个指向 `struct event` 的指针，代表创建的定时器事件。

7. **`event_add` 函数：**

   - `event_add` 函数用于将事件添加到事件循环中，使其生效。

   - 函数原型如下：

     ```
     int event_add(struct event *ev, const struct timeval *tv);
     ```

   - `ev` 是事件结构体指针，`tv` 是一个指定事件何时生效的时间间隔。如果为 `NULL`，表示立即生效。

   - 返回 0 表示成功，-1 表示失败。

8. **`event_base_dispatch` 函数：**

   - `event_base_dispatch` 用于启动事件循环，开始处理事件。

   - 函数原型如下：

     ```
     int event_base_dispatch(struct event_base *base);
     ```

   - `base` 是事件循环的基础结构体。

   - 返回 0 表示正常退出，-1 表示出错。

9. **`event_free` 函数：**

   - `event_free` 用于释放通过 `evsignal_new` 或 `evtimer_new` 创建的事件结构体。

   - 函数原型如下：

     ```
     void event_free(struct event *ev);
     ```

   - `ev` 是要释放的事件结构体指针。

10. **`event_base_free` 函数：**

    - `event_base_free` 用于释放通过 `event_init` 创建的事件循环结构体。

    - 函数原型如下：

      ```
      void event_base_free(struct event_base *base);
      ```

    - `base` 是要释放的事件循环结构体指针。

编译源代码执行时不要忘记附加依赖库：

```
g++ code12_1.c -o code12_1 -levent
```

代码主要逻辑如下：

1. 创建event_base实例，相当于创建一个Reactor实例；
2. 创建具体事件的处理器，设置从属的Reactor实例，evsignal_new和evtimer_new用来创建信号事件处理器和定时事件处理器，统一入口是event_new。

```
#define evsignal_new(b, x, cb, arg)	\
	event_new((b), (x), EV_SIGNAL|EV_PERSIST, (cb), (arg))
#define evtimer_new(b, cb, arg) event_new((cb), -1, 0, (cb), (arg))

struct event* event_new(struct event_base* base, evutil_socket_t fd, short events,
	void(*cb)(evutil_socket_t, short, void*), void* arg)
```

参数详解：

- base：新创建的事件处理器从属的Reactor；
- fd：与该事件处理器关联的句柄，I/O事件处理器，传递文件描述符；信号事件处理器，传递信号值；定时事件处理器，传递-1；
- events：指定事件类型，可选值在`include/event2/event.h`

```
#define EV_TIMEOUT 0x01			// 定时事件
#define EV_READ    0x02			// 可读事件
#define EV_WRITE   0x04			// 可写事件
#define EV_SIGNAL  0x08			// 信号事件
#define EV_PERSIST 0x10			// 永久事件，注册后自动调用event_add函数
#define EV_ET	   0x20			// 边沿触发模式
```

- cb：指定目标事件对应的回调函数；
- arg：传递给回调函数的参数；
- event_new：返回一个event类型的对象，也就是Libevent的事件处理器

1. **事件（Event）：**
   - 事件是程序执行过程中发生的某种事情，它可以是用户输入、系统信号、定时器超时等。事件通常由外部引起，程序需要对这些事件做出响应。
2. **事件处理器（Event Handler）：**
   - 事件处理器是一段代码或函数，用于处理特定类型的事件。当某个事件发生时，相应的事件处理器会被调用来执行相应的逻辑。
3. **事件多路分发器（Event Demultiplexer）：**
   - 事件多路分发器负责等待多个事件的发生，并根据事件的类型将事件分发到相应的事件处理器。它通常是一个事件循环中的一部分，负责监视多个事件源，例如文件描述符、信号等，并将事件分派给对应的事件处理器。
4. **事件队列（Event Queue）：**
   - 事件队列是一个存储待处理事件的数据结构，通常是一个队列。当事件发生时，相关信息被放入事件队列中等待处理。事件处理器从事件队列中取出事件并执行相应的逻辑。
5. **事件循环（Event Loop）：**
   - 事件循环是一个不断运行的程序主循环，负责检查事件队列中是否有待处理的事件。如果有，它会调用相应的事件处理器来处理事件。事件循环不断地执行这个过程，使得程序可以异步地响应事件。

这些概念之间的关系如下：

- **事件** 是引发程序响应的外部发生的事情。
- **事件处理器** 是对特定类型事件的响应逻辑。
- **事件多路分发器** 监视多个事件源，将事件分发给相应的处理器。
- **事件队列** 存储待处理的事件，保证它们按照发生的顺序被处理。
- **事件循环** 不断运行，检查事件队列中是否有待处理的事件，并调用相应的事件处理器。

## 12.2.2 源代码组织结构

- 头文件include/event2：头文件，event.h提供核心函数，http.h头文件提供HTTP协议相关服务，rpc.h提供远程过程调用支持；
- 源码目录的头文件：
  - include/event2目录下部分头文件的包装；
  - Libevent内部使用的辅助性头文件；
- 通用数据结构目录compat/sys，仅有一个文件-queue.h，封装了跨平台的技术数据结构，包括单向链表、双向链表、队列、尾队列和循环队列；
- sample目录：提供一些示例程序；
- test：特工一些测试代码；
- WIN32-Code：提供Windows平台上一些专用代码；
- event.c文件：实现Libevent整体框架，主要是event和event_base两个结构体的相关操作；
- devpoll.c，kqueue.c，evport.c，select.c，win32select.c，poll.c，epoll.c文件：封装I/O复用机制。
- minheap-internal.h：实现了一个时间堆，以实现对定时事件的支持；
- signal.c：提供对信号的支持，针对结构体eventop所定义的接口函数的具体实现；
- evmap.c：维护句柄和事件处理器的映射关系；
- event_tagging.c：往缓冲区添加标记数据以及从缓冲区中读取标记数据的函数；
- event_iocp.c：提供对WindowsIOCP（Input/Output Completion Port，输入输出完成端口）的支持；
- buffer*.c：提供对网络I/O缓冲的控制，包括输入输出数据过滤，传输速率的限制，使用SSL协议对数据进行保护，以及零拷贝文件传输等；
- evthread*.c：提供对多线程的支持；
- listener.c：封装了对监听socket的操作，包括监听连接和接受连接；
- logs.c：日志系统；
- evutil.c，evutil_rand.c，strlcpy.c，arc4random.c：生成随机数，获取socket地址信息，读取文件，设置socket属性等操作；
- evdns.c，http.c，evrpc.c：提供了对DNS协议，HTTP协议和RPC协议（Remote Procedure Call）的支持；
- epoll_sub.c：未见使用。

在整个源码中，event-internal.h、include/event2/event_struct.h、event.c和evmap.c等四个文件最为重要，定义了event和event_base结构体，并实现了这两个相关结构体的操作。

## 12.2.3 event结构体

```
#define TAILQ_ENTRY(type) \
struct { \
	struct type* tqe_next; 	// 下一个元素
	struct type** tqe_prev; // 前一个元素的地址
}

struct event
{
	// 活动事件队列，所有被激活的事件处理器通过该成员串联成一个尾队列，称之为活动事件队列
	// 活动事件队列不止一个，不同优先级的事件处理器被激活后被插入到不同的活动事件队列队列中
	// 事件循环中，Reactor按照优先级从高到低遍历所有活动事件队列，依次处理其中的事件处理器。
	TAILQ_ENTRY(event) ev_active_next;
	// 所有已经注册的事件处理器（包括I/O事件处理器和信号事件处理器）通过该成员串联成一个尾队列，	  
	// 称之为注册事件队列
	TAILQ_ENTRY(event) ev_next;
	// 超时相关信息，可以是事件链表中下一个节点或者最小堆中的索引
	union{
		TAILQ_ENTRY(event) ev_next_with_common_timeout;
		int min_heap_idx;
	} ev_timeout_pos;
	// 事件关联的文件描述符
	evutil_socket_t ev_fd;
	// 事件所属的事件循环基础结构
	struct event_base* ev_base;
	// 通过联合体实现对不同类型事件的支持，I/O事件和信号事件
	// 具有相同文件描述符值的I/O事件处理器通过ev.ev_io.ev_io_next成员串成尾队列，称之为I/O事件队列
	// 具有相同信号值的信号处理器通过ev.ev_signal.ev_signal_next成员串成尾队列，称之为信号事件队列
	// ev_ncalls成员指定该信号发生时，Reactor需要执行多少次该事件对应的事件处理器中的回调函数
	union {
		// I/O事件链表的下一个节点和超时时间
		struct {
			TAILQ_ENTRY(event) ev_io_next;
			struct timeval ev_timeout;
		} ev_io;
		// 信号事件链表的下一个节点、触发次数、指向触发次数的指针
		struct {
			TAILQ_ENTRY(event) ev_signal_next;
			short ev_ncalls;
			short *ev_pncalls;
		} ev_signal;
	} _ev;
	// 事件类型，非互斥事件相或
	short ev_events;
	// 当前激活事件的类型
	short ev_res;
	// 一些事件标志
	// #define EVLIST_TIMEOUT 	0x01				// 事件处理器从属于通用定时器队列或者时间堆
	// #define EVLIST_INSERTED  0x02				// 事件处理器从属于注册事件队列
	// #define EVLIST_SIGNAL    0x04				// 没有使用
	// #define EVLIST_ACTIVE    0x08				// 事件处理器从属于活动事件队列
	// #define EVLIST_INTERNAL  0x10				// 内部使用
	// #define EVLIST_INIT      0x80				// 事件处理器已被初始化
	// #define EVLIST_ALL       0xf000 | 0x9f		// 定义所有标志
	short ev_flags;
	// 定义处理器优先级，值越小则优先级越高
	ev_uint8_t ev_pri;
	// 指定event_base执行事件处理器的回调函数时的行为
	// #define EV_CLOSURE_NONE 	  0		// 默认行为
	// #define EV_CLOSURE_SIGNAL  1		// 执行信号事件处理器回调函数时，调用ev_ncalls次回调函数
	// #define EV_CLOSURE_PERSIST 2		// 执行完回调函数后，再次将事件处理器加入到注册事件上
	ev_uint8_t ev_closure;
	// 定时器的超时值
	struct timeval ev_timeout;
	// 事件处理器的回调函数
	void (*ev_callback)(evutil_socket_t, short, void *arg);
	// 传递给回调函数的参数
	void *ev_arg;
};
```

## 12.2.4 往注册事件队列中添加事件处理器

详情参考书中源码及注释。

主要负责将事件加入到多路分器中。

## 12.2.5 往事件多路分发器中注册事件

详情参考书中源码及注释。

主要负责监听对应的事件以及建立并绑定文件描述符。

## 12.2.6 eventop结构体

```
struct eventop
{
	// 后端I/O复用技术的名称
	const char* name;
	// 初始化函数
	void *(*init)(struct event_base *);
	// 注册事件
	int (*add)(struct event_base *, evutil_socket_t fd, short old, 
		short events, void *fdinfo);
	// 删除事件
	int (*del)(struct event_base *, evutil_socket_t fd, short old,
		short events, void *fdinfo);
	// 等待事件
	int (*dispathc)(struct event_base *, struct timeval *);
	// 释放I/O复用机制使用的资源
	void (*dealloc)(struct event_base *);
	// 程序调用fork之后是否需要重新初始化event_base
	int need_reinit;
	// I/O复用技术支持的一些特性，支持边沿触发事件EV_ET-EV_FEATURE_ET，事件检测算法的复杂度时O(1)-EV_FEATURE_O1和监听socket上的事件同时监听其他类型的文件描述符-EV_FEATURE_FDS。
	enum event_method_feature features;
	//  有的I/O复用机制尾每个事件队列和信号事件队列分配额外的内存，避免同一个文件描述符被重复插入到I/O
	// 复用机制的事件表中，evmao_id_add函数在调用eventop的add/del方法时，将这段内存的起始地址作为
	// 第5个参数传递给add/del方法，下面这个成员指定这段内存的长度
	size_t fdinfo_len;
}
```

```
#ifdef _EVENT_HAVE_EVENT_PORTS
extern const struct eventop evportops;
#endif

#ifdef _EVENT_HAVE_SELECT
extern const struct eventop selectops;
#endif

#ifdef _EVENT_HAVE_POLL
extern const struct eventop pollops;
#endif

#ifdef _EVENT_HAVE_EPOLL
extern const struct eventop epollops;
#endif

#ifdef _EVENT_HAVE_WORKING_QUEUE
extern const struct eventop kqops;
#endif

#ifdef _EVENT_HAVE_DEVPOLL
extern const struct eventop devpollops;
#endif

#ifdef WIN32
extern const struct eventop win32ops;
#endif

static const struct eventop *eventops[] = {
#ifdef _EVENT_HAVE_EVENT_PORTS 
	&evportops,
#endif

#ifdef _EVENT_HAVE_WORKING_QUEUE
	&kqops,
#endif

#ifdef _EVENT_HAVE_EPOLL
	&epollops,
#endif

#ifdef _EVENT_HAVE_DEVPOLL
	&devpollops,
#endif

#ifdef _EVENT_HAVE_POLL
	&pollops,
#endif

#ifdef _EVENT_HAVE_SELECT
	&selectops,
#endif

#ifdef WIN32
	&win32ops,
#endif
	NULL;
}
```

Linux下默认使用的后端I/O复用技术是EPOLL。

## 12.2.7 event_base结构体

```
struct event_base {
	// 初始化Reactor的时候选择后端I/O复用机制
	const struct eventop *evsel;
	// I/O复用机制真正存储的数据，通过evsel成员的init函数来初始化
	void *evbase;
	// 事件变化队列，如果一个文件描述符上注册的事件被多次修改，使用缓冲来避免重复的系统调用，仅能用于
	// 时间复杂度是O(1)的I/O复用技术
	struct event_changelist changelist;
	// 指向信号的后端处理机制
	const struct eventop *evsigsel;
	// 信号事件处理器使用的数据结构，封装了一个由socketpair创建的管道，用于信号处理和事件多路分发器之间	// 的通信
	struct evsig_info sig;
	// 添加到event_base的虚拟事件，所有事件和激活事件的数量
	int virtual_event_count;
	int event_count;
	int event_count_active;
	// 是否执行完活动事件队列上剩余的任务后就退出事件循环
	int event_gotterm;
	// 是否立即退出循环，不管是否还有任务要处理
	int event_break;
	// 是否启动新的事件循环
	int event_continue;
	// 目前正在处理的活动事件队列的优先级
	int event_running_priority;
	// 事件循环是否已经启动
	int running_loop;
	// 活动事件队列数组，索引值越小，优先级越高
	struct event_list* activequeues;
	// 活动事件队列数组的大小，有多少个不同优先级的活动事件队列
	int nactivequeue;
	// 下面三个管理通用定时器
	struct common_timeout_list **common_timeout_queues;
	int n_common_timeouts;
	int n_common_timeouts_allocated;
	// 存放延迟回调函数的链表，事件循环每处理完一个活动事件队列中的所有事件后，调用一次延迟回调函数
	struct deferred_cb_queue defer_queue;
	// 文件描述符和I/O之间的映射关系表
	struct event_io_map io;
	// 信号值和信号事件之间的映射关系
	struct event_signal_map sigmap;
	// 注册事件队列，存放I/O事件处理器和信号事件处理器
	struct event_list eventqueue;
	// 时间堆
	struct min_heap timeheap;
	// 管理系统时间的成员
	struct timeval event_tv;
	struct timeval tv_cache;
#if defined(_EVENT_HAVE_CLOCK_GETTIME) && defined(CLOCK_MONOTONIC)
	struct timeval tv_clock_diff;
	time_t last_updated_clock_diff;
#endif

// 下面是多线程支持
#ifdef _EVENT_DISABLE_THREAD_SUPPORT
	// 当前运行该event_base事件循环的线程
	unsigned long th_owner_id;
	// 对event_base的独占苏
	void *th_base_lock;
	// 当前事件循环正在执行哪个事件处理器的回调函数
	struct event* current_event;
	// 条件变量，用于唤醒正在等待某个事件处理完毕的线程
	void *current_event_cond;
	// 等待条件变量的线程数
	int current_event_waiters;
#endif

#ifdef WIN32
	struct event_iocp_port *iocp;
#endif
	// event_base的一些配置参数
	enum event_base_config_flag flags;
	// 下面的变量使用socketpair提供了工作线程唤醒主线程的方法
	int is_notify_pending;
	evutil_socket_t th_notify_fd[2];
	struct event th_notify;
	int (*th_notify_fn)(struct event_base *base);
};
```

## 12.2.8 事件循环

event_base_loop事件循环函数，调用I/O事件多路分发器的事件监听函数，以等待事件，当有事件发生时，就依次处理。

```
int event_base_loop(struct event_base *base, int flags) {
    while (!event_base_got_exit(base)) {
        struct timeval tv;
        struct timeval *tv_p;

        // 获取离最近的定时器事件触发还有多久
        if (event_base_gettimeofday_cached(base, &tv) == 0) {
            struct timeval next;
            if (evtimer_queue_get_duration(base, &next) != 0) {
                tv_p = NULL;  // 无定时器事件
            } else {
                evutil_timersub(&next, &tv, &tv);
                tv_p = &tv;
            }
        } else {
            tv_p = NULL;
        }

        // 多路事件分发器等待事件发生
        int res = evport_base_loop(base, flags, tv_p);

        // 处理已激活的事件
        event_process_active(base, res);

        // 处理超时的定时器事件
        if (evtimer_queue_incref_and_lock_(base)) {
            evtimer_queue_purge(base);
            evtimer_queue_decref_and_unlock_(base);
        }

        // 检查是否需要退出事件循环
        if (event_base_got_exit(base)) {
            break;
        }
    }

    return 0;
}
```

详情参考书中源码及注释。