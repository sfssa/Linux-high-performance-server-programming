# 10.1 Linux信号概述

信号是由用户、应用程序、系统发送给目标进程的信息，通知目标进程某个状态改变或系统异常，Linux发送信号情况如下：

- 对于前台进程，通过输入特殊字符发送信号，例如输入`CTRL+C`给进程发送一个终止信号；
- 系统异常，例如浮点异常或者越界访问；
- 系统状态变化，比如`alarm`定时到期会引起`SIGALARM`信号；
- 运行`KILL`命令或者`KILL`调用。

服务器必须处理或者忽略常用的信号，以免异常终止。

## 10.1.1 发送信号

在Linux中，`kill` 函数用于向指定的进程或进程组发送信号。它有以下形式：

```
#include <signal.h>

int kill(pid_t pid, int sig);
```

其中：

- `pid` 是目标进程的进程ID。
- `sig` 是要发送的信号的编号。

`kill` 函数的主要作用是向目标进程发送指定的信号。以下是一些常用的信号编号，其中 `SIGTERM` 和 `SIGKILL` 是比较常见的：

- `SIGTERM` (15)：终止进程。通常用于请求进程正常退出。
- `SIGKILL` (9)：强制终止进程。该信号会立即终止目标进程，不允许进程进行清理工作。
- `SIGHUP` (1)：挂起。通常用于通知进程重新加载配置。
- `SIGINT` (2)：中断。通常由终端产生，用于中断运行中的进程。
- `SIGQUIT` (3)：退出。类似于 `SIGINT`，但产生的 core dump。

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10_1.png)

Linux定义的信号值都大于0，如果等于0，表示不发送任何信号，可以用来检测目标进程或进程组是否存在，因为检查工作在信号发送前就完成了。但是这种检测方式不可靠，一方面由于PID回绕，可能导致被检测的PID不是期望的PID；另一方面这种检测不是原子检测。

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10_2.png)

## 10.1.2 信号处理方式

处理信号的接收函数原型：

```
#include <signal.h>
typedef void (*__sighandler_t) (int)
```

信号处理函数是可重入的，否则会导致竞态条件，在信号处理函数严禁调用不安全的函数。

在信号处理中，`DFL_DEL` 和 `DFL_IGN` 并非标准的函数名称。在标准的信号处理中，通常使用以下两种方式来处理信号：

1. **默认处理（Default Action）：** 也称为默认行为。当进程接收到某些信号时，系统会执行默认的处理操作。例如，对于 `SIGTERM` 信号，默认的处理是终止进程。
2. **忽略处理（Ignore Action）：** 也称为忽略。当进程接收到某些信号时，系统会忽略该信号，不采取任何操作。通常，这对于某些信号不希望被处理的情况很有用。

在信号处理中，可以使用 `signal` 函数（较老的方式）或 `sigaction` 函数（更灵活和可移植的方式）来设置信号处理行为。

```
#include <bits/signm.h>
#define SIG_DEL ((__sighandler_t) 0)
#define SIG_IGN ((__SIGHANDLER_t) 1)
```

默认处理方式有以下几种：

- 结束进程-Term；
- 忽略进程-Ign；
- 结束进程并生成核心转储文件-Core；
- 暂停进程-Stop；
- 继续进程-Cont。

## 10.1.3Linux信号

Linux中可用信号都定义在`bits/signum.h`头文件中：

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10_3.png)

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10_4.png)

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10_5.png)

## 10.1.4 中断系统调用

如果程序在执行处于阻塞态的系统调用时收到了信号，并且为该函数设置了处理函数，默认情况下这个系统调用会被中断，将errno设置为EINTR，可以使用sigaction函数微信号设置SA_RESTART标志以自动重启被该信号中断的系统调用。

对于默认行为是暂停进程的信号（SIGSTOP,SIGTTIN），即使没有设置信号处理函数，它们也可以中断某些系统调用（connect,epoll_wait）。

# 10.2 信号函数

## 10.2.1 signal系统调用

`signal` 函数是C语言中用于设置信号处理函数的函数。它的原型如下：

```
#include <signal.h>
_sighandler_t signal (int sig, _sighangler_t _handler);
```

其中：

