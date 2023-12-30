#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <errno.h>

// 最大同时监听事件数
#define MAX_EVENT_LIMIE 1024
// TCP缓冲区大小
#define TCP_BUFFER_SIZE 512
// UDP缓冲区大小
#define UDP_BUFFER_SIZE 1024

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
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setNonBlocking(fd);
}

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address, port_number\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int ret;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    // 创建UDPsocket并绑定端口
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int udpfd = socket(PF_INET, SOCK_DGRAM, 0);
    assert(udpfd >= 0);
    
    ret = bind(udpfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    epoll_event events[MAX_EVENT_LIMIE];
    int epollfd = epoll_create(5);
    assert(epollfd != -1);

    addFd(epollfd, listenfd);
    addFd(epollfd, udpfd);
    
    while(1) {
        int number = epoll_wait(epollfd, events, MAX_EVENT_LIMIE, -1);
        if(number < 0) {
            printf("epoll_wait failed\n");
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
            } else if(sockfd == udpfd) {
                char buf[UDP_BUFFER_SIZE];
                memset(buf, '\0', UDP_BUFFER_SIZE);
                struct sockaddr_in client_address;
                socklen_t client_addr_len = sizeof(client_address);
                ret = recvfrom(udpfd, buf,UDP_BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_address, &client_addr_len);
                if(ret > 0) {
                    sendto(udpfd, buf, UDP_BUFFER_SIZE - 1, 0, (struct sockaddr*)&client_address, client_addr_len);
                }
            } else if(events[i].events & EPOLLIN) {
                char buf[TCP_BUFFER_SIZE];
                while(1) {
                    memset(buf, '\0', TCP_BUFFER_SIZE);
                    ret = recv(sockfd, buf, TCP_BUFFER_SIZE - 1, 0);
                    if(ret < 0) {
                        if(errno == EAGAIN || errno == EWOULDBLOCK) {
                            break;
                        } 
                        close(sockfd);
                        break;
                    } else if(ret == 0) {
                        close(sockfd);
                    } else {
                        send(sockfd, buf, ret, 0);
                    }
                }
            } else {
                printf("something else happen\n");
            }
        }
    }
    close(listenfd);
    return 0;
}
