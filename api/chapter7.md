### `日志级别`

```
#include <syslog.h>
#define LOG_EMERG	// 系统不可用-0
#define LOG_ALERT	// 报警，需要立即采取动作-1
#define LOG_CRIT	// 非常严重的情况
#define LOG_ERR		// 错误
#define LOG_WARNING // 警告
#define LOG_NOTICE  // 通知
#define LOG_INFO	// 信息
#define LOG_DEBUG 	// 调试
```

---

### `openlog` 

函数是一个用于配置和初始化系统日志的函数。它通常与 `syslog` 函数一起使用，用于将程序的日志消息发送到系统日志。在使用 `syslog` 函数输出日志之前，通常需要调用 `openlog` 来进行初始化配置。

函数原型如下：

```
#include <syslog.h>

void openlog(const char *ident, int option, int facility);
```

- `ident`：一个标识符字符串，用于在每条日志消息的前面加上一个标识符。通常是程序的名字或其他标识符。
- `option`：配置选项，可以是下列常量的按位或（OR）组合：
  - `LOG_CONS`：如果无法将消息写入到系统日志，将消息输出到控制台。
  - `LOG_NDELAY`：在调用 `syslog` 函数之前立即打开连接到系统日志的设备。
  - `LOG_PERROR`：同时将日志消息写入到系统日志和标准错误流。
  - `LOG_ODELAY`：延迟打开日志功能知道第一次调用syslog。
  - `LOG_PID`：在每条日志消息中包含进程标识符。
- `facility`：指定日志消息的设施（facility）。设施表示日志消息的类型，例如是来自用户级别还是系统级别的消息。可以是下列常量之一：
  - `LOG_USER`：用户级别的消息。
  - `LOG_LOCAL0` 到 `LOG_LOCAL7`：本地设施，用于自定义应用程序。

通常，程序在启动时调用 `openlog` 进行初始化设置，然后在程序运行期间使用 `syslog` 函数将日志消息写入系统日志。

---

### `setlogmask` 

函数是用于设置系统日志中哪些优先级的消息应该被记录的函数。在使用 `syslog` 函数输出日志消息之前，通常需要调用 `setlogmask` 进行配置，以确定记录哪些级别的日志消息。

函数原型如下：

```
#include <syslog.h>

int setlogmask(int maskpri);
```

- `maskpri`：一个掩码，表示要记录的日志消息的最小优先级。掩码是一个位掩码，每一位对应一个日志消息的优先级。如果相应位被设置为1，表示记录该优先级的日志消息；如果设置为0，表示不记录。

`maskpri` 的取值可以使用下列常量之一，也可以通过按位或（OR）组合多个常量来设置多个级别：

- `LOG_EMERG`：紧急情况，系统不可用。
- `LOG_ALERT`：需要立即采取行动的情况，例如数据库损坏。
- `LOG_CRIT`：临界状态，如硬件错误。
- `LOG_ERR`：错误消息。
- `LOG_WARNING`：警告消息。
- `LOG_NOTICE`：普通但重要的事件。
- `LOG_INFO`：一般的信息性消息。
- `LOG_DEBUG`：调试级别的消息。

返回值是之前的日志掩码。如果发生错误，返回 -1。

---

### `closelog` 

`closelog` 函数用于关闭系统日志，它的原型为：

```
#include <syslog.h>

void closelog(void);
```

`closelog` 函数通常在程序结束时调用，用于清理与系统日志相关的资源。

---

### `用户信息`

```
#include <sys/types.h>
#include <unistd.h>
uid_t getuid();				// 获得真实用户id
uid_t geteuid();			// 获得有效用户id
gid_t getgid();				// 获得真实组id
gid_t getegid();			// 获得有效组id
int setuid(uid_t uid);		// 设置真实用户id
int seteuid(uid_t uid);		// 设置有效用户id
int setgid(gid_t gid);		// 设置真实组id
int setegid(gid_t gid);		// 设置有效组id
```

1. **真实组（Real Group）：**
   - **概念：** 用户登录时分配给他们的组，通常是用户在系统中的默认组。
   - **获取方式：** 用户登录时，系统会将真实组设置为用户的默认组。
   - **影响：** 用户的真实组用于确定其在文件系统中创建新文件时的默认组所有权。
