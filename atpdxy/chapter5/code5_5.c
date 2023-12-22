#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

// 结论：accept只从监听队列中取出连接，不论连接出于何种状态，更不关心网络状态的变化

int main(int argc, char* argv[])
{
    if(argc <= 2){
        printf("usage: %s ip_address  port_number\n", basename(argv[0]));
        return 1;
    }

    // 获得IP地址和端口号
    const char* ip = argv[1];
    int port = atoi(argv[2]);

    // 创建address_in
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = PF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    // 创建socket
    int sock =socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // 绑定
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    // 监听
    ret = listen(sock, 5);
    assert(ret != -1);
    
    // 睡眠一分钟等待客户端操作
    sleep(60);

    // 准备客户端
    struct sockaddr_in client;
    socklen_t clientLength = sizeof(client);

    // 连接客户端
    int connfd = accept(sock,(struct sockaddr*)&client, &clientLength);
    if(connfd < 0){
        printf("errno is: %d\n", errno);
    }else{
        char remote[INET_ADDRSTRLEN];
        printf("connected with ip:%s and port: %d\n", inet_ntop(AF_INET, &client.sin_addr, remote, INET_ADDRSTRLEN),
               ntohs(client.sin_port));
        close(connfd);
    }

    close(sock);
    
    return 0;
}

