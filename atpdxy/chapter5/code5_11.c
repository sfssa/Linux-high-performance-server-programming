#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

// Tips:在码代码时不要照着书中给的代码示例写，尽量自己记下来，过一遍脑子，可以先将步骤写出来，然后用代码填充

// 缓冲区大小
const int BUF_SIZE = 1024;

int main(int argc, char* argv[]){

    if(argc <= 2){
        printf("usage: %s ip_address port_number recv buffer size", basename(argv[0]));
        return 1;
    }

    // 获得IP和端口号
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
    // 设置并获得缓冲区大小
    int recvBuf = atoi(argv[3]);
    int recvBufLen = sizeof(recvBuf);
    setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvBuf, sizeof(recvBuf));
    getsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvBuf, (socklen_t*)&recvBufLen);
    printf("the tcp recv buffer size after setting is: %d\n", recvBuf);
    // 绑定
    int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    // 监听
    ret = listen(sockfd, 5);
    assert(ret != -1);
    // 创建客户端地址
    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    // 接收并获得数据
    int connfd = accept(sockfd, (struct sockaddr*)&client, &clientLen);
    if(connfd < 0){
        printf("error is: %d", errno);
    }else{
        char buffer[BUF_SIZE];
        memset(buffer, '\0', BUF_SIZE);
        while(recv(connfd, buffer, BUF_SIZE - 1, 0) > 0){}
        close(connfd);
    }
    // 关闭文件描述符
    close(sockfd);
    return 0;
}
