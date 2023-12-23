### `int pipe` 

函数用于创建一个管道(pipe)，管道是一种进程间通信的机制，它允许一个进程将数据写入管道，而另一个进程从管道中读取数据。`pipe` 函数创建一个具有两个文件描述符的管道，其中 `fd[0]` 用于读取数据，`fd[1]` 用于写入数据。

函数的参数 `fd` 是一个包含两个整数的数组，传递给函数时，该数组会被填充为两个新的文件描述符，分别用于读取和写入。通常，`fd[0]` 用于从管道读取数据，`fd[1]` 用于向管道写入数据。

函数原型如下：

```
#include <unistd.h>
int pipe(int fd[2]);
```

- 参数 `fd`：一个包含两个整数的数组，用于存储管道的两个文件描述符。
- 返回值：如果成功，返回 0；如果失败，返回 -1。

其中[0]只负责读，[1]只负责写，如果要实现双向通信需要两个管道。这对文件描述符默认都是阻塞的，调用read会阻塞，直到有数据可读；调用write会阻塞，直到有空闲空间可用。如果对[1]的引用减少至0，也就是没有任何进程往里面写数据，那么read函数返回0，也就是读取到了EOF（End Of File）；反之，如果[0]引用计数减少至0，也就是没有任何进程去读数据，则write函数会执行失败，返回一个SIGPIPE信号。

管道自身有容量限制，65536字节，可以通过fcntl函数修改。

---

### `socketpair`

在C语言中，`socketpair` 函数用于创建一对相互连接的套接字(socket)，这两个套接字可以用于进程间通信。`socketpair` 是一种在本地进程间进行通信的简便方式，它创建了两个相关联的套接字，一个用于读取，另一个用于写入。

函数原型如下：

```
#include <sys/types.h>
#include <sys/socket.h>
int socketpair(int domain, int type, int protocol, int sv[2]);
```

- 参数 `domain`：套接字的协议族，通常是 `AF_UNIX` 表示Unix域套接字。
- 参数 `type`：套接字的类型，通常是 `SOCK_STREAM` 表示面向连接的套接字。
- 参数 `protocol`：套接字使用的协议，通常是 0。
- 参数 `sv`：一个包含两个整数的数组，用于存储创建的两个套接字的文件描述符。
- 返回值：如果成功，返回 0；如果失败，返回 -1。

其中dimain只能使用AF_UNIX，因为只能在本地创建双向管道，最后一个参数和pipe系统调用一样，只不过是每个文件描述符既可读又可写。

---

### `dup` 

```
#include <unistd.h>

int dup(int oldfd);
```

- 参数 `oldfd`：被复制的文件描述符。
- 返回值：返回新的文件描述符（复制的副本），如果出错则返回 -1。

`dup` 函数用于复制文件描述符，它会返回一个新的文件描述符，该文件描述符与 `oldfd` 打开的文件或套接字指向相同的文件表项。这意味着两个文件描述符可以独立地访问相同的文件，它们共享文件偏移量和文件状态标志。通常，`dup` 返回的文件描述符是当前未使用的最小的文件描述符。`dup`默认返回系统可用最小值。

---

### `dup2` 

```
#include <unistd.h>

int dup2(int oldfd, int newfd);
```

- 参数 `oldfd`：被复制的文件描述符。
- 参数 `newfd`：新的文件描述符。
- 返回值：返回新的文件描述符，如果出错则返回 -1。

`dup2` 函数与 `dup` 类似，但允许显式指定新的文件描述符 `newfd`。如果 `newfd` 已经是一个打开的文件描述符，`dup2` 将先关闭它，然后再复制 `oldfd`。`dup2`返回不小于newfd的整数值。

**注意：**dup和dup2函数都不会继承源文件的描述符属性，例如close_on_exec，no_blocking等。

---

###  `readv` 

```
#include <sys/uio.h>

ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
```

- 参数 `fd`：文件描述符，用于指定从哪个文件或套接字读取数据。
- 参数 `iov`：一个指向 `struct iovec` 数组的指针，每个 `struct iovec` 描述一个缓冲区的位置和长度。
- 参数 `iovcnt`：`iov` 数组中元素的数量。
- 返回值：成功时返回读取的字节数，失败时返回 -1。

`readv` 函数从文件描述符 `fd` 中读取数据，并将数据按照 `iov` 数组中描述的缓冲区顺序填充。每个 `struct iovec` 包含一个指向缓冲区的指针和缓冲区的长度。这使得可以在一个系统调用中读取到多个不连续的缓冲区。

---

### `writev` 

