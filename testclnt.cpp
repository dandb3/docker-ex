#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/uio.h>
#include <vector>
#include <string>

void err(const char *msg)
{
    printf("%s\n", msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    struct sockaddr_in addr;
    char buf[1024];
    struct iovec *iov;
    ssize_t readlen;
    int sockfd, filefd;
    std::vector<std::string> request;

    if (argc != 3) {
        printf("Usage: %s, <IP> <port>\n", argv[0]);
        return 1;
    }
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(atoi(argv[2]));
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) == -1)
        err("socket failed");
    if (connect(sockfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
        err("connect failed");
    if ((filefd = open("request.txt", O_RDONLY)) == -1)
        err("open failed");
    while ((readlen = read(filefd, buf, sizeof(buf) - 1)) > 0) {
        buf[readlen] = '\x00';
        request.push_back(buf);
    }
    if (readlen == -1)
        return 1;
    if ((iov = (struct iovec *) malloc(sizeof(struct iovec) * request.size())) == NULL)
        return 1;
    for (int i = 0; i < request.size(); ++i) {
        iov[i].iov_base = (void *) request[i].c_str();
        iov[i].iov_len = request[i].size();
    }
    writev(sockfd, iov, request.size());
    shutdown(sockfd, SHUT_WR);
    while ((readlen = read(sockfd, buf, sizeof(buf) - 1)) > 0) {
        buf[readlen] = '\x00';
        printf("%s", buf);
    }
    if (readlen == -1)
        exit(1);
    close(filefd);
    close(sockfd);
    return 0;
}