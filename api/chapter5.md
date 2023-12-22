

**（注：需要包含头文件`arpa/inet.h`）**

### `htonl`（Host to Network Long）

- **参数**：`hostlong` - 主机字节序中的32位无符号整数。
- **返回值**：网络字节序中的32位无符号整数。
- **定义**：`uint32_t htonl(uint32_t hostlong);`
- **目的**：将32位无符号整数从主机字节序（通常是小端序）转换为网络字节序（大端序）。
- **主要用途**：确保数据在网络传输时字节顺序正确。

### `htons`（Host to Network Short）

- **参数**：`hostshort` - 主机字节序中的16位无符号整数。
- **返回值**：网络字节序中的16位无符号整数。
- **定义**：`uint16_t htons(uint16_t hostshort);`
- **目的**：将16位无符号整数从主机字节序（通常是小端序）转换为网络字节序（大端序）。
- **主要用途**：类似于 `htonl`，确保短整数在网络传输时字节顺序正确。

### `ntohl`（Network to Host Long）

- **参数**：`netlong` - 网络字节序中的32位无符号整数。
- **返回值**：主机字节序中的32位无符号整数。
- **定义**：`uint32_t ntohl(uint32_t netlong);`
- **目的**：将32位无符号整数从网络字节序（大端序）转换为主机字节序（通常是小端序）。
- **主要用途**：在接收网络数据时进行字节序转换，以确保数据在本机正确解释。

### `ntohs`（Network to Host Short）

- **参数**：`netshort` - 网络字节序中的16位无符号整数。
- **返回值**：主机字节序中的16位无符号整数。
- **定义**：`uint16_t ntohs(uint16_t netshort);`
- **目的**：将16位无符号整数从网络字节序（大端序）转换为主机字节序（通常是小端序）。
- **主要用途**：在接收网络数据时进行字节序转换，以确保数据在本机正确解释。

---

`#include <bits/socket.h>`

`struct sockaddr{`

​	`sa_family_t 		sa_family;		// 地址族类型`

​	`char 				sa_data[14];	// 具体地址信息`

`}`

![image-20231221193523957](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/1.png)![image-20231221193530103](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/2.png)

`#include <bits/socket.h>`

`struct sockaddr_storage{`

​	`sa_family_t 			sa_family;		// 地址族`

​	`unsigned long int 	__ss_align;		// 用以内存对齐`

​	`char 					__ss_padding[128 - sizeof(__ss_align)];		// 实际存放地址空间`

`}`

**注意:**解决了内存空间不够存放地址的问题;并使用了内存对齐.

`#include <sys/un.h>`

`struct sockaddr_un{`

​	`sa_family_t 		sin_family;				// 地址族:AF_UNIX`

​	`char 				sun_path[108];			// 文件路径名`

`}`

**注意:**UNIX本地域地址族专用数据结构.

`struct sockaddr_in{`

​	`sa_family_t 			 sin_family;			// 地址族:AF_INET`

​	`uint16_t 				 sin_port;				// 端口号-网络字节序`

​	`struct in_addr 		 sin_addr				// IPV4地址结构体`

`}`

`struct in_addr{`

​	`u_int32_t 		s_addr;							// IPV4地址-网络字节序`

`}`

`struct sockaddr_in6{`

​	`sa_family_t 			sin6_family;			// 地址族:AF_INET6`

​	`u_int16_t 			sin6_port;				// 端口号-网络字节序`

​	`u_int32_t 			sin6_flowinfo;			// 流消息-设置为0`

​	`struct in6_addr  		sin6_addr;				// IPV6地址结构体`

​	`u_int32_t 			sin6_scope_id;		    // 试验阶段`

`}`

`struct in6_addr{`

​	`unsigned char 		sa_addr[16];			// IPV6地址-网络字节序`

`}`

---

这三个函数涉及到IPv4地址的处理，通常需要包含 `<arpa/inet.h>` 头文件。

### **`inet_addr`**：

- **头文件**：`<arpa/inet.h>`
- **函数原型**：`in_addr_t inet_addr(const char *cp);`
- **作用**：将点分十进制的IPv4地址字符串转换为网络字节序的32位整数表示。
- **返回值**：如果成功，返回对应的32位整数；如果输入字符串不是有效的IPv4地址，返回 `INADDR_NONE`。

```
#include <arpa/inet.h>

const char* ip_str = "192.168.1.1";
in_addr_t ip_addr = inet_addr(ip_str);

```

### **`inet_aton`**：

- **头文件**：`<arpa/inet.h>`
- **函数原型**：`int inet_aton(const char *cp, struct in_addr *inp);`
- **作用**：将点分十进制的IPv4地址字符串转换为网络字节序的32位整数，并存储到 `struct in_addr` 结构中。
- **返回值**：如果成功，返回非零；如果输入字符串不是有效的IPv4地址，返回零。

```
#include <arpa/inet.h>

const char* ip_str = "192.168.1.1";
struct in_addr ip_addr;
int result = inet_aton(ip_str, &ip_addr);
```

