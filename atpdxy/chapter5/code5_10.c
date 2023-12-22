#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

// 缓冲数组大小
const int BUF_SIZE = 512;

int main(int argc, char* argv[]){

    if(argc <= 2){
        printf("usage: %s ip_address port_number buf_size\n", basename(argv[0]));
        return 1;
    }

    // 获得IP和端口号
    const char* ip = argv[1];
    const int port = atoi(argv[2]);
    // 创建服务器地址
    struct sockaddr_in serverAddress;
    
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    inet_pton(AF_INET, ip, (struct sockaddr*)&serverAddress);
    serverAddress.sin_port = htons(port);
    // 创建与服务器连接的文件描述符
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);
    // 设置并获得缓冲区大小
    int sendBuf = atoi(argv[3]);
    int sendBufSize = sizeof(sendBuf);
    setsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendBuf, sendBufSize);
    getsockopt(sockfd, SOL_SOCKET, SO_SNDBUF, &sendBuf, (socklen_t*)&sendBufSize);
    printf("the tcp send buffer size after setting is: %d\n", sendBuf);
    // 连接
    if(connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != -1){
        char buffer[BUF_SIZE];
        memset(buffer, 'a', BUF_SIZE);
        send(sockfd, buffer, BUF_SIZE, 0);
    }
    // 关闭
    close(sockfd);
    return 0;
}
