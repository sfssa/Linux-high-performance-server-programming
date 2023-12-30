#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>

const int MAX_EVENT_NUMBER = 1024;
const int BUFFER_SIZE = 10;

// 将fd设置非阻塞
int setNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// 将事件fd添加到内核事件表epollfd上，并根据enable_et选择是否启用ET模式
void addFd(int epollfd, int fd, bool enable_et) {
    // 创建epoll事件
    epoll_event event;
    // 对fd进行赋值
    event.data.fd = fd;
    // 设置事件要监听的事件
    event.events = EPOLLIN;
    // 判断是否启动ET模式
    if(enable_et) {
        event.events |= EPOLLET;
    }
    // 添加到内核事件表
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    // 设置非阻塞
    setNonBlocking(fd);
}

// LT 模式工作流程
// events-所有就绪的事件集合；number-就绪事件的总数；epollfd-内核事件表的唯一标识；listenfd-监听文件描述符（意味着新的连接）
void LTMode(epoll_event* events, int number, int epollfd, int listenfd) {
   char buf[BUFFER_SIZE];
   for(int i = 0; i < number; ++i) {
       // 新的连接
       int sockfd = events[i].data.fd;
       if(sockfd == listenfd) {
            struct sockaddr_in client;
            socklen_t clientLen = sizeof(client);
            int connfd = accept(listenfd, (struct sockaddr*)&client, &clientLen);
            // 使用LT模式
            addFd(epollfd, connfd, false);            
       } else if(events[i].events & EPOLLIN) {
            // 只要还有未读出的数据就会触发
            printf("event trigger once\n");
            memset(buf, '\0', BUFFER_SIZE);
            int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
            if(ret <= 0) {
                close(sockfd);
                continue;
            }
            printf("get %d bytes of content: %s\n", ret, buf);
       } else {
           printf("something else happened\n");
       }
    }
}

// ET模式工作流程
void ETMode(epoll_event* events, int number, int epollfd, int listenfd) {
    char buf[BUFFER_SIZE];
    for(int i = 0; i < number; ++i) {
        int sockfd = events[i].data.fd;
        if(sockfd == listenfd) {
            struct sockaddr_in client;
            socklen_t clientLen = sizeof(client);
            int connfd = accept(listenfd, (struct sockaddr*)&client, &clientLen);
            // 使用ET模式
            addFd(epollfd, connfd, true);
        } else if(events[i].events & EPOLLIN) {
            printf("event trigger once\n");
            while(1) {
                memset(buf, '\0', BUFFER_SIZE);
                int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
                if(ret < 0) {
                    // 当前没有更多数据可读
                    if(errno == EAGAIN || errno == EWOULDBLOCK) {
                        printf("read later\n");
                        break;
                    }
                    close(sockfd);
                    break;
                } else if(ret == 0) {
                    // 对端已经关闭
                    close(sockfd);
                } else {
                    printf("get %d bytes of content: %s\n", ret, buf);
                }
            }
        } else {
            printf("something else happened\n");
        }
    }
}

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    
    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    // 设置ET模式一方面减少频繁的触发，减少不必要的触发可以提高效率
    // 另一方面ET模式更加接近底层的通知机制
    addFd(epollfd, listenfd, true);

    while(1) {
       ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
       if(ret < 0) {
           printf("epoll failed\n");
           break;
       }
       // LTMode(events, MAX_EVENT_NUMBER, epollfd, listenfd);
       ETMode(events, MAX_EVENT_NUMBER, epollfd, listenfd);
    }
    close(epollfd);
    close(listenfd);
    return 0;
}
