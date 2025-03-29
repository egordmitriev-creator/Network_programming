#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <IP> <PORT>" << std::endl;
        return 1;
    }

    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cout << "Socket creation error" << std::endl;
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));

    // Преобразование IP адреса из текстового в бинарный формат
    if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0) {
        std::cout << "Invalid address/ Address not supported" << std::endl;
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        std::cout << "Connection Failed" << std::endl;
        return -1;
    }

    int i;
    std::cout << "Enter a number between 1 and 10: ";
    std::cin >> i;
    if (i < 1 || i > 10) {
        std::cout << "Number out of range" << std::endl;
        return -1;
    }

    while (true) {
        std::string message = std::to_string(i);
        send(sock, message.c_str(), message.length(), 0);
        sleep(i); // Задержка перед отправкой следующего сообщения
    }

    close(sock);
    return 0;
}