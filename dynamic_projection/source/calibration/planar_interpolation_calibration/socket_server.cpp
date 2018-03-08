/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cassert>
#include <sstream>

#define FULLSCREEN_WIDTH  4096
#define FULLSCREEN_HEIGHT 2160
#define MY_WIDTH 4096
#define MY_HEIGHT 1080
#define MY_LEFT 0
#define MY_BOTTOM 1080

#include "socket_helper.h"

void error(const char *msg)
{
  std::cout << "SERVER ERROR" << std::endl;
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     char receive_buffer[MAX_BUFFER_SIZE];
     //     char receive_buffer[MAX_BUFFER_SIZE];
     struct sockaddr_in serv_addr, cli_addr;
     int n;
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     newsockfd = accept(sockfd, 
                 (struct sockaddr *) &cli_addr, 
                 &clilen);
     if (newsockfd < 0) 
          error("ERROR on accept");

     std::cout << "client connected" << std::endl;

     while (1) {
       
       bzero(receive_buffer,MAX_BUFFER_SIZE);
       n = read(newsockfd,receive_buffer,MAX_BUFFER_SIZE-1);
       if (n < 0) error("ERROR reading from socket");
       printf("Here is the message: %s\n",receive_buffer);

       pid_t childPID = commandify(receive_buffer,false);
       if (childPID == 0) {
	 std::cout << "EXITING SERVER" << std::endl;
	 break;
       }

       std::cout << "CHILD PROCESS " << childPID << std::endl;

       std::stringstream ss;
       ss << "I got your message " << childPID;

       std::string return_message = ss.str();

       n = write(newsockfd,return_message.c_str(),return_message.size()+1);//ss.str().c_str(),18);
       if (n < 0) error("ERROR writing to socket");

     }

     close(newsockfd);
     close(sockfd);
     return 0; 
}