### **`inet_ntoa`**：

- **头文件**：`<arpa/inet.h>`
- **函数原型**：`char *inet_ntoa(struct in_addr in);`
- **作用**：将32位网络字节序的IPv4地址转换为点分十进制的字符串表示。
- **注意**：该函数返回的指针指向一个静态分配的内存，因此在多线程环境中应谨慎使用，最好立即将结果拷贝到其他缓冲区。

**注意:**本函数用静态变量存放转换的结果,因此只能保留一个值,不可重入.

```
#include <arpa/inet.h>

struct in_addr ip_addr;
// 假设ip_addr已经被赋值为某个IPv4地址
char* ip_str = inet_ntoa(ip_addr);
```

下面这两个函数是IPv4和IPv6地址转换的更为通用和安全的版本，它们支持IPv4和IPv6两种地址类型。通常需要包含 `<arpa/inet.h>` 头文件。

### **`inet_pton`**：

- **头文件**：`<arpa/inet.h>`

- 函数原型

  ```
  int inet_pton(int af, const char *src, void *dst);
  ```

- **作用**：将点分十进制的IPv4或IPv6地址字符串转换为二进制格式，并存储到指定的内存中。

- 参数

  - `af`：地址族（`AF_INET` 表示IPv4，`AF_INET6` 表示IPv6）。
  - `src`：要转换的字符串形式的IP地址。
  - `dst`：用于存储转换后的二进制格式地址的内存。

- **返回值**：成功返回1，转换失败返回0，出错返回-1。

```
#include <arpa/inet.h>

const char* ipv4_str = "192.168.1.1";
const char* ipv6_str = "2001:0db8:85a3:0000:0000:8a2e:0370:7334";
struct in_addr ipv4_addr;
struct in6_addr ipv6_addr;

int result_ipv4 = inet_pton(AF_INET, ipv4_str, &ipv4_addr);
int result_ipv6 = inet_pton(AF_INET6, ipv6_str, &ipv6_addr);
```

### **`inet_ntop`**：

- **头文件**：`<arpa/inet.h>`

- 函数原型

  ```
  const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
  ```

- **作用**：将二进制格式的IPv4或IPv6地址转换为点分十进制的字符串形式。

- 参数

  - `af`：地址族（`AF_INET` 表示IPv4，`AF_INET6` 表示IPv6）。
  - `src`：要转换的二进制格式地址。
  - `dst`：存储转换后的字符串的缓冲区。
  - `size`：缓冲区大小。

- **返回值**：成功返回指向存储在 `dst` 中的字符串的指针，失败返回 `NULL`。

```
#include <arpa/inet.h>

struct in_addr ipv4_addr;
struct in6_addr ipv6_addr;
char ipv4_str[INET_ADDRSTRLEN];
char ipv6_str[INET6_ADDRSTRLEN];

const char* result_ipv4 = inet_ntop(AF_INET, &ipv4_addr, ipv4_str, INET_ADDRSTRLEN);
const char* result_ipv6 = inet_ntop(AF_INET6, &ipv6_addr, ipv6_str, INET6_ADDRSTRLEN);
```

这两个函数是更为通用和安全的选择，特别是在处理IPv6地址时，推荐使用它们。

---

### `socket` 

函数用于创建套接字（socket），通常需要包含 `<sys/socket.h>` 头文件。

```
#include <sys/socket.h>

int socket(int domain, int type, int protocol);
```

- **作用**：创建一个套接字。
- 参数
  - `domain`：协议族，常见的有 `AF_INET`（IPv4）、`AF_INET6`（IPv6）等。
  - `type`：套接字类型，常见的有 `SOCK_STREAM`（流套接字，用于 TCP）和 `SOCK_DGRAM`（数据报套接字，用于 UDP）等。
  - `protocol`：具体使用的协议，通常设置为 0，表示使用默认协议。
- **返回值**：成功返回非负整数作为套接字描述符，失败返回 -1。

---

### `bind` 

函数用于将套接字与特定的地址（包括 IP 地址和端口号）绑定，通常需要包含 `<sys/socket.h>` 头文件。