- `sig`参数指出要捕获的信号类型，`_handler`参数是信号的处理函数；
- `signal`函数成功返回一个函数指针，该函数指针类型也是`_sighandler_t`,这个返回值是前一次调用`signal`函数时传入的函数指针或者时信号对应的默认函数指针。`signal`系统调用出错时返回`SIG_ERR`，设置`errno`。

## 10.2.2 sigaction系统调用

`sigaction` 是一个用于设置和检索信号处理函数的系统调用。它提供了比较灵活和可靠的方式来处理信号，相对于较早的 `signal` 函数而言，更为强大。

其函数原型如下：

```
#include <signal.h>

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact);
```

- `signum` 是要操作的信号的编号。
- `act` 是一个指向 `struct sigaction` 结构的指针，表示要设置的新的信号处理方式。
- `oldact` 是一个指向 `struct sigaction` 结构的指针，用于存储原来的信号处理方式。

`struct sigaction` 结构的定义如下：

```
struct sigaction {
    void (*sa_handler)(int);    // 指定信号的处理函数
    sigset_t sa_mask;           // 指定在信号处理函数执行期间需要阻塞的信号集合
    int sa_flags;               // 指定信号处理的一些标志
    void (*sa_sigaction)(int, siginfo_t *, void *);  // 用于扩展的信号处理函数，可以携带更多信息
};
```

下面是 `sigaction` 的一些关键参数的说明：

- `sa_handler`：指定信号的处理函数，与早期的 `signal` 函数类似。可以指定为一个函数指针，也可以是 `SIG_DFL` 表示使用默认处理，或 `SIG_IGN` 表示忽略信号。
- `sa_mask`：在执行信号处理函数期间需要阻塞的信号集合。这可以防止在处理一个信号时被同一信号再次中断（哪些信号不能发送到进程）。
- `sa_flags`：指定信号处理的一些标志，例如 `SA_RESTART` 表示在信号处理函数返回后自动重启被中断的系统调用。
- `sa_sigaction`：用于扩展的信号处理函数，可以携带更多的信息。

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10_6.png)

# 10.3 信号集

## 10.3.1 信号集函数

`sigset_t` 是用于表示信号集合的数据类型。它是一个位图，每个位表示一个特定的信号。在 C 语言中，`sigset_t` 是由系统提供的一个数据类型，通常是一个数组或者一个整数。

在使用 `sigset_t` 时，我们可以通过一些相关的宏和函数来进行操作，例如：

- `sigemptyset(sigset_t *set)`：将信号集合清空，即所有的位都被设置为 0。
- `sigfillset(sigset_t *set)`：将信号集合填满，即所有的位都被设置为 1。
- `sigaddset(sigset_t *set, int signum)`：向信号集合中添加一个指定信号。
- `sigdelset(sigset_t *set, int signum)`：从信号集合中删除一个指定信号。
- `sigismember(const sigset_t *set, int signum)`：检查一个指定信号是否属于信号集合。

## 10.3.2 进程信号掩码

在 Linux 中，`sigprocmask` 是一个用于检索或修改当前进程的信号屏蔽字（signal mask）的系统调用。信号屏蔽字是一个位图，用于指定在当前进程中哪些信号是被阻塞（屏蔽）的。当一个信号被阻塞时，它将被暂时忽略，直到信号被解除阻塞。

`sigprocmask` 的函数原型如下：

```
#include <signal.h>

int sigprocmask(int how, const sigset_t *set, sigset_t *oldset);
```

- ```
  how
  ```

   参数表示要执行的操作，可以是以下几种值之一：

  - `SIG_BLOCK`：将 `set` 中的信号添加到当前信号屏蔽字中。
  - `SIG_UNBLOCK`：从当前信号屏蔽字中移除 `set` 中的信号。
  - `SIG_SETMASK`：将当前信号屏蔽字设置为 `set` 中的值。

- `set` 参数是一个指向 `sigset_t` 类型的指针，表示要设置的信号屏蔽字。

- `oldset` 参数是一个指向 `sigset_t` 类型的指针，如果不为 `NULL`，则表示存储之前的信号屏蔽字。

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10_7.png)

## 10.3.3 被挂起的信号

设置进程信号掩码后，被屏蔽的信号无法被进程接收，如果给进程发送一个被屏蔽的信号，操作系统会为该信号设置为进程的一个被挂起的信号，当取消对该信号的屏蔽，进程会立刻收到这个信号。

