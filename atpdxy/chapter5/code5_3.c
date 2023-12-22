#include <arpa/inet.h>
#include <unistd.h>
#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static bool stop = false;

// signalterm信号的处理函数
static void handleTerm(int sig){
    stop = true;
}

int main(int argc,char* argv[]){
    signal(SIGTERM, handleTerm);
    if(argc <= 3){
        printf("usage: %s ip_address port_number backlog\n",
               basename(argv[0]));
        return 1;
    }

    // 获得IP地址的字符串
    const char* ip = argv[1];
    // 将字符串转换成整数获得端口号
    int port = atoi(argv[2]);
    // 获得最大连接数
    int backlog = atoi(argv[3]);
    
    // 创建文件描述符
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    // 创建IPV4地址
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = PF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    // 绑定地址和文件描述符
    int ret = bind(sock, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    // 开始监听
    ret = listen(sock, backlog);
    assert(ret != -1);

    while(!stop){
        sleep(1);
    }

    close(sock);
    return 0;
}