```
#include <sys/socket.h>

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

- **作用**：将套接字与一个地址（IP 地址和端口号）关联起来，使其能够监听指定的地址或与指定的地址通信。
- 参数
  - `sockfd`：套接字描述符，即通过 `socket` 创建的套接字。
  - `addr`：一个指向 `sockaddr` 结构的指针，其中包含了要绑定的地址信息。
  - `addrlen`：`sockaddr` 结构的长度。
- **返回值**：成功返回 0，失败返回 -1。

`sockaddr` 结构用于表示通用的地址信息，具体的类型是根据协议族的不同而变化的，例如 `sockaddr_in` 用于 IPv4 地址。

常见的错误值有:

- EACCES(error access):绑定的地址是受保护的,只有超级用户可以访问,比如普通用户将socket绑定到0-1023端口时;
- EADDRINUSE(err address in use):被绑定的地址正在使用,比如将socket绑定到一个处于TIME_WAIT状态的socket地址;

### `listen` 

函数用于将套接字转换为监听套接字，使其能够接受连接请求。函数原型如下：

```
int listen(int sockfd, int backlog);
```

- `sockfd`：要监听的套接字描述符。
- `backlog`：连接队列的最大长度，表示同时可以处理的等待连接的客户端数量。

函数返回值：

- 成功时返回0。
- 失败时返回-1，并设置 `errno` 表示具体的错误原因。

`listen` 函数将套接字从 CLOSED 状态转换为 LISTEN 状态，此时套接字可以接受连接请求。`backlog` 参数指定等待连接的队列长度，已经建立连接但还没有被 `accept` 处理的连接请求会被放入队列中。

在调用 `listen` 之前，通常需要先调用 `bind` 将套接字与一个本地地址和端口绑定。

---

### `signal`

 函数用于设置对特定信号的处理方式。以下是对该函数的用法、参数说明和函数返回值的解释：

**用法：**

```
#include <signal.h>

__sighandler_t signal(int __sig, __sighandler_t __handler);
```

**参数说明：**

- `__sig`：表示要处理的信号的编号。这是一个整数参数，代表了特定信号，如 `SIGINT`、`SIGTERM` 等。
- `__handler`：是一个指向函数的指针，表示在接收到指定信号时要调用的处理函数。函数指针的类型通常为 `void (*__handler)(int)`，接受一个整数参数，表示信号的编号。

**函数返回值：**

- 如果成功设置信号处理程序，返回值是先前与该信号相关联的信号处理函数的指针（类型为 `__sighandler_t`）。
- 如果设置信号处理程序失败，返回值是 `SIG_ERR`。

---

### `basename`

 函数通常用于提取路径中的文件名部分，即去除路径中的目录部分，只返回文件名。在C语言中，`basename` 函数的声明如下：

```
#include <libgen.h>

char *basename(char *path);
```

**参数说明：**

- `path`：要提取文件名的路径字符串。

**返回值：**

- `basename` 函数返回一个指向提取出的文件名的指针，该指针指向原始字符串中的相应位置。请注意，`basename` 函数可能会修改输入的路径字符串。

---

### `atoi` 

函数是一个标准库函数，用于将字符串转换为整数（ASCII to Integer）。它的声明如下：

```
#include <stdlib.h>

int atoi(const char *str);
```

**参数说明：**

- `str`：要转换为整数的字符串。

**返回值：**

- `atoi` 函数返回转换后的整数值。如果输入字符串不能正确转换为整数，或者溢出，则返回值为0。

---

1. **LISTEN：** 服务器正在等待传入的连接请求。
2. **SYN-SENT：** 客户端已发送连接请求，等待服务器的确认。
3. **SYN-RECEIVED：** 服务器已接收到连接请求并发送确认，等待客户端的确认。
4. **ESTABLISHED：** 连接已经建立，数据可以在双方之间传输。
5. **FIN-WAIT-1：** 客户端发起关闭连接请求。
6. **FIN-WAIT-2：** 服务器确认了关闭请求，但仍可以发送数据。
7. **CLOSE-WAIT：** 服务器在等待关闭连接请求。
8. **LAST-ACK：** 服务器发送关闭请求，等待客户端的确认。
9. **TIME-WAIT：** 连接已关闭，等待可能延迟的报文。

---

### `accept` 

函数是在使用套接字（socket）编程中，特别是在 TCP 服务器中，用于接受客户端的连接请求的系统调用。在服务器程序中，`accept` 被用于接受来自客户端的连接请求并创建一个新的套接字，用于与客户端进行通信。

以下是 `accept` 函数的一般形式：

```
#include <sys/types.h>
#include <sys/socket.h>

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

**参数说明：**

- `sockfd`：是服务器套接字（通过 `socket` 和 `bind` 创建的套接字），用于监听客户端连接请求。
- `addr`：是一个指向 `struct sockaddr` 结构的指针，用于存储客户端的地址信息。
- `addrlen`：是一个指向 `socklen_t` 类型的指针，指示 `addr` 结构的大小。

**返回值：**

- 如果成功，`accept` 返回一个新的套接字描述符，该描述符用于与客户端进行通信。这个新的套接字是专门用于该连接的，而原始的服务器套接字 (`sockfd`) 仍然继续监听其他连接。
- 如果出现错误，`accept` 返回 `-1`，并设置全局变量 `errno` 来指示错误的类型。

---

### `connect` 

函数是在套接字编程中，特别是在客户端程序中使用的系统调用。它用于建立与远程服务器的连接。当客户端调用 `connect` 时，它向服务器发起连接请求，经过一系列握手过程后，建立了与服务器的通信通道。

以下是 `connect` 函数的一般形式：

