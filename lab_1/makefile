PROGRWS = client server
CC = g++

all: $(PROGRWS)

client: client.cpp
	$(CC) -o client client.cpp

server: server.cpp
	$(CC) -o server server.cpp

runServer:
	./server

clean:
	rm -f $(PROGRWS)