2. **有效组（Effective Group）：**
   - **概念：** 用户在执行程序时使用的组，用于确定对文件的访问权限。
   - **获取方式：** 用户登录时，有效组通常与真实组相同。然而，在执行一些程序时，系统可能会修改有效组。
   - **影响：** 有效组用于确定用户在运行程序时对文件的访问权限。这允许用户在执行某些程序时具有不同于其真实组的文件访问权限。

1. **真实用户ID（Real User ID）：**
   - **概念：** 用户登录时分配给他们的唯一用户标识。
   - **获取方式：** 用户登录时，系统将真实用户ID设置为登录用户的唯一标识。
   - **影响：** 用于标识用户的真实身份，决定用户对文件和系统资源的访问权限。
2. **有效用户ID（Effective User ID）：**
   - **概念：** 用户在执行程序时使用的用户ID，用于确定对文件的访问权限。
   - **获取方式：** 用户登录时，有效用户ID通常与真实用户ID相同。但在执行一些程序时，系统可能会修改有效用户ID。
   - **影响：** 有效用户ID用于确定用户在运行程序时对文件和系统资源的访问权限。这允许用户在执行某些程序时具有不同于其真实用户ID的权限。

---

### `set_user_id`

指设置用户标识的一种机制，通常通过执行文件时设置程序的用户标识。机制的目的是为了让程序在执行时以指定用户的权限运行，而不是执行程序的用户的权限。例如`su`程序需要访问`/etc/passwd`文件，但这个文件为`root`所拥有，因此需要设置`set_user_id`，在执行时使用有效`id`也就是`root`的`id`来运行。

---

### `getpgid` 

- 原型：

  ```
  #include <unistd.h>
  
  pid_t getpgid(pid_t pid);
  ```

- **作用：** 获取指定进程的进程组ID。

- **参数：** `pid` 是指定的进程ID，如果 `pid` 为 0，则表示获取调用进程的进程组ID。

- **返回值：** 如果成功，返回指定进程的进程组ID；如果出错，返回 -1。

---

### `setpgid` 

- 原型：

  ```
  #include <unistd.h>
  
  int setpgid(pid_t pid, pid_t pgid);
  ```

- **作用：** 设置指定进程的进程组ID。

- 参数：

  - `pid` 是指定的进程ID，如果为 0，则表示使用调用进程的进程ID。
  - `pgid` 是要设置的新的进程组ID。

- **返回值：** 如果成功，返回 0；如果出错，返回 -1。

这两个函数通常用于在创建子进程时管理进程组。在Unix系统中，每个进程都属于一个进程组。进程组的ID由其中一个进程的进程ID决定，这个进程被称为进程组的领导者。进程组的ID用于向组中的所有进程发送信号。

Linux每个进程都所属一个进程组，每个进程组有一个首领进程，首领进程的PID和PGID相等。在设置PID时，将PID为pid的进程的PGID设置为pgid，如果：

- pid和pgid相同，则由pid指定的进程被设置为首领进程；
- pid为0，表示设置当前进行的PGID为pgid；
- pgid为0，使用pid作为目标PGID；（将一个进程移动到与它的进程ID相匹配的进程组）

一个进程只能设置自己或者其子进程的PGID，并且当子进程调用exec系列函数后，不能在父进程中设置PGID。（人家都死了，你还想扒人家坟墓？？？）

---

### `setsid`

 函数用于创建一个新的会话（session）并设置调用进程为该会话的首领（session leader）。这样的新会话通常用于创建孤儿进程，即脱离父进程并成为一个新的进程组和会话的进程。

函数原型如下：

```
#include <unistd.h>

pid_t setsid(void);
```

- **返回值：** 如果调用成功，返回新的会话的会话ID（SID）；如果失败，返回 -1。

`setsid` 函数的调用过程：

1. 如果调用进程不是一个进程组的首领（session leader），那么创建一个新的会话。
2. 将调用进程设置为新会话的首领，并将其进程ID设为新的会话ID。
3. 将调用进程从原来的进程组中移出，使其成为一个新的进程组的唯一成员。