```
#include <sys/types.h>
#include <sys/socket.h>

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

**参数说明：**

- `sockfd`：是客户端套接字（通过 `socket` 创建的套接字），用于连接服务器。
- `addr`：是一个指向 `struct sockaddr` 结构的指针，包含了远程服务器的地址信息。
- `addrlen`：是一个指向 `socklen_t` 类型的指针，指示 `addr` 结构的大小。

**返回值：**

- 如果成功，`connect` 返回 `0`。
- 如果出现错误，`connect` 返回 `-1`，并设置全局变量 `errno` 来指示错误的类型。

**常见错误:**

- ECONNREFUSED:目标端口不存在,连接被拒绝;
- ETIMEDOUT:连接超时;

---

### `close` 

函数是一个用于关闭文件描述符的系统调用，也包括套接字描述符。在套接字编程中，它用于关闭已打开的套接字连接。以下是 `close` 函数的一般形式：

```
#include <unistd.h>

int close(int fd);
```

**参数说明：**

- `fd`：是文件描述符（包括套接字描述符）的整数值，它指定要关闭的文件或套接字。

**返回值：**

- 如果成功关闭文件描述符，`close` 返回 `0`。
- 如果出现错误，`close` 返回 `-1`，并设置全局变量 `errno` 来指示错误的类型。

**注意:**close采用计数的方式来管理文件秒描述符,调用close会将对应文件描述符的计数值-1,直到为0时才会真正关闭连接,在多进程中,一次fork系统调用默认将父进程的文件描述符的引用计数+1,必须在父进程和子进程都执行close才能真正关闭文件描述符.

---

### `shutdown` 

函数是用于关闭套接字的一种方式。它允许在不关闭整个套接字的情况下，通过禁用输入、输出或双向通信来进行细粒度的关闭。

以下是 `shutdown` 函数的一般形式：

```
#include <sys/socket.h>

int shutdown(int sockfd, int how);
```

**参数说明：**

- `sockfd`：是套接字描述符，指定要进行关闭操作的套接字。
- `how`：是一个整数，指定关闭的方式，可以是以下值之一：
  - `SHUT_RD`（0）：关闭读取（禁用接收操作）。
  - `SHUT_WR`（1）：关闭写入（禁用发送操作）。
  - `SHUT_RDWR`（2）：关闭读取和写入，即禁用双向通信。

**返回值：**

- 如果成功，`shutdown` 返回 `0`。
- 如果出现错误，`shutdown` 返回 `-1`，并设置全局变量 `errno` 来指示错误的类型。

**注意:**无论如何立即关闭socket.

![image-20231221225656052](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/3.png)

---

在网络编程中，`recv` 和 `send` 是用于在套接字（socket）之间进行数据传输的两个常用函数。这两个函数通常用于TCP和UDP套接字，以在网络上发送和接收数据。

### `recv`

```
ssize_t recv(int sockfd, void *buf, size_t len, int flags);
```

- `sockfd`：套接字文件描述符，表示数据的接收端。
- `buf`：存放接收数据的缓冲区的指针。
- `len`：缓冲区的长度，即能够接收的最大字节数。
- `flags`：可选的标志，通常为0。

`recv` 函数用于从已连接套接字（TCP）或者无连接套接字（UDP）中接收数据。函数返回接收到的字节数，如果发生错误则返回-1，如果对方关闭连接则返回0。

### `send`

```
ssize_t send(int sockfd, const void *buf, size_t len, int flags);
```

- `sockfd`：套接字文件描述符，表示数据的发送端。
- `buf`：存放要发送数据的缓冲区的指针。
- `len`：要发送的字节数。
- `flags`：可选的标志，通常为0。

`send` 函数用于向已连接套接字（TCP）或者无连接套接字（UDP）中发送数据。函数返回实际发送的字节数，如果发生错误则返回-1。

这两个函数在实际使用时，需要注意以下几点：

- **返回值：** `recv` 和 `send` 的返回值通常是实际接收或发送的字节数，但并不一定等于 `len`。因此，调用者应该检查返回值以确保所有数据都已经被处理。
- **阻塞和非阻塞：** 这两个函数默认是阻塞的，即在没有数据可读或者无法发送数据的情况下，它们会阻塞等待。可以通过设置套接字为非阻塞模式来改变这种行为。
- **错误处理：** 当 `recv` 或 `send` 返回-1 时，可以通过调用 `errno` 函数获取具体的错误信息。
- **标志参数（flags）：** 通常情况下，这个参数可以设置为0。在一些特殊情况下，可以使用不同的标志来控制接收和发送的行为，比如 `MSG_WAITALL` 可以要求 `recv` 在接收到指定长度的数据之前一直等待。

---

### `memset`

 函数是C语言中的一个标准库函数，用于设置一段内存块的值。其声明如下：

```
#include <string.h>

void *memset(void *s, int c, size_t n);
```

- `s` 是指向要设置值的内存块的指针。
- `c` 是要设置的值，以整数形式传入。通常是 `unsigned char` 类型的值，因为 `memset` 操作是按字节进行的。
- `n` 是要设置的字节数。

`memset` 函数将内存块的前 `n` 个字节设置为指定的值 `c`。返回值是指向内存块的指针。

---

`recvfrom` 和 `sendto` 是用于在UDP套接字上进行数据传输的两个函数，它们通常用于接收和发送数据报（datagrams）。UDP是一种无连接的协议，每个数据报都是一个独立的消息单元。

### `recvfrom`

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
                 struct sockaddr *src_addr, socklen_t *addrlen);
```

