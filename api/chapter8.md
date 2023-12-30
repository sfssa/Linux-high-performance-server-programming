# 8.1 服务器模型

## 8.1.1 C/S模型

![image-20231225101528165](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_1.png)

C/S模型逻辑简单，服务器启动后，首先创建一个或多个监听socket监听客户端请求，调用`bind`函数绑定到某个端口上，之后调用`listen`函数开始监听客户端的请求。客户端通过`connect`与服务器建立连接，由于客户端的请求到来是异步的，服务器需要使用某种I/O模型监听这一事件。下图是使用`select`系统调用实现的。服务器监听到连接请求后，调用`accept`函数接受这个连接，并分配一个逻辑单元来处理客户端的请求，可以是一个子进程或者子线程。将处理结果返回给客户端后，客户端可以继续申请所需资源。服务器在处理某个客户端请求的同时，也会不断监听其他客户端的连接，这是通过`select`系统调用实现的。

![image-20231227145840212](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_2.png)

C/S适合资源非常集中的场景，实现起来也很简单，但是问题也很明显，服务器是服务的中心，当访问量很多时，会导致所有的客户端都得到很慢的相应。

## 8.1.2 P2P模型

摒弃以服务器为中心的设计，将所有机器回归对等的地位。

![image-20231225102639091](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_3.png)

优点是在享受服务的同时也在提供服务，缺点也很明显，当用户之间的传输过多时，网络的负载严重。其中a类型的设计主机之间难以发现，因此b类型在a类型基础上提供了发现服务器，方便查找主机。

# 8.2 服务器编程框架

![image-20231225102917692](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_4.png)

下面是对各个模块的功能描述：

![image-20231225102944551](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_5.png)

![image-20231225103028061](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_6.png)

I/O处理单元是服务器处理客户端连接的模块，主要任务如下：

- 等待并接受新的客户端连接请求；
- 接收客户数据并将响应数据发送给客户端。

但是要注意，在逻辑单元中也会涉及到数据的收发。对于服务器机群来说，I/O处理单元是一个专门的接入服务器，实现负载均衡，在所有逻辑服务器中选取负载最小的机器来提供服务。

逻辑单元是一个进程或者线程，主要任务如下：

- 分析处理客户端的请求，将结果返回给I/O处理单元或者直接返回给客户端。

对服务器机群来说，逻辑单元是一台逻辑服务器，服务器有多个逻辑单元，从而为多个客户端提供服务。

网络存储单元可以是数据库、缓存、文件等，也可以是单独的服务器，但并不是必须的。

请求队列是各单元之间的通信方式的抽象，I/O单元接收客户端请求后需要通知逻辑单元进行处理，同样，多个逻辑单元访问一个存储单元时也需要一种机制处理竞态条件，请求队列作为池的一部分。对于服务器机群来说，请求队列是各服务器之间预先建立的、竞态的、永久的TCP连接。目的是提高服务器之间交换数据的效率，避免TCP连接建立的开销。

# 8.3 I/O模型

阻塞：创建的socket默认都是阻塞的，可以通过fcntl函数进行设置为非阻塞。阻塞I/O可能因为无法立即返回被操作系统挂起，直到等待的事件发生。

非阻塞：非阻塞I/O执行的系统调用总是立即返回，不管事件是否已经发生，没有立即发生，系统调用返回-1，和出错的返回值一样，此时需要根据`errno`来进行判断，对于`accept`，`recv`，`send`，事件未发生时`errno`通常被设置为`EAGAIN`（意为再来一次），或者`EWOULDBLOCK`（意为期望阻塞）。对`connect`来说，事件未发生`errno`被设置为`EINPROGRESS`（意为在处理中）。

I/O复用机制：应用程序向内核注册一组事件，内核通过I/O复用将准备好的事件返回通知给应用程序。Linux上常用的I/O复用是：select，poll，epoll_wait。需要说明，I/O复用本身是阻塞的，之所以能提高效率是同时监听一组事件。

同步I/O需要应用程序自己实现I/O操作（将数据从用户缓冲区读取到内核缓冲区或者从内核缓冲区读取到用户缓冲区），而异步I/O由内核来完成（数据在用户缓冲区和内核缓冲区之间的移动是通过内核在后台完成的）。同步I/O向应用程序通知就绪事件；异步I/O向应用程序通知完成事件。

对于同步I/O和异步I/O可以参考食堂打饭，一个人一个人排队需要浪费很多时间，那么同步I/O为每个人分发一个号码，当轮到你时，食堂员工叫号，然后你带着餐盘取到你想吃的饭菜；异步I/O可以理解为将每个人想吃的饭菜打印到一张纸上，和餐盘一起交给食堂员工，食堂员工根据你的纸条将饭菜盛到你的餐盘中，然后通知你可以吃饭了。一个是通知你可以打饭了，一个是通知你打饭完成可以吃了。

