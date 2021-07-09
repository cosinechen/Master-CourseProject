#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h> 
#include <time.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    int sockfd = 0, portnum, n;

    if(getuid()!=0 || geteuid()!=0){
		printf("ERROR: You must be root to use this tool!\n");
		exit(1);
  	}

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <0){
		perror ("socket fail\n");
		exit(1);
    }

    struct sockaddr_in  server, cliaddr;
    bzero(&server, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    portnum = atoi(argv[2]);
    server.sin_port = htons(portnum);

    int addrlen = sizeof(cliaddr);
    char message[100] = {};
    char receiveMessage[100] = {};
    
    if(connect(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0){
      printf("Connection error\n");      
      exit(1);
    }

    getsockname(sockfd, (struct sockaddr *)&cliaddr, (socklen_t *)&addrlen);

    while(1){
      printf("Please enter a message: ");

      //Send a message to server
      memset(message, 0, sizeof(message));      
      fgets(message, 100, stdin);             
      send(sockfd, message, sizeof(message), 0);
      memset(receiveMessage, 0, sizeof(receiveMessage));      
      recv(sockfd, receiveMessage, sizeof(receiveMessage), 0);

      if(receiveMessage[0] != '\0'){
        printf("Receive a message from server: %s\n", receiveMessage);
        break;
      }
    }
    close(sockfd);
    return 0;
}