- `sockfd`：套接字文件描述符。
- `buf`：用于接收数据的缓冲区。
- `len`：缓冲区长度。
- `flags`：一些可选标志，通常为0。
- `src_addr`：用于存储发送端地址信息的结构体指针。
- `addrlen`：`src_addr` 结构体的长度。

`recvfrom` 用于从指定的UDP套接字接收数据报。它会将接收到的数据放入 `buf` 缓冲区中，并填充 `src_addr` 结构体以存储发送端的地址信息。`addrlen` 会被设置为 `src_addr` 结构体的长度。

返回值是接收到的字节数，如果出错返回-1。

### `sendto`

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen);
```

- `sockfd`：套接字文件描述符。
- `buf`：要发送的数据的缓冲区。
- `len`：要发送的数据的长度。
- `flags`：一些可选标志，通常为0。
- `dest_addr`：目标地址信息的结构体指针。
- `addrlen`：`dest_addr` 结构体的长度。

`sendto` 用于向指定的UDP套接字发送数据报。它会将 `buf` 缓冲区中的数据发送到指定的目标地址，由 `dest_addr` 结构体指定。`addrlen` 是 `dest_addr` 结构体的长度。

返回值是实际发送的字节数，如果出错返回-1。

**注意：**由于UDP不是面向连接的传输，因此接收方必须指明消息的发送方；发送方必须指明消息的接收方。也可以用于面向连接的传输，将最后的地址和地址长度设置为NULL即可。

---

`recvmsg` 和 `sendmsg` 函数是在套接字编程中用于更为灵活的 I/O 操作的函数，它们支持在消息头中携带附加信息，以及在一次调用中进行多个缓冲区的操作。这对于进行更为复杂的数据交换和控制是非常有用的。同时支持TCP和UDP。

### recvmsg

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags);
```

- `sockfd`：套接字文件描述符。
- `msg`：指向 `msghdr` 结构体的指针，其中包含了用于接收数据的缓冲区信息、控制信息等。
- `flags`：一些可选标志，通常为0。

`recvmsg` 函数用于接收消息，并允许将接收到的数据存储在多个缓冲区中。`msg` 结构体中包含了多个成员，其中的 `msg_iov` 指向一个 `iovec` 结构体数组，每个 `iovec` 结构体定义了一个缓冲区的起始地址和长度。`msg_control` 和 `msg_controllen` 可用于接收辅助数据（control data）。

返回值是接收到的字节数，如果出错返回-1。

### sendmsg

```
#include <sys/types.h>
#include <sys/socket.h>

ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags);
```

- `sockfd`：套接字文件描述符。
- `msg`：指向 `msghdr` 结构体的指针，其中包含了用于发送数据的缓冲区信息、控制信息等。
- `flags`：一些可选标志，通常为0。

`sendmsg` 函数用于发送消息，并允许将要发送的数据存储在多个缓冲区中。与 `recvmsg` 类似，`msg` 结构体中包含了多个成员，`msg_iov` 指向一个 `iovec` 结构体数组，`msg_control` 和 `msg_controllen` 可用于发送辅助数据。

返回值是实际发送的字节数，如果出错返回-1。

这两个函数的主要优点是它们能够处理多个缓冲区的数据，以及在消息头中携带控制信息，从而提供更为灵活的数据交换和控制。

`struct msghdr {`
    `void         *msg_name;       // 指向目标地址的指针（可能是sockaddr结构体）`
    `socklen_t     msg_namelen;    // 目标地址的长度`
    `struct iovec *msg_iov;        // 指向iovec结构体数组的指针，用于描述数据缓冲区`
    `size_t        msg_iovlen;     // iovec结构体数组的长度`
    `void         *msg_control;    // 用于辅助数据的指针`
    `size_t        msg_controllen; // 辅助数据的长度`
    `int           msg_flags;      // 消息的标志`
`};`

- `msg_name`：指向目标地址的指针，通常是一个指向 `sockaddr` 结构体的指针，用于指定消息的目标地址。在 `recvmsg` 中，可以通过设置 `msg_name` 来接收消息的发送地址，在 `sendmsg` 中可以通过设置 `msg_name` 来指定消息的目标地址。
- `msg_namelen`：`msg_name` 指向的目标地址的长度。
- `msg_iov`：指向 `iovec` 结构体数组的指针，用于描述消息中的数据缓冲区。

`struct iovec {`
    `void  *iov_base; // 数据缓冲区的起始地址`
    `size_t iov_len;  // 数据缓冲区的长度`
`};`

- `msg_iovlen`：`msg_iov` 数组的长度，即消息中包含的数据缓冲区的个数。
- `msg_control`：指向辅助数据的指针，用于传递一些控制信息。通常用于在消息头中携带辅助数据，比如发送或接收文件描述符。
- `msg_controllen`：`msg_control` 指向的辅助数据的长度。
- `msg_flags`：消息的标志，表示消息的状态或其他属性。

