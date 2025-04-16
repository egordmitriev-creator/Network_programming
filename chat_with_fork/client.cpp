#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <Server IP> <Port>" << std::endl;
        return 1;
    }

    const char* server_ip = argv[1];
    int port = atoi(argv[2]);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        return 1;
    }

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        return 1;
    }

    std::cout << "Connected to chat. Type 'exit' to leave.\n";

    pid_t pid = fork();

    if (pid == 0) {
        // child — receive messages
        char buffer[BUFFER_SIZE];
        while (true) {
            int bytes = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytes <= 0) break;
            buffer[bytes] = '\0';

            std::cout << buffer;
            // Добавляем перенос строки, если в сообщении его не было
            if (buffer[bytes - 1] != '\n') std::cout << std::endl;
        }
        std::cout << "Disconnected from server.\n";
        close(sock);
        exit(0);
    } else {
        // parent — send messages
        std::string input;
        while (true) {
            std::getline(std::cin, input);
            if (input == "exit") break;

            if (input.back() != '\n') input += '\n';

            send(sock, input.c_str(), input.size(), 0);
        }
        close(sock);
        kill(pid, SIGKILL); // завершение дочернего процесса
    }

    return 0;
}
