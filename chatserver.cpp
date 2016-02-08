/*******************************************************
* Name: CS372 Program 1 chatserver.cpp                 *
* Author: Tatsiana Clifton                             *
* Date: 2/6/2016                                       *
* Description: The chat server implementation of       *
*              a client-server network application     *
*              The application is a simple chat system *
*              for one pair of users                   *
* Usage: chatserver port                               *
*                                                      *
* Sources: beej.us/guide/bgnet/output/html/multipage/  *
* clientserver.html#simpleserver                       *
********************************************************/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

#define BACKLOG 10 //how many pending connections queue will hold
#define MAXDATASIZE 500 //the max number of characters in messages

//functions prototypes
void setConnect(char *port);
void sendMessage(int sockfd);
void receiveMessage(int sockfd);

//global variables
char buffer[MAXDATASIZE];
int numbytes;


int main(int argc, char *argv[]){
   //check usage, port must be provided
   if (argc != 2){
      fprintf(stderr, "usage: chatserver port\n");
      exit(1);
   }
   //set connection, exchange messages
   setConnect(argv[1]);

   return 0;
}

/*******************************************************
 Function: sigchld_handler
 Receive: signal
 Return: none
 Description: reap dead child processes
 Sources: http://beej.us/guide/bgnet/output/html/multipage/index.html
 ********************************************************/
void sigchld_handler(int s){
   //waitpid() might overwrite errno, so we save and restore it
   int saved_errno = errno;

   while (waitpid(-1, NULL, WNOHANG) > 0);

   errno = saved_errno;
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
 Receive: port number
 Return: none
 Description: set up the server to listen on specified
 port number and except request and set up connection
 Sources: beej.us/guide/bgnet/output/html/multipage/clientserver.html#simpleserver
 ********************************************************/
void setConnect(char *port){
   int sockfd, new_fd; //listen on sock_fd, new connection on new_fd
   struct addrinfo hints, *servinfo, *p;
   struct sockaddr_storage their_addr; //connector's address info
   socklen_t sin_size;
   struct sigaction sa;
   int yes = 1;
   char s[INET6_ADDRSTRLEN];
   int rv;

   memset(&hints, 0, sizeof hints);
   hints.ai_family = AF_UNSPEC;//set family that both IPv4 and IPv6
                               //can be looked
   hints.ai_socktype = SOCK_STREAM;//set socket to be SOCK_STREAM
   hints.ai_flags = AI_PASSIVE; //use my IP
   //get info about host, otherwise error
   if ((rv = getaddrinfo(NULL, port, &hints, &servinfo)) != 0){
      cout << stderr << "getaddrinfo: " << gai_strerror(rv) <<endl;
      exit(1);
   }

   //loop through all the results and bind to the first we can
   for (p = servinfo; p != NULL; p = p->ai_next){
      if ((sockfd = socket(p->ai_family, p->ai_socktype,
             p->ai_protocol)) == -1){
         perror("server: socket");
         continue;
      }
      //allow other sockets to bind, if not error
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
             sizeof(int)) == -1){
         perror("setsockopt");
         exit(1);
      }
      //bind socket
      if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1){
         close(sockfd);
         perror("server: bind");
         continue;
      }
      break;
   }

   freeaddrinfo(servinfo); //all done with  this structure
   //if did not found where to bind, error
   if (p == NULL){
      cout << stderr << "server: failed to bind" << endl;
      exit(1);
   }
   //listen for connection
   if (listen(sockfd, BACKLOG) == -1){
      perror("listen");
      exit(1);
   }

   sa.sa_handler = sigchld_handler; //reap all dead processes
   sigemptyset(&sa.sa_mask);
   sa.sa_flags = SA_RESTART;
   if (sigaction(SIGCHLD, &sa, NULL) == -1){
      perror("sigaction");
      exit(1);
   }

   cout << "server: waiting for connections..." << endl;

   while (1){ //main accept() loop
      sin_size = sizeof their_addr;
      //accept connection, if not error
      new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
      if (new_fd == -1){
         perror("accept");
         continue;
      }
      //get IP address
      inet_ntop(their_addr.ss_family,
          get_in_addr((struct sockaddr *)&their_addr),
          s, sizeof s);
      cout << "server: got connection from " << s << endl;
      //receive messages
      receiveMessage(new_fd);
   }
   //close connection
   close(new_fd);
}

/*******************************************************
 Function: receiveMessage
 Receive: socket descriptor new_fd
 Return: none
 Description: receives messages from the client
 Sources: source: codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementation
 ********************************************************/
void receiveMessage (int new_fd){
   while(1){
      //clear buffer
      memset(buffer, 0, MAXDATASIZE);
      //count how many bytes received, if none error
      if ((numbytes = recv(new_fd, buffer, sizeof buffer, 0)) == -1){
         perror("recv");
         exit(1);
      }
      //check if message is "\quit", if yes break
      if (strncmp(buffer, "\\quit", 5) == 0){
         cout << "connection closed by client" << endl;
         close(new_fd);
         break;
      }
      //get rid of new line charachter at the end of buffer
      int len = strlen(buffer) - 1;
      if (buffer[len] == '\n'){
         buffer[len] = '\0';
      }
      //display message
      cout << buffer << endl;
      //send message back
      sendMessage(new_fd);
   }
}

/*******************************************************
 Function: sendMessage
 Receive: socket descriptor new_fd
 Return: none
 Description: sends messages to the client
 Sources: source: codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementation
 ********************************************************/
void sendMessage (int new_fd){
   //clear buffer
   memset(buffer, 0, MAXDATASIZE);
   //display server's handle
   cout << "Server> ";
   //get input
   fgets(buffer, sizeof buffer, stdin);
   //check if the input is "\quit" and send it that
   //server know that it should also quit
   if(strncmp(buffer, "\\quit", 5) == 0){
      send(new_fd, buffer, sizeof buffer, 0);
      close(new_fd);
      exit(0);
   }
   //send message, if not successful show error and close connection
   if((send(new_fd, buffer, sizeof buffer, 0)) == -1){
      cout << stderr << "server: cannot send message" << endl;
      close(new_fd);
      exit(1);
   }
}