![image-20231225111355624](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_7.png)

# 8.4 两种高效的事件处理模式

## 8.4.1 Reactor模式

同步I/O常用Reactor模式。主线程只负责监听文件描述符是否有事件发生，有就通知工作线程。此外，主线程不负责其他任何业务。读写、新连接、处理客户请求都在工作线程完成。

使用同步I/O模型实现Reactor模式：

- 主线程往`epoll`内核事件表中注册`socket`上的读就绪事件；
- 主线程调用`epoll_wait`等待`socket`上有数据可读；
- 当`socket`上有数据可读时，`epoll_wait`通知主线程，将可读事件放到请求队列；
- 唤醒某个工作线程，从`socket`读取数据并进行处理；然后注册`socket`上写就绪事件；
- 主线程调用`epoll_wait`等待`socket`可写；
- 当`socket`可写时，`epoll_wait`通知主线程，主线程将可写事件放入请求队列；
- 唤醒某个工作线程，往`socket`上写入服务器处理客户请求的结果。

![image-20231225134201410](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_8.png)

## 8.4.2 Proactor模式

异步I/O常用Proactor模式。所有的任务都交给主线程和内核来处理，工作现场仅负责业务逻辑。异步I/OProactor模式的流程是：

- 主线程调用`aio_read`向内核注册`socket`上的读完成事件，告知内核的读缓冲区位置，以及读操作完成时如何通知应用程序；
- 主线程继续执行其他逻辑；
- 当`socket`上的数据被读入用户缓冲区后，内核向应用程序发送一个信号，通知应用程序系统可用；
- 应用程序预定义好的信号处理函数选择一个工作线程处理客户请。完成后向内核注册`socket`上的写完成事件，告知内核写缓冲区位置，以及通知应用程序的方式。
- 主线程继续执行其他逻辑；
- 当用户缓冲区的数据写入`socket`后，内核向应用程序发送信号，通知应用程序数据已经发送完毕；
- 应用程序预先定义的好的信号处理函数选择一个线程来善后工作，比如是否关闭`socket`。

![image-20231225135725600](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_9.png)

上图中，读写操作是内核完成的，完成后通过信号来通知处理，因此主线程`epoll_wait`调用监听连接请求事件，而不能用来检测连接`socket`上的读写事件。

## 8.4.3 模拟Proactor模式

使用同步I/O方式模拟Proactor模式，原理是主线程完成数据的读写，完成后通知工作线程，从工作线程的角度来看，它并没有涉及到读写操作，从而以同步I/O来模拟Proactor模式。流程如下：

- 主线程向`epoll`内核注册`socket`读就绪事件；
- 主线程调用`epoll_wait`等待`socket`可读；
- 当`socket`上有数据可读，主线程读取数据并封装成一个请求对象并插入到请求队列中；
- 睡眠的某个工作队列被唤醒，开始读取请求对象并处理客户请求，然后向`epoll`内核注册写就绪事件；
- 主线程调用`epoll_wait`等待`socket`可写；
- 当`socket`可写时，`epoll_wait`通知主线程，主线程往`socket`上写服务器处理客户端请求的结果。

![image-20231225140644423](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_10.png)

# 8.5 两种高效的并发模式

并发编程的目的是为了让计算机同时执行多个任务，如果程序是CPU密集型，并发编程并没有优势，反而由于任务的切换使效率降低，但如果程序时I/O密集型，比如经常读写文件或者访问数据库，由于I/O的速度远没有CPU计算快，所以让程序所阻塞于I/O操作会浪费大量CPU的时间，如果程序有多个线程，则当前被阻塞的线程可以放弃CPU的执行权，交给其他的线程去执行其他任务，从而大大提高CPU利用率。服务器主要有两种并发编程模型：半同步/半异步模式和领导者/追随者模式。

## 8.5.1 半同步/半异步模式

并发模式的同步和异步并不是事件处理中的同步和异步，并发模式的同步指按照代码的序列顺序执行；异步在执行代码时需要在系统事件驱动下来执行，常见的系统事件包括中断和信号。

![image-20231225145913837](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_11.png)

异步模式实时性高，效率更好，适合嵌入式，但编写更加困难，难以调试和扩展，并且不适合大量的并发；而同步模式虽然实时性和效率不如异步，但是编写更加简单。对于服务器这种需要同时满足较好的实时性，又要同时处理多个客户端请求，使用同时采用同步和异步的并发模式，也就是半同步/半异步的方式来实现。

