#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char** argv)
{
    int    sockfd;
    char    recvline[4096], sendline[4096];
    

    if( argc != 2){
        printf("usage: ./client <ipaddress>\n");
        exit(0);
    }

    printf("send msg to server: \n");
    fgets(sendline, 4096, stdin);

    while(sendline[0] != '!'){

        if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
            printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
            exit(0);
        }

        struct sockaddr_in servaddr;
        memset(&servaddr, 0, sizeof(servaddr));

        servaddr.sin_family = AF_INET;
        servaddr.sin_port = htons(6666);
        if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
            printf("inet_pton error for %s\n",argv[1]);
            exit(0);
        }


    
        if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
            printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
            exit(0);
        }

        uint sid = 1;
        size_t msg_len = strlen(sendline)+sizeof(size_t)+sizeof(uint);
        int x = send(sockfd, &msg_len, sizeof(size_t), MSG_NOSIGNAL);
        printf("send msg error:%d\n", x);
        x = send(sockfd, &sid, sizeof(uint), MSG_NOSIGNAL);
        printf("send msg error:%d\n", x);
        x = send(sockfd, sendline, strlen(sendline), MSG_NOSIGNAL);
        printf("send msg error:%d\n", x);


        close(sockfd);
        
        printf("send msg to server: \n");
        fgets(sendline, 4096, stdin);

        
    }

    
    exit(0);
}