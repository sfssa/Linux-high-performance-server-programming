#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    assert(argc == 2);
    return 0;   

    char* host = argv[1];
    struct hostent* hostInfo = gethostbyname(host);
    assert(hostInfo);
    struct servent* servInfo = getservbyname("daytime", "tcp");
    assert(servInfo);

    printf("daytime port is: %d\n", ntohs(servInfo->s_port));

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = servInfo->s_port;
    address.sin_addr = *(struct in_addr*)*hostInfo->h_addr_list;

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    int ret = connect(sockfd, (struct sockaddr*)&address, sizeof(address));
    assert(ret != -1);
    char buffer[128];
    ret = read(sockfd, buffer, sizeof(buffer));
    assert(ret > 0);
    buffer[ret] = '\0';
    printf("the day time is: %s", buffer);
    close(sockfd);
    return 0;
}