```
#include <sys/uio.h>

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

- 参数 `fd`：文件描述符，用于指定将数据写入到哪个文件或套接字。
- 参数 `iov`：一个指向 `struct iovec` 数组的指针，每个 `struct iovec` 描述一个缓冲区的位置和长度。
- 参数 `iovcnt`：`iov` 数组中元素的数量。
- 返回值：成功时返回写入的字节数，失败时返回 -1。

`writev` 函数将多个不连续的缓冲区中的数据一并写入文件描述符 `fd`。与 `readv` 类似，每个 `struct iovec` 包含一个指向缓冲区的指针和缓冲区的长度。

---

### `readv` 

```
#include <sys/uio.h>

ssize_t readv(int fd, const struct iovec *iov, int iovcnt);
```

- 参数 `fd`：文件描述符，用于指定从哪个文件或套接字读取数据。
- 参数 `iov`：一个指向 `struct iovec` 数组的指针，每个 `struct iovec` 描述一个缓冲区的位置和长度。
- 参数 `iovcnt`：`iov` 数组中元素的数量。
- 返回值：成功时返回读取的字节数，失败时返回 -1。

`readv` 函数从文件描述符 `fd` 中读取数据，并将数据按照 `iov` 数组中描述的缓冲区顺序填充。每个 `struct iovec` 包含一个指向缓冲区的指针和缓冲区的长度。这使得可以在一个系统调用中读取到多个不连续的缓冲区。

---

### `writev` 

```
#include <sys/uio.h>

ssize_t writev(int fd, const struct iovec *iov, int iovcnt);
```

- 参数 `fd`：文件描述符，用于指定将数据写入到哪个文件或套接字。
- 参数 `iov`：一个指向 `struct iovec` 数组的指针，每个 `struct iovec` 描述一个缓冲区的位置和长度。
- 参数 `iovcnt`：`iov` 数组中元素的数量。
- 返回值：成功时返回写入的字节数，失败时返回 -1。

`writev` 函数将多个不连续的缓冲区中的数据一并写入文件描述符 `fd`。与 `readv` 类似，每个 `struct iovec` 包含一个指向缓冲区的指针和缓冲区的长度。

---

### `sendfile`

 函数是用于在两个文件描述符之间直接传输数据的系统调用，通常用于优化文件传输操作。它在文件的传输中可以显著提高性能，因为数据在内核中传输，而不需要在用户空间和内核空间之间复制。

函数原型如下：

```
#include <sys/sendfile.h>

ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
```

- 参数 `out_fd`：目标文件描述符，用于指定数据的目标。
- 参数 `in_fd`：源文件描述符，用于指定数据的来源。
- 参数 `offset`：一个指向 `off_t` 类型的指针，表示文件中的偏移量。如果不需要偏移，则可以将其设置为 `NULL`。
- 参数 `count`：要传输的字节数。
- 返回值：成功时返回传输的字节数，出错时返回 -1。

`sendfile` 函数的工作原理是将 `in_fd` 文件的数据传输到 `out_fd` 文件中。它避免了在用户空间和内核空间之间的缓冲区复制，提高了数据传输的效率。通常用于网络编程中将文件发送到套接字。

---

### `fstat` 

```
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int fstat(int fd, struct stat *buf);
```

- 参数 `fd`：文件描述符，用于指定要获取信息的文件。
- 参数 `buf`：一个指向 `struct stat` 结构的指针，用于存储文件信息。
- 返回值：成功时返回0，失败时返回-1。

`fstat` 函数通过文件描述符 `fd` 获取与之相关的文件信息，并将结果存储在 `struct stat` 结构体中。这个结构体包含了有关文件的各种信息，例如文件的大小、权限、拥有者等。

---

### `stat` 

```
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

int stat(const char *path, struct stat *buf);
```

- 参数 `path`：文件路径，用于指定要获取信息的文件。
- 参数 `buf`：一个指向 `struct stat` 结构的指针，用于存储文件信息。
- 返回值：成功时返回0，失败时返回-1。

`stat` 函数通过文件路径 `path` 获取指定文件的信息，并将结果存储在 `struct stat` 结构体中。与 `fstat` 不同，`stat` 不需要文件描述符，因此可以直接通过文件路径获取文件信息。

---

### `mmap` 

```
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
```

- 参数 `addr`：映射的起始地址。通常设置为 `NULL`，让操作系统选择合适的地址。
- 参数 `length`：映射区域的长度，以字节为单位。
- 参数 `prot`：映射区域的保护权限，可以是 `PROT_READ`、`PROT_WRITE`、`PROT_EXEC` 等的组合。
- 参数 `flags`：映射区域的标志，可以是 `MAP_SHARED`、`MAP_PRIVATE` 等的组合。
- 参数 `fd`：被映射的文件描述符，如果不映射文件可以设置为 -1。
- 参数 `offset`：文件映射的起始位置，通常设置为 0。
- 返回值：成功时返回映射区域的起始地址，失败时返回 `MAP_FAILED`。

`mmap` 函数将一个文件或者其它对象映射到调用进程的地址空间。它还可以用于创建匿名内存映射（不与文件关联），或者用于共享内存的创建。

![image-20231221225656052](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/6_1.png)

---

### `munmap` 

```
#include <sys/mman.h>

