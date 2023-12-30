// 聊天室客户端

// 告诉编译器包含GNU扩展的特性，在C语言种，这种宏定义用于启用一些特定于GNU编译器的功能和扩展 
#define _GNU_SOURCE 1

#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <poll.h>

#define BUFFER_SIZE 64

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number name\n", basename(argv[0]));
        return 1;
    }    

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    struct sockaddr_in server_address;
    bzero(&server_address, sizeof(server_address));
    server_address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &server_address.sin_addr);
    server_address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    if(connect(sockfd, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("connect failed: %d\n", errno);
        close(sockfd);
        return 1;
    } 

    // 创建结构体数组
    pollfd fds[2];
    // 监听的文件描述符为标准输入
    fds[0].fd = 0;
    // 监听的事件是输入
    fds[0].events = POLLIN;
    // 等待内核写入发生的事件
    fds[0].revents = 0;
    // 监听的文件描述符是sockfd
    fds[1].fd = sockfd;
    // 监听的事件是输入或者对端挂断
    fds[1].events = POLLIN | POLLRDHUP;
    // 等待内核填入发生的事件
    fds[1].revents = 0;

    // 对话缓冲区大小
    char read_buf[BUFFER_SIZE];

    // 创建管道
    int pipefd[2];
    int ret = pipe(pipefd);
    assert(ret != -1);

    while(1) {
        // 阻塞，直到有事件发生
        ret = poll(fds, 2, -1);
        
        // poll失败
        if(ret < 0) {
            printf("poll failed\n");
            break;
        }

        // 服务器关闭连接
        if(fds[1].revents & POLLRDHUP) {
            printf("server close connection\n");
            break;
        } else if(fds[1].revents & POLLIN) {
            // 从服务器读取数据
            memset(read_buf, '\0', BUFFER_SIZE);
            recv(fds[1].fd, read_buf, BUFFER_SIZE - 1, 0);
            printf("%s\n", read_buf);
        }

        // 输入
        if(fds[0].revents & POLLIN) {
            // 使用splice将用户输入的数据直接写到sockfd上（零拷贝）
            ret = splice(0, NULL, pipefd[1], NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
            ret = splice(pipefd[0], NULL, sockfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        }
    }

    close(sockfd);
    return 0;
}
