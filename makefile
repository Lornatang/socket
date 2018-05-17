tar:Server  Client
Server:Server.c
	gcc -ggdb3 -Wall -o  $@  $< -lpthread
Client:Client.c
	gcc -ggdb3 -Wall  -o  $@ $< -lpthread
clean:
	rm -rf Server Client  FIFO
