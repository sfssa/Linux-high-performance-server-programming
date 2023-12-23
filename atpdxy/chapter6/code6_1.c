#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[]){

    if(argc <= 2){
        printf("usage: %s ip_address port_number", basename(argv[0]));
        return 1;
    }

    // 获取IP和端口号
    const char* ip = argv[1];
    const int port = atoi(argv[2]);
    // 创建服务器地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    // 创建服务器文件描述符
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    // 绑定
    int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    // 监听
    ret = listen(sockfd, 5);
    assert(ret != -1);
    // 客户端地址和文件描述符
    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    int connfd = accept(sockfd, (struct sockaddr*)&client, &clientLen);
    if(connfd < 0){
        printf("error is: %d", errno);
    }else{
        // 关闭标准输出
        close(STDOUT_FILENO);
        // 标准输出是1，dup函数返回系统最小可用的文件描述符的值，也就是会返回1；
        // 这样printf的标准输出会转嫁到connfd这个文件描述符中，abcd不会显示在终端
        // 上，而是会被客户端获得，这就是CGI程序的基本工作原理。
        dup(connfd);
        printf("abcd\n");
        close(connfd);
    }
    // 关闭文件描述符
    close(sockfd);
    return 0;
}
