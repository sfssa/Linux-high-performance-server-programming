#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <arpa/inet.h>

// 这是一个客户端程序，调用connect函数连接服务器并发送消息

int main(int argc, char* argv[]){
    if(argc <= 2){
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    // 获得服务器的IP和端口号
    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    // 设置服务器地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    // 创建与服务器连接的文件描述符
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // 与服务器连接
    if(connect(sock, (struct sockaddr*)&address, sizeof(address)) < 0){
        printf("connection failed\n");
    }else{
        // 连接成功发送普通数据和带外数据
        const char* oodData = "abc";
        const char* normalData = "123";
        send(sock, normalData, strlen(normalData), 0);
        send(sock, oodData, strlen(oodData), MSG_OOB);
        send(sock, normalData, strlen(normalData), 0);
    }

    close(sock);
    return 0;
}
