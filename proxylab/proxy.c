#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "csapp.h"

/* Recommended max cache and object sizes */
#define MAX_CACHE_SIZE 1049000
#define MAX_OBJECT_SIZE 102400

/* You won't lose style points for including this long line in your code */
static const char *user_agent_hdr = "User-Agent: Mozilla/5.0 (X11; Linux x86_64; rv:10.0.3) Gecko/20120305 Firefox/10.0.3\r\n";

int main(int argc, char const *argv[])
{
    struct sockaddr_storage addr;
    socklen_t clientlen;
    char buffer[MAX_CACHE_SIZE];
    char request[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    rio_t rio;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(0);
    }

    int listenfd = open_listenfd(argv[1]);

    while (true)
    {
        clientlen = sizeof(struct sockaddr_storage);
        int connfd = Accept(listenfd, (SA *)&addr, &clientlen);

        if(Fork() == 0)
        {
            Close(listenfd);

            bzero(request, MAXLINE);
            bzero(buffer, MAXLINE);

            rio_readinitb(&rio, connfd);
            rio_readlineb(&rio, request, MAXLINE);
            printf("request: %s", request);

            sscanf(request, "%s %s %s", method, uri, version);

            char *p = strchr(uri, ':');
            p = strchr(p + 1, ':');
            p = strchr(p + 1, '/');
            sprintf(buffer, "%s %s %s\r\n", method, p, version);
            printf("%s", buffer);

            while (strcmp(request, "\r\n"))
            {
                rio_readlineb(&rio, request, MAXLINE);
                printf("%s", request);
                strcat(buffer, request);
            }

            int clientfd = Open_clientfd("127.0.0.1", "64258");
            Rio_writen(clientfd, buffer, strlen(buffer));
            int responseSize = Rio_readn(clientfd, buffer, MAX_CACHE_SIZE);

            Rio_writen(connfd, buffer, responseSize);

            Close(clientfd);
            Close(connfd);
            exit(0);
        }

        Close(connfd);
    }
    Close(listenfd);

    return 0;
}

