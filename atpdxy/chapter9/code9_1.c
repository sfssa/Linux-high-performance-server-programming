#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

int main(int argc, char* argv[]) {
    if(argc <= 2) {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    // 获取IP和端口
    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    // 创建服务器地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &(address.sin_addr));
    address.sin_port = htons(port);

    // 创建服务器文件描述符
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd > 0);

    // 绑定地址
    int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    // 监听
    ret = listen(sockfd, 5);
    assert(ret != -1);

    // 创建客户端地址和文件描述符
    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    int clientfd = accept(sockfd, (struct sockaddr*)&client, &clientLen);
    if(clientfd < 0) {
        printf("error in accept %d\n", errno);
        return 1;
    }

    char buf[1024];
    fd_set readFds;
    fd_set exceptFds;
    FD_ZERO(&readFds);
    FD_ZERO(&exceptFds);

    while(1) {
        memset(buf, '\0', sizeof(buf));
        // 每次使用都要重新设置文件描述符，因为会被内核修改
        FD_SET(clientfd, &readFds);
        FD_SET(clientfd, &exceptFds);
        int ret = select(clientfd + 1, &readFds, NULL, &exceptFds, NULL);
        if(ret < 0) {
            printf("error in select %d\n", errno);
            return 1;
        }
        // select返回后可能接收正常数据也可能接收带外数据
        if(FD_ISSET(clientfd, &readFds)) {
            ret = recv(clientfd, buf, sizeof(buf) - 1, 0);
            if(ret <= 0) {
                break;
            }
            printf("Get %d Bytes Of Normal Data: %s\n", ret, buf);
        } else if(FD_ISSET(clientfd, &exceptFds)) {
            ret = recv(clientfd, buf, sizeof(buf) - 1, MSG_OOB);
            if(ret <= 0) {
                break;
            }
            printf("Get %d Bytes Of Oob Data: %s", ret, buf);
        }
    }
    close(clientfd);
    close(sockfd);

    return 0;
}
