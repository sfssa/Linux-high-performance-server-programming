#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>
#include <time.h>
#include <unistd.h>

const int BUFFER_SIZE = 1023;

int setNonBlocking(int fd) {
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
    return old_option;
}

int unBlockConnect(const char* ip, const int port, const int time) {
    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int old_opt = setNonBlocking(sockfd);

    ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
    if(ret == 0) {
        // 返回值为0，表示连接成功建立
        printf("connect with server immediately\n");
        fcntl(sockfd, F_SETFL, old_opt);
        return sockfd;
    } else if(errno != EINPROGRESS) {
        // 错误码不等于EINPROGRESS，连接出错
        printf("unblock connect not support\n");
        return -1;
    }
    // 正在建立连接，设置等待时间并用select来检测
    fd_set readfds;
    fd_set writefds;
    FD_ZERO(&readfds);
    FD_ZERO(&writefds);
    
    struct timeval timeout;
    timeout.tv_sec = time;
    timeout.tv_usec = 0;
    
    ret = select(sockfd + 1, NULL ,&writefds, NULL, &timeout);
    if(ret <= 0) {
        // select出错或者连接时间内没有建立连接
        printf("connection time out\n");
        close(sockfd);
        return -1;
    }

    // 判断是否可写，不可写连接出错
    if(!FD_ISSET(sockfd, &writefds)) {
        printf("no event on sockfd found\n");
        close(sockfd);
        return -1;
    }
    
    int error = 0;
    socklen_t length = sizeof(error);
    // 获取并清除sockfd上的错误
    if(getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &error, &length) < 0) {
        printf("get socket option failed\n");
        close(sockfd);
        return -1;
    }
    // 错误号不为0表示连接出错
    if(error != 0) {
        printf("connection failed after select with the error: %d\n", error);
        close(sockfd);
        return -1;
    }
    // 连接成功
    printf("connection ready after select with the socket: %d\n", sockfd);
    fcntl(sockfd, F_SETFL, old_opt);
    return sockfd;
}

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }
    
    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int sockfd = unBlockConnect(ip, port, 10);
    if(sockfd < 0) {
        return 1;
    }
    close(sockfd);
    return 0;
}