使用 `setsid` 函数的典型场景是在守护进程（daemon）的初始化过程中，通过创建一个新的会话，使守护进程脱离父进程的控制终端，从而避免受到终端会话的影响。

一群有关联的进程组将形成一个会话，不能由首领进程创建新的会话（你把人家的首领挖走了，让原来的进程组怎们办？），非首领进程调用创建会话函数后：

- 调用进程称为会话的首领，该进程是会话的唯一成员；
- 新建一个进程组，PGID就是调用进程的PID；
- 调用进程如果有终端，会甩开终端；

---

### `getsid` 

函数用于获取指定进程的会话ID（SID）。这个函数允许你通过指定一个进程ID获取对应进程所属会话的会话ID。

函数原型如下：

```
#include <unistd.h>

pid_t getsid(pid_t pid);
```

- **参数：** `pid` 是目标进程的进程ID。
- **返回值：** 如果调用成功，返回目标进程所属会话的会话ID；如果失败，返回 -1。

---

### `相关id`

1. **PID（Process ID）：**
   - **意义：** 进程的唯一标识符，用于在系统中唯一标识一个进程。
   - **获取方式：** 使用系统调用 `getpid()` 获取当前进程的PID。
2. **PPID（Parent Process ID）：**
   - **意义：** 父进程的PID，即创建当前进程的进程的PID。
   - **获取方式：** 使用系统调用 `getppid()` 获取当前进程的父进程的PID。
3. **PGID（Process Group ID）：**
   - **意义：** 进程组的唯一标识符，用于将一组相关的进程组织在一起。
   - **获取方式：** 使用系统调用 `getpgid(pid)` 获取指定进程的进程组ID。对于当前进程，可以使用 `getpgid(0)` 获取当前进程所属的进程组ID。
4. **SID（Session ID）：**
   - **意义：** 会话的唯一标识符，用于将一组进程组织在一起形成一个会话。
   - **获取方式：** 使用系统调用 `getsid(pid)` 获取指定进程的会话ID。对于当前进程，可以使用 `getsid(0)` 获取当前进程所属的会话ID。

---

### `getrlimit` 

```
#include <sys/resource.h>

int getrlimit(int resource, struct rlimit *rlim);
```

- **作用：** 获取指定资源的当前限制。
- 参数：
  - `resource`：表示要获取限制的资源，例如 `RLIMIT_CPU` 表示CPU时间限制。
  - `rlim`：一个指向 `rlimit` 结构体的指针，用于存储获取到的资源限制。
- **返回值：** 如果成功，返回0；如果失败，返回-1。

---

###  `setrlimit`

```
#include <sys/resource.h>

int setrlimit(int resource, const struct rlimit *rlim);
```

- **作用：** 设置指定资源的限制。
- 参数：
  - `resource`：表示要设置限制的资源，例如 `RLIMIT_CPU` 表示CPU时间限制。
  - `rlim`：一个指向 `rlimit` 结构体的指针，用于指定新的资源限制。
- **返回值：** 如果成功，返回0；如果失败，返回-1。

---

###  `rlimit` 

```
#include <sys/resource.h>

struct rlimit {
    rlim_t rlim_cur;  // 软限制（soft limit）
    rlim_t rlim_max;  // 硬限制（hard limit）
};
```

- `rlim_cur`：表示当前的资源限制。软限制是内核弹性的限制，进程可以在不超过硬限制的情况下调整软限制。
- `rlim_max`：表示硬限制，是软限制的上限，超过这个限制将导致进程收到信号。

这些函数和结构体用于管理各种资源的限制，例如CPU时间、内存、文件大小等。通过设置这些资源限制，可以帮助操作系统有效地管理系统资源，防止某个进程滥用系统资源。

![image-20231224110835210](C:\Users\16645\AppData\Roaming\Typora\typora-user-images\image-20231224110835210.png)

---

### `getcwd` 

```
#include <unistd.h>

char *getcwd(char *buf, size_t size);
```

- **作用：** 获取当前工作目录的绝对路径。
- 参数：
  - `buf`：一个指向存储路径名的缓冲区的指针。
  - `size`：缓冲区的大小。