`sigpending` 函数用于获取当前被阻塞（屏蔽）且已经产生但尚未处理的信号集。它可以获取在调用 `sigprocmask` 时被阻塞的信号集中已经产生但尚未被处理的信号。

函数原型如下：

```
#include <signal.h>

int sigpending(sigset_t *set);
```

- `set` 参数是一个指向 `sigset_t` 类型的指针，用于存储已经产生但尚未处理的信号集。

调用成功时，`sigpending` 返回 0，表示成功获取了信号集。如果出现错误，返回 -1，并设置 `errno`。

即使收到多个同一个被挂起的信号，也只能反应一次，并且取消屏蔽后只能触发一次处理函数。

# 10.4 统一事件源

信号是一种异步机制，信号处理函数和程序的主循环是两条不同的执行线，信号处理函数需要尽快的执行完毕，以确保该信号不会被屏蔽太久。典型的解决方案是：把信号的主要处理逻辑放到程序的主循环中，当信号处理函数被触发时，简单的通知主循环接收到信号，把信号传递给主循环，主循环根据接收到的信号值执行目标信号对应的逻辑代码。通常信号处理函数使用管道来将信号传递给主循环，信号处理函数往管道的写段写入信号值；主循环从管道的读端读出该信号值。为了使主循环知道管道上有数据可读，需要使用I/O复用系统调用来监听读端文件描述符上的可读事件。

`sockpair` 函数是一个用于创建全双工的 UNIX 域套接字对（Unix Domain Socket Pair）的系统调用。这个函数的目的是创建两个相互连接的套接字，它们可以在两个进程之间进行通信。UNIX 域套接字用于在同一台计算机上的进程之间进行本地通信。

函数原型如下：

```
#include <sys/types.h>
#include <sys/socket.h>

int socketpair(int domain, int type, int protocol, int sv[2]);
```

- `domain`：指定套接字的域，通常为 `AF_UNIX`（UNIX 域套接字）。
- `type`：指定套接字的类型，通常为 `SOCK_STREAM`（流套接字）。
- `protocol`：指定协议，通常为 `0`。
- `sv`：一个包含两个套接字描述符的整数数组，其中 `sv[0]` 是用于读的套接字，`sv[1]` 是用于写的套接字。

调用成功时，`socketpair` 返回 `0`，并将两个套接字的文件描述符分别存储在 `sv` 数组中。调用失败时，返回 `-1`，并设置 `errno` 指示错误的原因。

这两个套接字是全双工的，可以同时进行读和写操作。它们通过文件描述符传递数据，因此可以在两个相关联的进程之间进行通信。UNIX 域套接字通常用于本地通信，具有低延迟和高性能的特点。

# 10.5 网络编程相关信号

## 10.5.1 sighup

当挂起进程的控制终端时，`SIGHUP`信号被触发，对于没有控制终端的网络后台程序而言，常利用`SIGHUP`来强制服务器重读配置文件，例如`xinetd`程序，接收到SIGHUP信号后将调用`hard_reconfig`函数，循环读取`/etc/xinetd.d/`目录下的每个子配置文件，检测其变化。如果某个正在运行的自服务的配置文件被修改以停止服务，则`xinetd`主进程将给该子服务进程发送`SIGTERM`信号来结束它；如果某个子服务的配置文件被修改以开启服务，则`xinetd`将创建新的`socket`并绑定到该服务对应的端口上。

## 10.5.2 sigpipe

默认情况下，往读端被关闭的管道或`socket`写数据时会触发`SIGPIPE`信号，需要在代码中捕捉这个信号或者至少忽略它，因为程序默认接收到`SIGPIPE`信号的默认行为是结束进程。可以使用`send`函数的`MSG_NOSIGNAL`标志来禁止写操作触发`SIGPIPE`信号，在这种情况下，使用`send`函数返回的`errno`来判断管道或者`socket`连接的读端是否已经关闭。也可以利用I/O复用系统调用来检测。以`poll`为例，当管道的读端关闭时，写端文件描述符上的`POLLHUP`事件被触发，当socket连接被对方关闭时，`socket`上的`POLLRDHUP`事件被触发。

## 10.5.3 sigurg

利用SIGURT信号来通知应用程序带外数据到达。