---

### `sockatmark` 

函数用于确定套接字当前的读写位置是否处于带外标记（out-of-band mark）。带外标记是在流式套接字（如TCP套接字）中的一种机制，用于指示接下来的数据是否是带外数据（Out-of-Band data）。

带外数据是指在正常数据流之外的、具有优先级的数据。在TCP套接字中，带外数据通常用于发送紧急消息或通知，而且带外数据的处理通常是按照发送的顺序，不依赖于正常数据流的顺序。

```
#include <sys/socket.h>

int sockatmark(int sockfd);
```

- `sockfd`：套接字文件描述符。

**返回值：**

- 如果当前套接字读写位置处于带外标记，则返回1。
- 如果不在带外标记上，则返回0。
- 如果出现错误，则返回-1，并设置 `errno`。

**工作原理：**

`sockatmark` 函数检查当前套接字的读写位置，确定是否处于带外标记上。如果是，返回1，否则返回0。带外标记通常用于指示下一个要接收或发送的数据是否为带外数据。

---

`getsockname` 和 `getpeername` 是用于获取套接字地址信息的两个函数，它们通常用于在网络编程中获取本地套接字和对端套接字的地址信息。

### `getsockname`

```
#include <sys/socket.h>

int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

- `sockfd`：套接字文件描述符。
- `addr`：指向保存本地地址信息的 `sockaddr` 结构体的指针。
- `addrlen`：`addr` 结构体的长度，同时也用于返回实际的地址长度。

`getsockname` 用于获取与指定套接字关联的本地地址信息。调用成功时返回0，失败时返回-1，并设置 `errno`。

### `getpeername`

```
#include <sys/socket.h>

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
```

- `sockfd`：套接字文件描述符。
- `addr`：指向保存对端地址信息的 `sockaddr` 结构体的指针。
- `addrlen`：`addr` 结构体的长度，同时也用于返回实际的地址长度。

`getpeername` 用于获取与指定套接字关联的对端地址信息。调用成功时返回0，失败时返回-1，并设置 `errno`。

**工作原理：**

- `getsockname` 和 `getpeername` 函数通过传递一个指向 `sockaddr` 结构体的指针来获取套接字的地址信息。这个结构体可以是 `sockaddr_in`（IPv4）或 `sockaddr_in6`（IPv6）等，具体取决于套接字的地址族。
- `addrlen` 参数在调用前需要设置为传入的缓冲区的长度。在调用完成后，`addrlen` 会被设置为实际的地址长度。

---

`getsockopt` 和 `setsockopt` 是用于获取和设置套接字选项的两个函数，它们提供了对套接字行为进行配置和查询的能力。

### `getsockopt`

```
#include <sys/socket.h>

int getsockopt(int sockfd, int level, int optname, void *optval, socklen_t *optlen);
```

- `sockfd`：套接字文件描述符。
- `level`：选项所在的协议层。
- `optname`：选项的名称。
- `optval`：用于存储选项值的缓冲区。
- `optlen`：`optval` 缓冲区的长度。

`getsockopt` 用于获取指定套接字的选项值。调用成功时返回0，失败时返回-1，并设置 `errno`。

### `setsockopt`

```
#include <sys/socket.h>

int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
```

- `sockfd`：套接字文件描述符。
- `level`：选项所在的协议层。
- `optname`：选项的名称。
- `optval`：指向包含要设置的选项值的缓冲区。
- `optlen`：`optval` 缓冲区的长度。

`setsockopt` 用于设置指定套接字的选项值。调用成功时返回0，失败时返回-1，并设置 `errno`。

**工作原理：**

- `level` 参数指定了选项所在的协议层，常见的值包括 `SOL_SOCKET`（通用套接字选项）、`IPPROTO_TCP`（TCP选项）、`IPPROTO_IP`（IPv4选项）等。
- `optname` 参数指定了具体的选项名称，比如在 `SOL_SOCKET` 层，可以使用 `SO_REUSEADDR` 选项来允许重用本地地址。
- `optval` 参数是一个指向包含选项值的缓冲区的指针。
- `optlen` 参数是 `optval` 缓冲区的长度。

![image-20231222101902472](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/4.png)

![image-20231222102106421](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/5.png)

![image-20231222102116409](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/6.png)

---

### `linger`

```
struct linger {
    int l_onoff;    // 非零表示启用linger，零表示禁用linger
    int l_linger;   // 在linger启用的情况下，设置超时时间（秒）
};
```

- `l_onoff`：非零值表示启用 `linger`，零值表示禁用 `linger`。当 `linger` 启用时，`l_linger` 的值将变得有效。
- `l_linger`：在启用 `linger` 的情况下，它指定了超时时间（秒）。如果 `linger` 启用且 `l_linger` 的值大于零，则 `close` 操作会等待尽可能长的时间，直到发送缓冲区中的数据被发送或超时。

![image-20231222110906750](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/7.png)

---

`gethostbyname` 和 `gethostbyaddr` 是两个用于获取主机信息的函数，它们属于网络编程中的 Berkeley 套接字 API。

### `gethostbyname` 

```
#include <netdb.h>