- **返回值：** 如果成功，返回指向缓冲区的指针；如果失败，返回 NULL。

**注意：**

- 如果当前工作目录的绝对路径 + \0 的长度超过了size，那么会返回NULL，将errno设置为ERANGE；
- 如果buf为NULL，并且size不等于0，那么getcwd可能在内部使用malloc动态分配内存，将进程的当前工作目录存储到其中。这种情况需要自己释放这块内部创建的内存。

---

###  `chdir` 

```
#include <unistd.h>

int chdir(const char *path);
```

- **作用：** 改变当前工作目录。
- 参数：
  - `path`：一个指向要变更为的目录路径的字符串。
- **返回值：** 如果成功，返回0；如果失败，返回-1。

---

### `chroot`

 函数用于将当前进程的根目录切换为指定目录，创建一个新的根文件系统。这对于安全性或特定环境隔离的需求很有用。注意，`chroot` 对于提高系统安全性的效果有限，且需要特殊的权限。

函数原型如下：

```
#include <unistd.h>

int chroot(const char *path);
```

- **作用：** 将当前进程的根目录切换为指定目录。
- 参数：
  - `path`：要成为新的根目录的目录路径。
- **返回值：** 如果成功，返回0；如果失败，返回-1。

使用 `chroot` 后，程序将无法访问指定目录之外的文件和目录，从而在该目录中创建一个独立的环境。这对于某些安全性和隔离的应用场景非常有用，例如容器技术和一些安全测试工具。

注意事项：

- 使用 `chroot` 需要特殊的权限（通常需要超级用户权限）。
- `chroot` 不会关闭已经打开的文件描述符，因此在切换根目录后需要小心处理文件访问。
- 由于 `chroot` 并不提供强大的安全性保证，现代系统更倾向于使用更强大的容器技术（如Docker）来实现更可靠的隔离。

`chroot`并不改变进程当前工作目录，调用后仍需要使用`chdir("/")`将工作目录切换到新的根目录。更改到新目录后，程序可能无法访问类似/dev的文件和目录，因为这些文件和目录并不在于indeed根目录下，不过调用`chroot`之前打开的文件描述符依然生效。此外，只有特权进程才能改变根目录。

---

### `daemon` 

函数用于将当前进程转变为后台守护进程。守护进程是在后台运行的一种特殊类型的进程，通常与终端分离，不受用户登录或注销的影响。

函数原型如下：

```
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int daemon(int nochdir, int noclose);
```

- **作用：** 将当前进程转变为守护进程。
- 参数：
  - `nochdir`：为非零值时，表示不改变当前工作目录；为零值时，表示将当前工作目录切换到根目录。
  - `noclose`：为非零值时，表示不关闭标准输入、标准输出和标准错误文件描述符；为零值时，表示关闭这些文件描述符（重定向到/dev/null）。
- **返回值：** 如果成功，返回0；如果失败，返回-1。

`daemon` 函数通常在调用 `fork` 后的子进程中使用。它执行以下操作：

1. 关闭父进程的标准输入、标准输出和标准错误文件描述符（如果 `noclose` 参数为零）。
2. 如果 `nochdir` 参数为零，将当前工作目录切换到根目录。
3. 调用 `setsid` 创建一个新的会话，并将该进程设置为新会话的首领，断开与终端的关联。
4. 再次调用 `fork`，终止父进程，使子进程变成孤儿进程，不再关联任何终端。

---

### `umask`

 函数用于设置文件创建掩码，它决定了新建文件的默认权限。掩码中的位是被屏蔽的权限，因此 `umask` 的作用是限制文件的默认权限。

```
#include <sys/types.h>
#include <sys/stat.h>

mode_t umask(mode_t mask);
```

- **作用：** 设置文件创建掩码，返回之前的掩码值。
- 参数：
  - `mask`：要设置的新的文件创建掩码，可以通过按位或操作设置多个权限位。
- **返回值：** 返回之前的文件创建掩码。

`umask` 函数的参数 `mask` 是一个权限掩码，它指定了在创建新文件时将被屏蔽的权限。新文件的实际权限是使用默认权限（通常是 0666）减去掩码中指定的权限。

---

