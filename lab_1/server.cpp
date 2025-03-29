#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    int sockfd;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in clientAddr, serverAddr;
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
        close(sockfd);
    }

    socklen_t clientAddrLen = sizeof(clientAddr);

    while (true) {
        int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 0, (struct sockaddr *)&clientAddr, &clientAddrLen);
        buffer[n] = '\0';
        std::cout << "Received from client: " << buffer << std::endl;
        std::cout << "Client IP: " << inet_ntoa(clientAddr.sin_addr) << ", Port: " << ntohs(clientAddr.sin_port) << std::endl;
        
        int number = atoi(buffer);

        std::string message = "Server response to client: " + std::to_string(number);

        sendto(sockfd, message.c_str(), strlen(message.c_str()), 0, (struct sockaddr *)&clientAddr, clientAddrLen);
    }
    close(sockfd);
    return 0;
}