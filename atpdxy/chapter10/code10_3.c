#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <signal.h>

#define BUFFER_SIZE 1024

static int connfd;

// SIGURG信号的处理函数
void sig_urg(int sig)
{
    int save_errno = errno;
    char buffer[BUFFER_SIZE];
    memset(buffer, '\0', BUFFER_SIZE);
    int ret = recv(connfd, buffer, BUFFER_SIZE - 1, MSG_OOB);
    printf("get %d bytes of oob data '%s'\n", ret, buffer);
    errno = save_errno;
}

void addsig(int sig, void (*sig_handler)(int))
{
    struct sigaction sa;
    memset(&sa, '\0', sizeof(sa));
    sa.sa_handler = sig_handler;
    sa.sa_flags |= SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig, &sa, NULL) != -1);
}

int main(int argc, char* argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    int ret;
    struct sockaddr_in address;
    memset(&address, '\0', sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);
    
    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);

    ret = bind(AF_INET, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct sockaddr_in client_address;
    socklen_t client_addr_len = sizeof(client_address);
    int connfd = accept(listenfd, (struct sockaddr*)&client_address, &client_addr_len);
    if(connfd < 0) 
    {
        printf("error in accept, errno is: %d\n", errno);
        return 1;
    }
    else 
    {
        addsig(SIGURG, sig_urg);
        // 设置文件描述符connfd的拥有者进程为当前进程ID，以便内核在接收到带外数据时可以通知特定的进程而不是默认进程
        // getpid()返回当前进程的ID
        // 指定connfd的拥有者进程是当前进程的ID
        fcntl(connfd, F_SETOWN, getpid());
        char buffer[BUFFER_SIZE];
        while(1) 
        {
            memset(buffer, '\0', sizeof(buffer));
            ret = recv(connfd, buffer, BUFFER_SIZE - 1, 0);
            if(ret <= 0) 
            {
                break;
            }
            printf("got %d bytes of normal data '%s'\n", ret, buffer); 
        }
        close(connfd);
    }
    close(listenfd);
    return 0;
}