其中同步模式主要用于处理客户端的业务逻辑，而异步模式用于处理I/O事件。异步模式监听到客户请求后，封装成请求对象并插入请求队列，请求队列通知某个工作在同步模式的工作现场读取并处理客户端请求。

![image-20231225150604759](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_12.png)

内部I/O单元通知异步I/O线程有可读事件，异步线程封装读就绪事件为请求对象并插入请求队列中，然后通知同步业务逻辑线程执行任务，任务执行完毕后将写就绪事件封装请求对象插入请求队列，然后异步I/O线程处理写事件，最终交付到内部I/O源。

此外，还有一种变体，半同步/半反应堆模式：

![image-20231225151255932](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_13.png)

异步线程只有一个，也就是主线程，负责监听所有socket事件。如果socket上有可读事件，也就是有新的客户端连接，主线程接受并得到新的socket，然后在epoll内核注册读写事件。如果有读写事件，就将对应的socket插入到请求队列中，所有的工作线程都在请求队列睡眠，当有任务时，通过竞争获得任务的执行权。（上述也可以用主线程来读取和写入数据，从而模拟Proactor模式）

半同步/半反应堆的缺点：

- 主线程和工作线程共享请求队列，无论是添加任务或者取出任务，都需要上锁来保证正确的访问，从而白白浪费CPU时间；
- 每个工作线程只负责处理一个客户端，如果客户端请求很多，那么请求任务会越积越多，客户的请求时间越来越长，如果增加工作线程又会导致线程间切换的开销；

下面是一个相对高效的半同步/半反应堆方式：

![image-20231225152220666](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_14.png)

主线程只负责监听socket，连接socket由工作线程来执行，当有新的连接到来时，主线程接受并将该socket派发给某个工作线程， 此后socket上的一切事件都由工作线程来处理，直到断开连接。派发的最简单方式是向主线程和工作线程的管道中写数据，工作线程检测到管道有可读数据，分析是否是一个新的连接，如果是，就将新的socket上的读写事件注册到自己的epoll内核事件表中。每个线程都维持自己的事件循环，监听各自的事件。

## 8.5.2 领导者/追随者模式

多个工作线程轮流获得事件集合、轮流监听、轮流分发并处理事件。任意时间点有且仅有一个领导者，负责监听I/O事件，其他线程在线程池等待成为领导者，当前领导者检测到I/O事件，先从线程池中选出新的领导者，然后去处理I/O事件，这样原来的领导者处理I/O事件，新的领导者监听新的I/O事件，两者实现并发。

![image-20231225153231877](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_15.png)

**句柄集**

​		表示I/O资源，Linux操作系统下就是一个文件描述符，句柄集顾名思义，多个句柄的集合，用`wait_for_event`监听句柄上的I/O事件，将就绪的事件通知给领导者线程，领导者调用绑定到`Handle`上的事件处理器来处理事件。领导者将`Handle`和事件处理器绑定是通过调用句柄集中的`register_handle`方法实现的

**线程集**

​		所有工作线程的管理者，负责各线程之间的同步以及新的领导者线程的推选。主要有三种状态：

- Leader：负责等待句柄上的I/O事件；
- Processing：线程正在处理I/O事件，领导者检测到I/O事件后，调用`promote_new_leader`来推选新的领导者。也可以指定其他追随者来处理事件，此时领导者身份不变，执行完毕后，没有新的领导者，那么它称为新的领导者，否则转换成追随者；
- Follower：追随者，调用`join`函数等待成为领导者，也可能等待被领导者派发任务。

![image-20231225154321812](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_16.png)

**事件处理器和具体事件处理器**

包括一个或多个回调函数handle_event。这些回调函数用于处理事件对应的业务逻辑。需要提前绑定到某个句柄上，当句柄有事件发生时，领导者执行与之绑定的处理函数。