struct hostent *gethostbyname(const char *name);
```

- **参数：**
  - `name`：主机名或 IP 地址的字符串表示。
- **返回值：**
  - 成功时返回一个指向 `hostent` 结构体的指针，该结构体包含了与主机名相关的信息，包括 IP 地址列表等。
  - 失败时返回 `NULL`，并设置 `h_errno` 变量以指示错误类型。
- **工作原理：**
  - `gethostbyname` 函数通过主机名获取主机信息。它会查询系统的主机名解析服务，如 `/etc/hosts` 文件或 DNS 服务器，以获取与给定主机名相关的信息。

### `gethostbyaddr` 

```
#include <netdb.h>

struct hostent *gethostbyaddr(const void *addr, socklen_t len, int type);
```

- **参数：**
  - `addr`：指向包含 IP 地址信息的缓冲区的指针。
  - `len`：`addr` 缓冲区的长度。
  - `type`：地址类型，通常为 `AF_INET`（IPv4）或 `AF_INET6`（IPv6）。
- **返回值：**
  - 成功时返回一个指向 `hostent` 结构体的指针，该结构体包含了与给定 IP 地址相关的信息，包括主机名等。
  - 失败时返回 `NULL`，并设置 `h_errno` 变量以指示错误类型。
- **工作原理：**
  - `gethostbyaddr` 函数通过 IP 地址获取主机信息。它会查询系统的反向 DNS 解析服务，从而获取与给定 IP 地址相关的信息。

---

### `struct hostent` 

是用于存储主机信息的结构体，包含了与主机相关的各种信息。这个结构体通常由 `gethostbyname` 和 `gethostbyaddr` 这两个函数返回，用于表示主机的名称、别名、地址类型以及地址列表等信息。

```
struct hostent {
    char  *h_name;           // 官方主机名
    char  **h_aliases;       // 主机别名列表
    int   h_addrtype;        // 主机地址类型，通常为 AF_INET（IPv4）或 AF_INET6（IPv6）
    int   h_length;          // 地址的字节数
    char  **h_addr_list;     // 主机地址列表
};
```

- `h_name`：官方主机名。
- `h_aliases`：主机别名列表，以 NULL 结尾的字符串数组。
- `h_addrtype`：主机地址类型，通常为 `AF_INET`（IPv4）或 `AF_INET6`（IPv6）。
- `h_length`：地址的字节数，对于 IPv4 地址通常为 4，IPv6 地址通常为 16。
- `h_addr_list`：主机地址列表，以 NULL 结尾的地址数组。

---

`getservbyname` 和 `getservbyport` 是用于获取服务信息的两个函数，它们属于 Berkeley 套接字 API。这两个函数允许通过服务名称或端口号来获取与之相关的服务信息。相应的 `servent` 结构体则用于存储服务的相关信息。

### `getservbyname` 

```
#include <netdb.h>

struct servent *getservbyname(const char *name, const char *proto);
```

- **参数：**
  - `name`：服务名称（比如 "http"）。
  - `proto`：协议名称（比如 "tcp" 或 "udp"）。
- **返回值：**
  - 成功时返回一个指向 `servent` 结构体的指针，该结构体包含了与给定服务名称和协议相关的信息。
  - 失败时返回 `NULL`，并设置 `h_errno` 变量以指示错误类型。
- **工作原理：**
  - `getservbyname` 函数通过服务名称和协议名称获取服务信息。它会查询系统服务配置文件（通常是 `/etc/services` 文件），以找到与给定服务名称和协议相关的端口号等信息。

---

### `getservbyport`

```
#include <netdb.h>

struct servent *getservbyport(int port, const char *proto);
```

- **参数：**
  - `port`：端口号。
  - `proto`：协议名称（比如 "tcp" 或 "udp"）。
- **返回值：**
  - 成功时返回一个指向 `servent` 结构体的指针，该结构体包含了与给定端口号和协议相关的服务信息。
  - 失败时返回 `NULL`，并设置 `h_errno` 变量以指示错误类型。
- **工作原理：**
  - `getservbyport` 函数通过端口号和协议名称获取服务信息。它会查询系统服务配置文件，以找到与给定端口号和协议相关的服务名称等信息。

---

### `servent`

```
struct servent {
    char  *s_name;     // 服务名称
    char  **s_aliases; // 服务别名列表
    int   s_port;      // 端口号
    char  *s_proto;    // 协议名称
};
```

- `s_name`：服务名称。
- `s_aliases`：服务别名列表，以 NULL 结尾的字符串数组。
- `s_port`：端口号（网络字节序）。
- `s_proto`：协议名称。

---

`getaddrinfo` 函数和 `addrinfo` 结构体是用于实现协议无关的地址和服务解析的一对 API，属于 Berkeley 套接字 API 的一部分。这对 API 提供了更灵活、更通用的方法，用于获取主机名、服务名等信息，以便进行套接字编程。

### `getaddrinfo` 

```
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getaddrinfo(const char *node, const char *service,
                const struct addrinfo *hints, struct addrinfo **res);
