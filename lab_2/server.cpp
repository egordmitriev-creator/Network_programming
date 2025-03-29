#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void reaper(int sig) {
    int status;
    while (wait3(&status, WNOHANG, (struct rusage*)0) >= 0);
}

int main(){

    char buffer[BUFFER_SIZE];
    int sockfd, connfd;
    struct sockaddr_in serverAddr;
    socklen_t len = sizeof(serverAddr);

    int reciveRead;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1){
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    //memset(&serverAddr, 0, sizeof(serverAddr));    
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 0;
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 5); // максимальная длинна очереди ожидающих соединений

    if(getsockname(sockfd, (struct sockaddr *)&serverAddr, &len) == -1){
        perror("getsockname failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port: " << ntohs(serverAddr.sin_port) << std::endl;

    signal(SIGCHLD, reaper);

    while (true) {
        connfd = accept(sockfd, 0, 0);

        if(connfd < 0){
            perror("accept failed");
            exit(EXIT_FAILURE);
        }
        switch (fork())
        {
        case -1:
            perror("fork failed");
            break;
        
        case 0:
            close(sockfd);
            while (true) {
                reciveRead = recv(connfd, buffer, BUFFER_SIZE, 0);
                if (reciveRead <= 0) break;

                buffer[reciveRead] = '\0';
                std::cout << "Client: " << buffer << std::endl;

                int number = atoi(buffer);

                std::string message = "Hello to client " + std::to_string(number);



                send(connfd, message.c_str(), sizeof(message), 0);
            }

            close(connfd);
            exit(0);
            
        default:
            close(connfd);
        }
    }
    close(sockfd);
    return 0;

}