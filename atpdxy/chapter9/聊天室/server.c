// 聊天室服务器
#define _GNU_SOURCE 1

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// 最大在线用户
#define USER_LIMIT 5

// 缓冲区大小
#define BUFFER_SIZE 64

// 文件描述符数量限制
#define FD_LIMIT 65535

// 客户端数据结构：客户端地址-待写到客户端的数据-从客户端读取的数据
struct ClientData {
    sockaddr_in address;
    char* write_buf;
    char read_buf[BUFFER_SIZE];
};

int setNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number\n",basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int ret = 0;
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

    // 分配FD_LIMIT个用户数据，以空间换时间
    ClientData* users = new ClientData[FD_LIMIT];
    pollfd fds[USER_LIMIT + 1];
    int user_count = 0;
    for(int i = 1; i <= USER_LIMIT; ++i) {
        fds[i].fd = -1;
        fds[i].events = 0;
    }
    fds[0].fd = listenfd;
    fds[0].events = POLLIN | POLLERR;
    fds[0].revents = 0;
    
    while(1) {
        ret = poll(fds, user_count + 1, -1);
        if(ret < 0) {
            printf("poll error: %d\n", errno);
            break;
        }

        for(int i = 0; i < user_count + 1; ++i) {
            if(fds[i].fd == listenfd && fds[i].revents & POLLIN) {
                struct sockaddr_in client_address;
                socklen_t client_addr_length = sizeof(client_address);
                int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addr_length);
                if(connfd < 0) {
                    printf("accept error： %d\n", errno);
                    continue;
                }
                // 判断最大在线人数
                if(user_count >= USER_LIMIT) {
                    const char* info = "to many users, please wait for a minute\n";
                    printf("%s", info);
                    send(connfd, info, strlen(info), 0);
                    close(connfd);
                    continue;
                }
                ++user_count;
                users[connfd].address = client_address;
                setNonBlocking(connfd);
                fds[user_count].fd = connfd;
                fds[user_count].events = POLLIN | POLLERR | POLLRDHUP;
                fds[user_count].revents = 0;
                printf("comes a new users, now have %d users\n", user_count);
            } else if(fds[i].revents & POLLERR) {
                printf("get an error from %d\n", fds[i].fd);
                char errors[100];
                memset(errors, '\0', 100);
                socklen_t length = sizeof(errors);
                if(getsockopt(fds[i].fd, SOL_SOCKET, SO_ERROR, &errors, &length) < 0) {
                    printf("get socket option failed\n");
                }
                continue;
            } else if(fds[i].revents & POLLRDHUP) {
                users[fds[i].fd] = users[fds[user_count].fd];
                close(fds[i].fd);
                fds[i] = fds[user_count];
                --i;
                --user_count;
                printf("a client left\n");
            } else if(fds[i].revents & POLLIN) {
                int connfd = fds[i].fd;
                memset(users[connfd].read_buf, '\0', BUFFER_SIZE);
                ret = recv(connfd, users[connfd].read_buf, BUFFER_SIZE - 1, 0);
                printf("get %d bytes of client data %s from %d\n", ret, users[connfd].read_buf, connfd);
                if(ret < 0) {
                    if(errno != EAGAIN) {
                        close(connfd);
                        users[fds[i].fd] = users[fds[user_count].fd];
                        fds[i] = fds[user_count];
                        --user_count;
                        --i;
                    }
                } else if(ret == 0) {
                } else {
                    // 其他客户端不能发送数据到客户端
                    for(int j = 1; j <= user_count; ++j) {
                        if(fds[j].fd == connfd) {
                            continue;
                        }

                        // 将其他socket取消监听POLLIN事件；开始监听POLLOUT事件，并初始化待写的数据
                        fds[j].events |= ~POLLIN;
                        fds[j].events |= POLLOUT;
                        users[fds[j].fd].write_buf = users[connfd].read_buf;
                    }
                }
            } else if(fds[i].revents & POLLOUT) {
                // 可以向该服务器发送数据
                int connfd = fds[i].fd;
                if(!users[connfd].write_buf) {
                    continue;
                }
                ret = send(connfd, users[connfd].write_buf, strlen(users[connfd].write_buf), 0);
                users[connfd].write_buf = NULL;
                // 该客户端可以发送数据了
                fds[i].events |= POLLIN;
                fds[i].events |= ~POLLOUT;  
            }
        }
    }
    delete[] users;
    close(listenfd);
    return 0;
}