```

- **参数：**
  - `node`：主机名或 IP 地址的字符串表示，也可以为 `NULL`。
  - `service`：服务名或端口号的字符串表示。
  - `hints`：一个指向 `addrinfo` 结构体的指针，用于提供关于期望结果的提示。可以为 `NULL`。
  - `res`：一个指向 `addrinfo` 结构体链表的指针，用于存储函数返回的结果。
- **返回值：**
  - 函数成功返回 0。
  - 失败时返回非零值，并设置 `errno` 变量以指示错误类型。
- **工作原理：**
  - `getaddrinfo` 函数通过主机名、服务名等信息，生成一个或多个 `addrinfo` 结构体，其中包含了主机地址、端口号等信息。
  - 这个函数可以用于同时支持 IPv4 和 IPv6，也可以用于获取套接字的各种配置信息。

```
struct addrinfo {
    int              ai_flags;     // 标志位，用于影响操作的行为
    int              ai_family;    // 地址族，比如 AF_INET（IPv4）或 AF_INET6（IPv6）
    int              ai_socktype;  // 套接字类型，比如 SOCK_STREAM（流套接字）或 SOCK_DGRAM（数据报套接字）
    int              ai_protocol;  // 协议类型，比如 IPPROTO_TCP（TCP）或 IPPROTO_UDP（UDP）
    socklen_t        ai_addrlen;   // 地址长度
    struct sockaddr *ai_addr;      // 指向 sockaddr 结构体的指针，包含实际的主机地址和端口号
    char            *ai_canonname; // 官方主机名的规范名称

    struct addrinfo *ai_next;      // 指向链表中下一个 addrinfo 结构体的指针

};
```

- `ai_flags`：标志位，用于影响操作的行为。
- `ai_family`：地址族，例如 `AF_INET`（IPv4）或 `AF_INET6`（IPv6）。
- `ai_socktype`：套接字类型，例如 `SOCK_STREAM`（流套接字）或 `SOCK_DGRAM`（数据报套接字）。
- `ai_protocol`：协议类型，例如 `IPPROTO_TCP`（TCP）或 `IPPROTO_UDP`（UDP）。
- `ai_addrlen`：地址长度。
- `ai_addr`：指向 `sockaddr` 结构体的指针，包含实际的主机地址和端口号。
- `ai_canonname`：官方主机名的规范名称。
- `ai_next`：指向链表中下一个 `addrinfo` 结构体的指针。

![image-20231222130846475](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8.png)

---

### `getnameinfo`

 函数是 Berkeley 套接字 API 中的一个函数，用于将套接字地址转换为主机名和服务名。它提供了一种方便的方式，将网络地址（IP 地址和端口号）转换为对应的主机名和服务名，从而帮助应用程序在网络编程中更好地理解和呈现地址信息。

以下是 `getnameinfo` 函数的声明：

```
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, size_t hostlen,
                char *service, size_t servicelen, int flags);
```

- **参数：**
  - `sa`：指向 `sockaddr` 结构体的指针，包含要转换的套接字地址。
  - `salen`：`sa` 结构体的长度。
  - `host`：用于存储主机名的缓冲区。
  - `hostlen`：`host` 缓冲区的长度。
  - `service`：用于存储服务名的缓冲区。
  - `servicelen`：`service` 缓冲区的长度。
  - `flags`：标志位，用于控制转换的行为。
- **返回值：**
  - 函数成功返回 0。
  - 失败时返回非零值，并设置 `errno` 变量以指示错误类型。
- **工作原理：**
  - `getnameinfo` 函数根据提供的套接字地址，将主机名和服务名转换为字符串形式，存储在指定的缓冲区中。
  - 可以使用 `flags` 参数来控制转换的行为，例如，使用 `NI_NUMERICHOST` 和 `NI_NUMERICSERV` 标志可以强制返回数值格式的主机名和服务名。

![image-20231222131042647](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/9.png)

![image-20231222131102745](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/10.png)

---

`gai_strerror` 是用于获取地址信息错误的函数，它属于与网络编程相关的 POSIX 标准函数。这个函数通常与 `getaddrinfo`、`getnameinfo` 等函数一起使用，以获取这些函数产生的错误的描述信息。

以下是 `gai_strerror` 函数的简要解释：

### `gai_strerror`

```
#include <netdb.h>

const char *gai_strerror(int errcode);
```

- **参数：**
  - `errcode`：`getaddrinfo` 或 `getnameinfo` 等函数返回的错误码。
- **返回值：**
  - 返回一个指向描述错误信息的字符串的指针。
- **工作原理：**
  - `gai_strerror` 函数接受一个错误码作为参数，返回与之对应的地址信息错误的描述字符串。



