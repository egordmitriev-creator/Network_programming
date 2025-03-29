#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <server_ip>" << std::endl;
        return 1;
    }

    int sockfd;
    struct sockaddr_in serverAddr;
    char buffer[BUFFER_SIZE];

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    socklen_t serverAddrLen = sizeof(serverAddr);

    int i = 1;
    while (i <= 5) {
        std::string message = std::to_string(i);
        std::cout << "Sending to server: " << message << std::endl;

        sendto(sockfd, message.c_str(), message.length(), 0, (const struct sockaddr *)&serverAddr, serverAddrLen);

        int n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 0, NULL, NULL);
        buffer[n] = '\0';
        std::cout << "Received from server: " << buffer << std::endl;

        sleep(i);
        i++;
    }
    close(sockfd);
    return 0;
}