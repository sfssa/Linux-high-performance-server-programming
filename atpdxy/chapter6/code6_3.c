#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/sendfile.h>

int main(int argc, char* argv[]){
    // 用法
    if(argc <= 3){
        printf("usage: %s ip_address port_number file_name\n", basename(argv[0]));
        return 1;
    }
    // 获得参数
    const char* ip = argv[1];
    const int port = atoi(argv[2]);
    const char* fileName = argv[3];
    // 打开文件并绑定状态
    int fileFd = open(fileName, O_RDONLY);
    assert(fileFd > 0);
    struct stat fileStat;
    fstat(fileFd, &fileStat);
    // 服务器地址初始化
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port); 
    // 服务器文件描述符
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd > 0);
    // 绑定
    int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    // 监听
    ret = listen(sockfd, 5);
    assert(ret != -1);
    // 客户端地址和文件描述符
    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    // 接受连接
    int connfd = accept(sockfd, (struct sockaddr*)&client, &clientLen);
    if(connfd > 0){
        sendfile(connfd, fileFd, NULL, fileStat.st_size);
        close(connfd);
    }else{
        printf("error is: %d" ,errno);
    }
    // 关闭文件描述符
    close(sockfd);
    return 0;

}
