#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <sys/epoll.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>

// 最大同时监听事件数量
#define MAX_EVENT_NUMBER 1024
// 全局管道
static int pipefd[2];

int setNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

void addFd(int epollfd, int fd) {
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
    setNonBlocking(fd);
}

// 信号处理函数
void sigHandler(int sig) {
    int old_errno = errno;
    int msg = sig;
    send(pipefd[1], (char*)&msg, 1, 0);
    errno = old_errno;
}

// 设置信号的处理函数
void addSig(int sig) {
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sigHandler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    struct sockaddr_in address;
    memset(&address, '\0', sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    if(ret == -1) {
        printf("error in bind: %d\n", errno);
        return 1;
    }

    ret = listen(listenfd, 5);
    if(ret == -1) {
        printf("error in listen: %d\n", errno);
        return 1;
    }

    epoll_event events[MAX_EVENT_NUMBER];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);
    addFd(epollfd, listenfd);
    
    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, pipefd);
    assert(ret != -1);
    setNonBlocking(pipefd[1]);
    addFd(epollfd, pipefd[0]);

    // 控制终端挂起
    addSig(SIGHUP);
    // 子进程状态发生改变
    addSig(SIGCHLD);
    // 终止进程
    addSig(SIGTERM);
    // 键盘输入以终止进程
    addSig(SIGINT);
    bool stop_server = false;
    
    while(!stop_server) {
        int number = epoll_wait(epollfd, events, MAX_EVENT_NUMBER, -1);
        if(number == -1 && errno != EINTR) {
            // EINTR是系统调用被中断
            printf("error in epoll_wait: %d\n", errno);
            break;
        }

        for(int i = 0; i < number; ++i) {
            int sockfd = events[i].data.fd;
            if(sockfd == listenfd) {
                struct sockaddr_in client_address;
                socklen_t client_addr_len = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addr_len);
                assert(connfd >= 0);
                addFd(epollfd, connfd);
            } else if(sockfd == pipefd[0] && events[i].events & EPOLLIN) {
                int sig;
                char signals[1024];
                ret = recv(pipefd[0], signals, sizeof(signals), 0);
                if(ret == -1) {
                    continue;
                } else if(ret == 0){
                    continue;
                } else {
                    for(int i = 0; i < ret; ++i) {
                        switch(signals[i]) 
                        {
                            case SIGCHLD:
                            case SIGHUP:  
                            {
                                continue;
                            }
                            case SIGTERM:
                            case SIGINT:
                            {
                                stop_server = true;
                            }
                            
                        }
                    }
                }
            } else {
                printf("something else happened\n");
            }
        }
    }
    printf("close fds\n");
    close(listenfd);
    close(epollfd);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}
