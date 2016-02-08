/**************************************************
File: README.txt
Author: Tatsiana Clifton
Date: 2/7/2016
Description: Instructions how to use chatserver.cpp
             and chatclient.c programs
***************************************************/

*****************
     COMPILE
*****************
Place chatserver.cpp chatclient.c makefile in the same directory
and with command line:

make

*****************
     CLEAN
*****************
Place chatserver.cpp chatclient.c makefile in the same directory
and run:

make clean  

****************
     USAGE
****************
chatserver starts with the command line ./chatserver <port#> and 
waits on <port#> for a client request

chatclient starts with the command line./chatclient <hostname> <port#>

Start chatserver first. It will begin listen for connections.
Start chatclient. Connection will be established.
chatclient will require to enter a handle before sending messages.

chatclient sends message first, chatserver excepts, displays the message
and responses; chaclient excepts, displays and responses and so on.

******************** 
   CLOSE CONNECTION
********************   
chatserver closes the connection with the command “\quit” for both; 
chatclient closes the connection with the command “\quit” 
only for chatclient, not chatserver; to close connection for chatserver
use CTRL+C

*******************
     TESTING
*******************
both programs on flip3: chatserver 30021	 
                        chatclient localhost 30021
	   
chatserver on flip2,
chatclient on flip3: chatserver 30021	
                     chatclient flip2.engr.oregonstate.edu 30021

*******************
   RESOURCES      
*******************
1) Beej's Guide to Network Programming
Using Internet Sockets
Brian "Beej Jorgensen" Hall
http://beej.us/guide/bgnet/output/html/multipage/index.html

2) codereview.stackexchange.com/questions/13461/two-way-communication-in-tcp-server-client-implementation
