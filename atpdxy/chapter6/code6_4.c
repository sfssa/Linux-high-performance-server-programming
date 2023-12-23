#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <string.h>

// 由于没有执行recv或者send函数，因此为未涉及到用户空间和内核空间之间的数据拷贝
// 实现了简单高效的回射服务，实现了零拷贝

int main(int argc, char* argv[]){
    if(argc <= 2){
        printf("usage: %s ip_address port_number\n", basename(argv[0]));
        return 1;
    }

    const char* ip = argv[1];
    const int port = atoi(argv[2]);

    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &address.sin_addr);
    address.sin_port = htons(port);

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(sockfd >= 0);

    int ret = bind(sockfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);

    ret = listen(sockfd, 5);
    assert(ret != 5);

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    int connfd = accept(sockfd, (struct sockaddr*)&client, &clientLen);
    if(connfd < 0){
        printf("error is: %d", errno);
    }else{
        int pipefd[2];
        assert(ret != -1);
        ret = pipe(pipefd);
        
        ret = splice(connfd, NULL, pipefd[1], NULL,32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);

        ret = splice(pipefd[0], NULL, connfd, NULL, 32768, SPLICE_F_MORE | SPLICE_F_MOVE);
        assert(ret != -1);
        close(connfd);
    }
    close(sockfd);
    return 0;
}