![image-20231225154505046](https://github.com/sfssa/Linux-high-performance-server-programming/blob/master/static/8_17.png)

# 8.6 提高服务器性能的其他建议

## 8.7.1 池

既然计算机硬件资源相对充裕，那么提高性能的一个很直接的方法就是以时间换空间，也就是用硬件资源换取运行效率，这就是池的概念。池是一组资源的集合，在程序启动之初就创建好并完成初始化，也被称为静态资源分配。当服务器正式运行时，可以直接从池中获取需要的资源而不需要动态分配；当服务器处理完客户端请求后，将资源放回池中，而无需执行系统调用来释放资源。从最终效果来看，池实现了服务器管理资源的应用层，减少了服务器对内核的频繁访问。

池的问题是，无法预先得知服务器需要分配多少个资源，分配的少了依然需要动态分配；分配的多了又会造成资源的消耗。第一种方案是分配足够多的资源，但是任一时刻的客户数量都可能远远没有达到服务器可以支持的最大数量，但是这种浪费一般来说不会对服务器产生问题。另一个方案是预先分配一定的资源，发现不够用时再动态分配一批资源。

**内存池：**常用于socket接收缓冲区和发送缓冲区，对于某些长度有限的客户请求，比如HTTP请求，设置一定大小的缓冲区（5000字节）是合理的，如果超过了这个缓冲区可以选择丢掉数据或者分配更大的空间。

**进程池/线程池：**是实现并发的方法，当我们需要用一个线程或者进程处理客户端请求时，从池中返回一个进程或线程来处理请求，不需要通过`fork`函数或者`pthread_create`函数来创建，处理完请求后则将线程或进程继续放到线程池中。

**连接池：**假设我们需要频繁的访问数据库，如果我们每次都创建连接、访问数据库、释放连接，这样涉及到频繁的创建和销毁连接毫无疑问会导致性能的下降。而通过连接池，提前将连接创建并放到池中，当需要连接时从池中获得，访问结束后将连接放回连接池。

## 8.7.2 数据复制

高性能服务器应该避免不必要的数据复制，特别是发生在用户空间和内核空间之间的代码复制，如果内核可以直接处理数据，从`socket`或文件读取数据，就不要将数据复制到用户态应用程序的缓冲区中，这里说的直接处理指的是不需要用户程序关心数据的内容，不需要对数据进行分析，例如FTP服务器，用户程序只需要确定文件是否存在以及用户是否有足够的权限即可。FTP服务器不需要将数据复制到用户空间的发送缓冲区并调用`send`函数发送，而是可以使用零拷贝函数`sendfile`来直接发送给客户端。

## 8.7.3 上下文切换和锁

并发程序必须考虑上下文切换的问题，也就是进程或线程切换所需要的花销，即使是I/O密集型的服务器，也不应该使用过多的工作线程，否则CPU切换上下文的时间会拉长，CPU执行实际任务的处理时间的比重会变小。因此为每一个客户端建立一个连接是不可取的，半同步/半异步的方式会比较合适，允许一个线程或进程处理多个请求。此外，多线程服务器的一个优点是不同的线程可以运行在不同的CPU上，那么不超过CPU的数量时并不会涉及到上下文切换的开销。

并发程序需要考虑的另一个问题是共享资源的系统保护，锁通常被认为是导致服务器效率低下的原因，它引入的代码不仅不处理任何的业务逻辑，还需要访问内核资源，因此，如果有更好的方案就不要用锁。如果必须要用锁，那么尽可能的减小锁的粒度，比如使用读写锁。当多个线程同时访问一块共享内存的内容时，并不会增加系统的额外开销，只有当某个线程试图去修改共享空间内存的内容时，系统才会必须锁住这块内存。

**用户级锁：**

**`std::mutex` (C++11及以上)：**

- `std::mutex` 是 C++11 引入的标准库提供的用户级互斥锁。
- 它在用户空间实现，不涉及系统调用。
- 通过 `std::lock_guard` 或 `std::unique_lock` 进行锁的自动管理。

```
#include <mutex>

std::mutex myMutex;

void exampleFunction() {
    std::lock_guard<std::mutex> lock(myMutex);
    // 临界区
}
```

**`std::shared_mutex` (C++14及以上)：**

- `std::shared_mutex` 是 C++14 引入的标准库提供的用户级读写锁。
- 它允许多个线程同时读取，但只允许一个线程写入。

```
#include <shared_mutex>

std::shared_mutex mySharedMutex;

void readOperation() {
    std::shared_lock<std::shared_mutex> lock(mySharedMutex);
    // 读取操作
}

void writeOperation() {
    std::unique_lock<std::shared_mutex> lock(mySharedMutex);
    // 写入操作
}
```

**内核级锁：**

**POSIX Threads（pthread）库：**

- POSIX Threads 提供了一套用于多线程编程的API，包括内核级锁。
- `pthread_mutex_t` 是一个内核级互斥锁。

```
#include <pthread.h>

pthread_mutex_t myMutex = PTHREAD_MUTEX_INITIALIZER;

void exampleFunction() {
    pthread_mutex_lock(&myMutex);
    // 临界区
    pthread_mutex_unlock(&myMutex);
}
```

**System V 信号量：**

- System V IPC（Inter-Process Communication）提供了一套机制，其中包括信号量用于同步。
- `sem_t` 是一个内核级信号量。

```
#include <semaphore.h>

sem_t mySemaphore;

void exampleFunction() {
    sem_wait(&mySemaphore);
    // 临界区
    sem_post(&mySemaphore);
}
```