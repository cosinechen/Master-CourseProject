#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <sys/select.h>

int main(int argc, char *argv[])
{
  	int sockfd = 0, clientsockfd = 0;
    int portnum;
    char buf[100] = {};
    char message[100] = {};

    if(getuid()!=0 || geteuid()!=0){
		printf("ERROR: You must be root to use this tool!\n");
		exit(1);
  	}

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <0){
		perror("socket fail\n");
		exit(1);
    }

    struct sockaddr_in  servaddr, cliaddr;
    int saddrlen = sizeof(servaddr); 
    int caddrlen = sizeof(cliaddr); 
    struct timeval tv;
    fd_set readfds;
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    //servaddr.sin_port = htons(0);
    portnum = atoi(argv[1]);
    servaddr.sin_port = htons(portnum);
          
    bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    listen(sockfd, 5);

    while(1){
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		select(sockfd+1, &readfds, NULL, NULL, &tv); 
		/*if(rv == -1)
			perror("select error\n");
		else if(rv == 0)
			printf("Timeout\n");*/
		//printf("isset %d\n",FD_ISSET(sockfd, &readfds));
      
		if(FD_ISSET(sockfd, &readfds)){
        
        if((clientsockfd = accept(sockfd, (struct sockaddr*)&cliaddr, (socklen_t *)&caddrlen)) <0){
          printf("accept fail\n");
          exit(1); 
        }
        getsockname(clientsockfd, (struct sockaddr *)&servaddr, (socklen_t *)&saddrlen);
        getpeername(clientsockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&caddrlen);

        recv(clientsockfd, buf, sizeof(buf), 0);
        printf("Get a message: %sfrom %s, port %d\n", buf, inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
        strcpy(message,buf);

        send(clientsockfd, message, sizeof(message), 0);
 
      }
    }
    close(clientsockfd);
    close(sockfd);
    return 0;
}