#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>

const int MAX_EVENT_NUMBER = 1024;
const int BUFFER_SIZE = 1024;

struct fds {
    int epollfd;
    int sockfd;
};

// 设置文件描述符fd为非阻塞
int setNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

// ET模式加入到内核事件表中并选择是否使用oneshut
void addFd(int epollfd, int fd, bool oneshot) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if(oneshot) {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

// 重置oneshot
void resetOneshot(int epollfd, int fd) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

// 工作线程
void* worker(void* arg) {
    int sockfd = ((fds*)arg)->sockfd;
    int epollfd = ((fds*)arg)->epollfd;

    printf("start new thread to receive data on fd: %d\n", sockfd);
    char buf[BUFFER_SIZE];
    memset(buf, '\0', BUFFER_SIZE);
    // ET模式一次性把数据读完
    while(1) {
        int ret = recv(sockfd, buf, BUFFER_SIZE - 1, 0);
        if(ret == 0) {
            printf("foreign closed the connection\n");
            close(sockfd);
            break;
        } else if(ret < 0) {
            if(errno == EAGAIN) {
                printf("read later\n");
                resetOneshot(epollfd, sockfd);
                break;
            }
        } else {
            printf("get content: %s\n", buf);
            sleep(5);
        }
    }
    printf("end thread recving data on fd: %d\n", sockfd);
    return NULL;
}

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_numeric\n", basename(argv[0]));
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
    assert(socket >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    // listenfd不能设置为oneshot，否则只处理一个连接
    addFd(epollfd, listenfd, false);

    while(1) {
        ret = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(ret < 0) {
            printf("epoll_wait failed\n");
            break;
        }

        for(int i = 0; i < ret; ++i) {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd) {
                struct sockaddr_in clientAddress;
                socklen_t clientAddrLen = sizeof(clientAddress);
                int connfd = accept(listenfd, (struct sockaddr*)&clientAddress, &clientAddrLen);
                assert(ret != -1);
                addFd(epollfd, connfd, true);
            } else if(events[i].events & EPOLLIN) {
                pthread_t thread;
                fds fdsForNewThread;
                fdsForNewThread.sockfd = sockfd;
                fdsForNewThread.epollfd = epollfd;
                pthread_create(&thread, NULL, worker, (void*)&fdsForNewThread);
            } else {
                printf("something else happened\n");
            }
        }
    }
    close(epollfd);
    close(listenfd);
    return 0;
}
