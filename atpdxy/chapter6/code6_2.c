#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <sys/uio.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const int BUFFER_SIZE = 1024;

static const char* STATUS_LINE[2] = { "200 OK", "500 Internal server error" };

int main(int argc, char* argv[]){
    if(argc <= 3){
        printf("usage: %s ip_address port_number filename\n", basename(argv[0]));
        return 1;
    }
    
    const char* ip = argv[1];
    int port = atoi(argv[2]);
    const char* fileName = argv[3];

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
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);

    int connfd = accept(sockfd, (struct sockaddr*)&client, &clientLen);
    if(connfd == -1){
        printf("error is: %d", errno);
    }else{
        // 存放响应头内容
        char headerBuffer[BUFFER_SIZE];
        memset(headerBuffer, '\0', BUFFER_SIZE);
        // 存放响应体内容
        char* fileBuffer;
        struct stat fileStat;
        bool isValid = true;
        int len = 0;
        // 查看文件是否存在并绑定文件和状态结构体
        if(stat(fileName, &fileStat) < 0){
            isValid = false;
        }else{
            // 是一个目录
            if(S_ISDIR(fileStat.st_mode)){
                isValid = false;
            }else if(fileStat.st_mode & S_IROTH){
                // 有读取目标文件的权限
                int fd = open(fileName, O_RDONLY);
                fileBuffer = new char[fileStat.st_size + 1];
                memset(fileBuffer, '\0', fileStat.st_size + 1);
                // 将fd对应的文件读取到fileBuffer中
                if(read(fd,fileBuffer, fileStat.st_size) < 0){
                    isValid = false;
                }
            }else{
                isValid = false;
            }
        }

        if(isValid){
            // 请求后
            int ret = snprintf(headerBuffer, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", STATUS_LINE[0]);
            len += ret;

            // 请求头
            ret = snprintf(headerBuffer + len, BUFFER_SIZE - 1 - len, "Content-Length: %d\r\n", fileStat.st_size);
            len += ret;

            // 空行
            ret = snprintf(headerBuffer + len, BUFFER_SIZE - 1 - len, "%s" "\r\n");

            struct iovec iv[2];
            iv[0].iov_base = headerBuffer;
            iv[0].iov_len = sizeof(headerBuffer);
            iv[1].iov_base = fileBuffer;
            iv[1].iov_len = fileStat.st_size;
            ret = writev(connfd, iv, 2);
        }else{
            ret = snprintf(headerBuffer, BUFFER_SIZE - 1, "%s %s\r\n", "HTTP/1.1", STATUS_LINE[1]);
            len += ret;
            ret = snprintf(headerBuffer + len, BUFFER_SIZE - 1 - len, "%s", "\r\n");
            send(connfd, headerBuffer, strlen(headerBuffer), 0);
        }
        close(connfd);
        delete[] fileBuffer;
    }
    close(sockfd);
    return 0;
}
