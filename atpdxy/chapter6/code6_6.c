#include <fcntl.h>

// 将输入的文件描述符fd设置为非阻塞的作用是使相应的I/O操作在没有数据可读或可写时不回阻塞当前进程
// 而是立即返回一个错误或特殊值，从而在等待I/O操作完成时继续执行其他任务，不必一直等待下去。
//
// 适用场景：
//      异步操作：发起一个I/O后立即返回，不必等待操作完成，提高程序的并发性，允许同时执行多个任务；
//      避免死锁：一个文件描述符被设置为阻塞模式，当数据不可用或缓冲区已满时，读写操作一直阻塞，非阻塞模式可以解决；
//      处理非阻塞事件：允许程序轮询文件描述符以检查是否有数据可用或是否可以写入，常与事件驱动的编程模型结合使用，例如网络编程
int setNoBlocking(int fd){
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}
