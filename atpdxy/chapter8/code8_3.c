#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>
#include <string.h>

// 读缓冲区大小
const int READ_BUFFER_SIZE = 2048;

// 主状态机，正在分析请求行还是请求头
enum CheckState {
    CHECK_STATE_REQUESTLINE = 0,
    CHECK_STATE_HEADER = 1
};
// 从状态机，读取到完整的行，行出错或者行数据不完整
enum LineStatus {
    LINE_OK = 1, 
    LINE_BAD = 2, 
    LINE_OPEN
};

// 服务器解析客户端请求结果
enum HttpCode {
    NO_REQUEST = 1, 
    GET_REQUEST = 2,
    BAD_REQUEST = 3,
    FORBIDDEN_REQUEST = 4,
    INTERNAL_ERROR = 5,
    CLOSED_CONNECTION
};

// 服务器响应报文
static const char* szret[] = { "I get a correct result\n", "Something wrong\n" };

// 从状态机，解析一行数据
// buffer - 读缓冲区；checkedIndex - 正在检查的字符；readIndex - 缓冲区读取的客户端最后一个数据的下一个位置
LineStatus parseLine(char* buffer, int& checkedIndex, int& readIndex) {
    char temp;
    for(; checkedIndex < readIndex; ++checkedIndex) {
        temp = buffer[checkedIndex];
        if(temp == '\r') {
            if(checkedIndex + 1 == readIndex) {
                return LINE_OPEN;
            } else if(buffer[checkedIndex + 1] == '\n') {
                buffer[checkedIndex++] = '\0';
                buffer[checkedIndex++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        } else if(temp == '\n') {
            // 这一部分有必要吗？？？
            if(checkedIndex > 1 && buffer[checkedIndex - 1] == '\r') {
                buffer[checkedIndex - 1] = '\0';
                buffer[checkedIndex++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    // 没有找到\r\n
    return LINE_OPEN;
}

// 分析请求行，例如：GET /index.html HTTP/1.1
HttpCode parseRequestLine(char* temp, CheckState& checkstate) {
    // 查找temp中第一个空白符或者制表符
    char* url = strpbrk(temp, " \t");
    // 没有\t说明格式错误
    if(!url) {
        return BAD_REQUEST;
    }
    // 将GET和/index.html切割开来
    *url++ = '\0';
    
    char* method = temp;
    // 在method中查询GET字符串（忽略大小写）
    if(strcasecmp(method, "GET") == 0) {
        printf("The Request Method Is GET\n");
    } else {
        return BAD_REQUEST;
    }

    // url中第一个不在 \t中的字符（跳过URL前的空格字符）
    url += strspn(url, " \t");
    // 找到URL和HTTP版本之间的第一个空格
    char* version = strpbrk(url, " \t");
    if(!version) {
        return BAD_REQUEST;
    }
    // 将URL和HTTP版本切割开来
    *version++ = '\0';
    // 跳过HTTP版本前的空格
    version  += strspn(version, " \t");
    if(strcasecmp(version, "HTTP/1.1") != 0) {
        return BAD_REQUEST;
    }

    // 检查URL是否合法，请求的是绝对路径
    if(strncasecmp(url, "http://", 7) == 0) {
        url += 7;
        url = strchr(url, '/');
    }

    // 相对路径以/为根目录起始
    if(!url || url[0] != '/') {
        return BAD_REQUEST;
    }
    printf("The Request URL Is: %s\n", url);
    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

HttpCode parseHeaders(char* temp) {
    // 如果是空行，意味着请求头已经结束了
    if(temp[0] == '\0') {
        return GET_REQUEST;
    } else if(strncasecmp(temp, "Host:", 5) == 0) {
        // 移动指针Host:后面的字符
        temp += 5;
        // 跳过空格
        temp += strspn(temp, " \t");
        printf("The Request Host Is: %d\n", temp);
    } else {
        printf("I Can't Handle This Header\n");
    }
    return NO_REQUEST;
}

// 解析请求体函数
// buffer - 带解析的数据； checkedIndex - 正在检查的字符；checkState - 检查状态
// readIndex - 数据结尾的下一个字符； startLine - 行在buffer起始的位置
HttpCode parseContent(char* buffer, int& checkedIndex, CheckState& checkState, int& readIndex, int& startLine) {
    // 当前行的读取状态
    LineStatus linestatus = LINE_OK;
    // HTTP请求处理结果
    HttpCode retcode = NO_REQUEST;
    while((linestatus = parseLine(buffer, checkedIndex, readIndex)) == LINE_OK) {
        // startLine是在buffer中的起始位置
        char* temp = buffer + startLine;
        // 保存下一行的起始位置
        startLine = checkedIndex;
        // checkState记录主状态机当前状态
        switch(checkState) {            
            // 分析请求行
            case CHECK_STATE_REQUESTLINE:
            {
                retcode = parseRequestLine(temp, checkState);
                if(retcode == BAD_REQUEST) {
                    return BAD_REQUEST;
                }
                break;
            }
            // 分析请求头
            case CHECK_STATE_HEADER:
            {
                retcode = parseHeaders(temp);
                if(retcode == BAD_REQUEST) {
                    return BAD_REQUEST;
                } else if(retcode == GET_REQUEST) {
                    return GET_REQUEST;
                }
                break;
            }
            default:
            {   
                return INTERNAL_ERROR;
            }
        }
    }
    // 没有读取到完整的行，需要进一步读取数据
    if(linestatus == LINE_OPEN) {
        return NO_REQUEST;
    } else {
        return BAD_REQUEST;
    }
}

int main(int argc, char* argv[]) {
    if(argc <= 2) {
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

    int listenfd = socket(PF_INET, SOCK_STREAM, 0);
    assert(listenfd >= 0);
    int ret = bind(listenfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    ret = listen(listenfd, 5);
    assert(ret != -1);

    struct sockaddr_in client;
    socklen_t clientLen = sizeof(client);
    int clientfd = accept(listenfd, (struct sockaddr*)&client, &clientLen);
    if(clientfd < 0) {
        printf("error in accept\n", errno);
    } else {
        char buffer[READ_BUFFER_SIZE];
        memset(buffer, '\0', READ_BUFFER_SIZE);
        // 记录每次读取的数据的个数
        int dataRead = 0;
        // 读取到的最后的数据
        int readIndex = 0;
        // 正在检查的数据
        int checkedIndex = 0;
        // 行在buffer中的起始位置
        int startLine = 0;
        // 初始化主状态机的状态，解析状态
        CheckState checkstate = CHECK_STATE_REQUESTLINE;
        // 循环读取客户数据并进行解析
        while(1) {
            dataRead = recv(clientfd, buffer + readIndex, READ_BUFFER_SIZE - readIndex, 0);
            if(dataRead == -1) {
                printf("reading failed\n");
                break;
            } else if(dataRead == 0) {
                printf("remote client closed connection\n");
                break;
            }
            readIndex += dataRead;
            // 解析本次获得的数据
            HttpCode result = parseContent(buffer, checkedIndex, checkstate, readIndex, startLine);
            if(result == NO_REQUEST) {
                // 尚未得到完整的请求
                continue;
            } else if(result == GET_REQUEST) {
                send(clientfd, szret[0], strlen(szret[0]), 0);
                break;
            } else {
                send(clientfd, szret[1], strlen(szret[1]), 0);
                break;
            }
        }
        close(clientfd);
    }
    close(listenfd);
    return 0;
}
