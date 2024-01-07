#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <signal.h>
#include "./code11_2.h"

// 最大文件描述符，用时间换空间
#define FD_LIMIT 65535
// 最大同时监听事件
#define MAX_EVENT_NUMBER 1024
// 超市时间，设置为5秒钟
#define TIMESLOT 5
// 通知主循环的管道
static int pipefd[2];
// 静态定时器链表
static sort_timer_lst timer_list;
// 全局内核事件表
static int epollfd = 0;

// 设置为非阻塞
int setnonblocking(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 向内核事件表中添加事件
void addfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

// 通过管道发送信号
void sig_handler(int sig)
{
    // 保存旧的错误码
    int save_errno = errno;
    // 拷贝信号
    int msg = sig;
    // 发送信号
    send(pipefd[1], (char*)&msg, 1, 0);
    // 恢复错误码
    errno = save_errno;
}

// 为信号添加处理函数
void addsig(int sig)
{
    // 新建信号事件结构体
    struct sigaction sa;
    // 清空结构体
    memset(&sa, '\0', sizeof(sa));
    // 设置结构体处理函数
    sa.sa_handler = sig_handler;
    // 设置结构体标志
    sa.sa_flags |= SA_RESTART;
    // 设置结构体掩码均为1
    sigfillset(&sa.sa_mask);
    // 注册信号事件
    assert(sigaction(sig, &sa, NULL) != -1);
}

// 定时处理任务，调用tick函数，执行到期的定时器
void timer_handler()
{
    timer_list.tick();
    // 每间隔5秒钟触发一个SIGALRM信号
    alarm(TIMESLOT);
}

// 定时器回调函数，删除非活动连接socket上的注册事件并关闭文件描述符
void cb_func(client_data* user_data)
{
    // 从内核事件表中删除文件描述符
    epoll_ctl(epollfd, EPOLL_CTL_DEL, user_data->sockfd, 0);
    assert(user_data);
    // 关闭文件描述符
    close(user_data->sockfd);
    printf("close fd %d\n", user_data->sockfd);
}

int main(int argc, char* argv[])
{
    // 判断输入是否正确
    if(argc <= 2)
    {
        printf("usage: %s ip_address, port_number\n", basename(argv[0]));
        return 1;
    }
    // 获得IP和端口号
    const char* ip = argv[1];
    const int port = atoi(argv[2]);
    // 创建服务器地址结构体
    struct sockaddr_in address;
    int ret;
    // 清空结构体
    memset(&address, '\0', sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    // 创建文件描述符
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    // 绑定地址信息
    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    // 监听
    ret = listen(listenfd, 5);
    assert(ret != -1);

    // 创建内核事件表以及最大内核事件
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    epoll_event events[MAX_EVENT_NUMBER];
    addfd(epollfd, listenfd);

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0 ,pipefd);
    assert(ret != -1);

    setnonblocking(pipefd[1]);
    addfd(epollfd, pipefd[0]);

    addsig(SIGALRM);
    addsig(SIGTERM);
    bool stop_server = false;

    client_data* users = new client_data[FD_LIMIT];
    bool timeout = false;
    alarm(TIMESLOT);

    while(!stop_server)
    {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        // 返回值小于0且错误不是EINTR，epoll_wait出错
        if(number < 0 && errno != EINTR)
        {
            printf("epoll_wait failer\n");
            break;
        }
        // 循环处理所有被触发的事件
        for(int i = 0; i < number; ++i)
        {
            int sockfd = events[i].data.fd;
            // 处理新的客户端连接
            if(sockfd == listenfd)
            {
                struct sockaddr_in client_address;
                socklen_t client_addrlength = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addrlength);
                addfd(epollfd, connfd);
                users[connfd].address = client_address;
                users[connfd].sockfd = connfd;

                // 创建定时器，设置回调函数以及超市时间，绑定定时器与用户数据，最后插入到定时链表中
                util_timer* timer = new util_timer;
                timer->user_data = &users[connfd];
                timer->cb_func = cb_func;
                time_t cur = time(NULL);
                timer->expire = cur + 3 * TIMESLOT;
                users[connfd].timer = timer;
                timer_list.add_timer(timer);
            }
            // 处理信号
            else if(sockfd == pipefd[0] && events[i].events & EPOLLIN)
            {
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if(ret == -1)
                {
                    continue;
                }
                else if(ret == 0)
                {
                    continue;
                }
                else 
                {
                    for(int i = 0; i < ret; ++i)
                    {
                        switch(signals[i])
                        {
                        case SIGALRM:
                        {
                            timeout = true;
                            break;
                        }
                        case SIGTERM:
                        {
                            stop_server = true;
                        }
                        }
                    }
                }
            }
            // 处理客户端传输数据
            else if(events[i].events & EPOLLIN)
            {
                memset(users[sockfd].buf, '\0', BUFFER_SIZE);
                ret = recv(sockfd, users[sockfd].buf, BUFFER_SIZE - 1, 0);
                printf("get %d bytes of client data %s from %d\n", ret, users[sockfd].buf, sockfd);
                util_timer* timer = users[sockfd].timer;
                // 发生读错误，关闭连接并移除对应的定时器
                if(ret < 0)
                {
                    if(errno != EAGAIN)
                    {
                        // 关闭连接并删除定时器
                        cb_func(&users[sockfd]);
                        if(timer)
                        {
                            timer_list.del_timer(timer);
                        }
                    }
                }
                else if(ret == 0)
                {
                    // 对方关闭连接， 服务器关闭连接并删除定时器
                    cb_func(&users[sockfd]);
                    if(timer)
                    {
                        timer_list.del_timer(timer);
                    }
                }
                else 
                {
                    // 有数据可读，调整定时器
                    if(timer)
                    {
                        time_t cur = time(NULL);
                        timer->expire = cur + 3 * TIMESLOT;
                        printf("adjust timer once\n");
                        timer_list.addjust_timer(timer);
                    }
                }
            }
            else
            {
                printf("something else\n");
            }
        }
        // 最后处理定时事件,I/O事件有更高的优先级，但是会导致定时任务不能精确的按照预期的时间执行
        if(timeout)
        {
            timer_handler();
            timeout = false;
        }
    }

    close(listenfd);
    close(pipefd[0]);
    close(pipefd[1]);
    delete[] users;
    return 0;
}
