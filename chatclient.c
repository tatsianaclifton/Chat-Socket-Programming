/*******************************************************
* Name: CS372 Program 1 chatclient.c                   *
* Author: Tatsiana Clifton                             *
* Date: 2/2/2016                                       *
* Description: The chat client implementation of       *
*              a client-server network application.    *
*              The application is a simple chat system *
*              for one pair of users.                  *
* Usage: chatclient hostname port                      *
*                                                      *
* Sources:1)beej.us/guide/bgnet/output/html/multipage/ *
* clientserver.html#simpleclient                       *
* 2)codereview.stackexchange.com/questions/13461/two-  *
* way-communication-in-tcp-server-client-implementation*
********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXDATASIZE 500 //max number of bytes it can be get at once
                        //set up to 500 to be able send a message
                        //up to 500 characters

//function prototypes
int setConnect(char *host, char *port);
void sendMessage(int sockfd, char *handle);
void receiveMessage(int sockfd);

//global variables
char input[500];
char buffer[MAXDATASIZE];
int numbytes;
char *quit = "\\quit";


int main(int argc, char *argv[]){
   int sockfd, len;
   char handle[10];

   //check for correct usage, host and port must be provided
   if (argc != 3){
      fprintf(stderr, "usage: chatclient hostname port\n");
      exit(1);
   }
   //set connection and get socket
   sockfd = setConnect(argv[1], argv[2]);

   //request handle form the user
   printf("Please enter handle: ");
   fgets(handle, 10, stdin);
   len = strlen(handle) - 1;
   if (handle[len] == '\n'){
      handle[len] = '\0';
   }
   //while connection is up continue sending and receiving
   while (1){
      sendMessage(sockfd, handle);
      receiveMessage(sockfd);
   }
   //close socket
   close(sockfd);
   return 0;
}

 /*******************************************************
 Function: get_in_addr
 Receive: pointer to sockaddr struct
 Return: IP4 or IP6
 Description: gets IPv4 or IPv6 in sockaddr struct
 Sources: http://beej.us/guide/bgnet/output/html/multipage/index.html
 ********************************************************/
void *get_in_addr(struct sockaddr *sa){
   if (sa->sa_family == AF_INET){
      return &(((struct sockaddr_in*)sa)->sin_addr);
   }
   return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/*******************************************************
 Function: setConnect
 Receive: host name and port number
 Return: socket descriptor sockfd
 Description: establish connection to the server with  specified
 host name and port number
 Sources: http://beej.us/guide/bgnet/output/html/multipage/index.html
 ********************************************************/
int setConnect(char *host, char *port){
   int sockfd, numbytes;
   char buffer[MAXDATASIZE];
   struct addrinfo hints, *servinfo, *p;
   int rv;
   char s[INET6_ADDRSTRLEN];

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;//set family that both IPv4 and IPv6
                               //can be looked
   hints.ai_socktype = SOCK_STREAM; //set socke to be SOCK_STREAM
   //get info about host, otherwise error
   if ((rv = getaddrinfo(host, port, &hints, &servinfo)) != 0){
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
      return 1;
   }

   //loop through all the results and connect to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next){
      //get new socket descriptor, otherwise error
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
              p->ai_protocol)) == -1){
         perror("client: socket");
         continue;
      }
      //connect to the socket, otherwise error
      if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1){
         close(sockfd);
         perror("client: connect");
         continue;
      }
       break;
   }
   //if all resuls were tried and connection not established 
   if (p == NULL){
      fprintf(stderr, "client: failed to connect\n");
      exit(1);
   }
   //get IP address of the server
   inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
              s, sizeof s);
   printf("client: connecting to %s\n", s);

   freeaddrinfo(servinfo); //all done with this structure

   return sockfd;
}

/*******************************************************
 Function: sendMessage
 Receive: socket descriptor sockfd and handle
 Return: none
 Description: sends messages to the server
 Sources: source: codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementation
 ********************************************************/
void sendMessage(int sockfd, char *handle){
   //clear buffer
   memset(buffer, 0, MAXDATASIZE);
   //clear input
   memset(input, 0, MAXDATASIZE);
   //always display handle
   printf("%s> ", handle);
   //get message input from the user 
   fgets(input, sizeof input, stdin);
   //concatenate handle and message to send together
   strcpy(buffer, handle);
   strcat(buffer, "> ");
   strcat(buffer, input);
   //check that message is not quit, if quit close connection
   if(strncmp(input, quit, 5) == 0){
      send(sockfd, input, strlen(buffer), 0);
      close(sockfd);
      exit(0);
   }
   //if message was not send display error and exit
   if ((send(sockfd, buffer, strlen(buffer), 0)) == -1){
      fprintf(stderr, "client: failed to send message\n");
      close(sockfd);
      exit(1);
   }
}

/*******************************************************
 Function: receiveMessage
 Receive: socket descriptor sockfd
 Return: none
 Description: receives messages from the server
 Sources: source: codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementation
 ********************************************************/
void receiveMessage(int sockfd){
   //clear buffer
   memset(buffer, 0, MAXDATASIZE);
   //count how many bytes received
   numbytes = recv(sockfd, buffer, sizeof buffer, 0);
   //if server send "\quit" close connection
   if (strncmp(buffer, quit, 5) == 0){
      printf("connection closed by server\n");
      close(sockfd);
      exit(0);
   }
   //if zero or less bytes, it is error
   if (numbytes <= 0){
      printf("connection closed or error\n");
      exit(1);
   }
   //get rid of new line charachter at the end of buffer
   int len = strlen(buffer) - 1;
   if (buffer[len] == '\n'){
      buffer[len] = '\0';
   }
   //display message from the server
   printf("Server> %s\n", buffer);
}
