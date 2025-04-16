PROGRWS = client server
CC = g++

all: $(PROGRWS)

client: client.cpp
	$(CC) -o client client.cpp -pthread -std=c++11

server: server.cpp
	$(CC) -o server server.cpp -std=c++11

clean:
	rm -f $(PROGRWS)
	rm -f *.o