int munmap(void *addr, size_t length);
```

- 参数 `addr`：映射区域的起始地址。
- 参数 `length`：映射区域的长度，以字节为单位。
- 返回值：成功时返回 0，失败时返回 -1。

`munmap` 函数用于解除由 `mmap` 创建的映射关系，释放占用的内存。通常，当不再需要映射的区域时，应该调用 `munmap` 来释放资源。

---

### `splice`

 函数是Linux系统中用于在两个文件描述符之间移动数据的系统调用。它可以用于将数据从一个文件描述符移动到另一个文件描述符，而无需在用户空间中复制数据。`splice` 的主要应用场景是在文件描述符和套接字之间高效地传输数据。

函数原型如下：

```
#include <fcntl.h>
#include <unistd.h>

ssize_t splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);
```

- 参数 `fd_in`：源文件描述符，从这个文件描述符读取数据。
- 参数 `off_in`：读取操作的偏移量，可以为 `NULL`。
- 参数 `fd_out`：目标文件描述符，将数据写入到这个文件描述符。
- 参数 `off_out`：写入操作的偏移量，可以为 `NULL`。
- 参数 `len`：要移动的字节数。
- 参数 `flags`：标志位，通常设置为 0。
- 返回值：成功时返回移动的字节数，失败时返回 -1。

`splice` 函数在传输数据时，可以避免在用户空间和内核空间之间的复制，因此在某些情况下，它比传统的 `read` 和 `write` 更高效。

![image-20231221225656052](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/6_2.png)

![image-20231221225656052](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/6_3.png)

**注意：**如果fd_in是管道文件描述符，那么off_in必须设置为NULL；

---

### `tee` 

函数通常用于管道（pipe）的实现。在系统调用 `tee` 中，它允许从一个文件描述符复制数据到两个不同的文件描述符，实现数据的分流。它的函数原型如下：

```
#include <fcntl.h>

ssize_t tee(int fd_in, int fd_out, size_t len, unsigned int flags);
```

- 参数 `fd_in`：源文件描述符，从这个文件描述符读取数据。
- 参数 `fd_out`：目标文件描述符，将数据写入到这个文件描述符。
- 参数 `len`：要移动的字节数。
- 参数 `flags`：标志位，通常设置为 0。
- 返回值：成功时返回移动的字节数，失败时返回 -1。

`tee` 函数在两个文件描述符之间复制数据，同时保留数据在输入文件描述符中的副本。这允许数据在管道中进行传递，同时保留一份副本以供进一步处理。请注意，`tee` 函数的使用可能受到一些限制，具体取决于操作系统和文件系统的实现。fd_in和fd_out必须是管道。

---

### `fcntl`

（file control）是一个在Unix-like操作系统中用于对文件描述符进行控制的系统调用。通过 `fcntl` 函数，可以执行一系列与文件描述符相关的操作，如设置文件描述符的属性、获取文件状态标志、设置文件锁等。

函数原型如下：

```
#include <fcntl.h>

int fcntl(int fd, int cmd, ... /* arg */);
```

- 参数 `fd`：文件描述符，表示要进行操作的文件。
- 参数 `cmd`：表示要执行的操作，可以是以下之一：
  - `F_DUPFD`：复制文件描述符。
  - `F_GETFD`：获取文件描述符标志。
  - `F_SETFD`：设置文件描述符标志。
  - `F_GETFL`：获取文件状态标志。
  - `F_SETFL`：设置文件状态标志。
  - `F_GETLK`：获取文件锁。
  - `F_SETLK`：设置文件锁。
  - `F_SETLKW`：设置文件锁，但是如果锁不可用，则等待。
- 参数 `arg`：根据 `cmd` 的不同，可能需要传递额外的参数。
- 返回值：成功时返回操作的结果，失败时返回 -1。

![image-20231221225656052](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/6_4.png)

![image-20231221225656052](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/6_5.png)

`SIGIO`：当被关联的文件描述符可读或可写时，系统触发SIGIO信号；（需要设置O_ASYNC标志-异步IO）

`SIGURG`：当被关联的文件描述符（必须是一个socket）上有带外数据可读时触发SIGURG信号；

