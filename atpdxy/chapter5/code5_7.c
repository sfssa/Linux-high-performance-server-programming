#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>

// TCP中的带外数据处理机制是针对一个字节的，因此每次发送和接收通常只能是一个字节。
// 当发送多个字符时，仅把最后一个字符当作带外数据处理。
// 并且服务器对带外数据的接收将被带外数据截断，带外数据前和后的普通数据不能被一个recv调用全部读出的。

// 缓冲区大小
const size_t BUF_SIZE = 1024;

int main(int argc, char* argv[]){
    if(argc <= 2){
        printf("usage: %s ip_address port_number", basename(argv[0]));
        return 1;
    }
    // 根据输入获得IP和PORT
    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    // 创建地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    // 创建文件描述符
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);
    // 绑定
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    // 监听
    ret = listen(sock, 5);
    assert(ret != -1);
    // 创建客户端地址
    struct sockaddr_in client;
    socklen_t clientArrLength = sizeof(client);
    // 接收
    int connfd = accept(sock,(struct sockaddr*)&client, &clientArrLength);
    if(connfd < 0){
        printf("errno is: %d\n", errno);
    }else{
        char buffer[BUF_SIZE];

        memset(buffer, '\0', BUF_SIZE);
        ret = recv(connfd, buffer, BUF_SIZE - 1, 0);
        printf("got %d bytes of normal data '%s'\n", ret, buffer);

        memset(buffer, '\0', BUF_SIZE);
        ret = recv(connfd, buffer, BUF_SIZE - 1,MSG_OOB);
        printf("get %d bytes of ood data '%s'\n", ret, buffer);

        memset(buffer, '\0', BUF_SIZE);                            
        ret = recv(connfd, buffer, BUF_SIZE - 1, 0);               
        printf("got %d bytes of normal data '%s'\n", ret, buffer); 
        close(connfd);
    }
    // 关闭文件描述符
    close(sock);
    return 0;
}
