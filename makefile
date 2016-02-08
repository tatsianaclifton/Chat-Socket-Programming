compile: chatserver chatclient

chatserver: chatserver.cpp
	g++ -o chatserver chatserver.cpp

chatclient: chatclient.c
	gcc -o chatclient chatclient.c

clean:
	$(RM) *.o chatserver chatclient
