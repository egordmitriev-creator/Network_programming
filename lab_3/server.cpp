#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <iostream>
#include <arpa/inet.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

// Структура для передачи данных в поток
struct ThreadData {
    int connfd;
};

// Функция, которая будет выполняться в потоке
void* handle_client(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    int connfd = data->connfd;
    char buffer[BUFFER_SIZE];
    int reciveRead;

    while (true) {
        reciveRead = recv(connfd, buffer, BUFFER_SIZE, 0);
        if (reciveRead <= 0) break;

        buffer[reciveRead] = '\0';
        std::cout << "Client: " << buffer << std::endl;

        int number = atoi(buffer);

        std::string message = "Hello to client " + std::to_string(number);

        send(connfd, message.c_str(), message.size(), 0);
    }

    close(connfd);
    delete data;
    return nullptr;
}

int main() {
    char buffer[BUFFER_SIZE];
    int sockfd, connfd;
    struct sockaddr_in serverAddr;
    socklen_t len = sizeof(serverAddr);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = 0;
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    listen(sockfd, 5); // максимальная длина очереди ожидающих соединений

    if (getsockname(sockfd, (struct sockaddr*)&serverAddr, &len) == -1) {
        perror("getsockname failed");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server started on port: " << ntohs(serverAddr.sin_port) << std::endl;

    while (true) {
        connfd = accept(sockfd, 0, 0);

        if (connfd < 0) {
            perror("accept failed");
            exit(EXIT_FAILURE);
        }

        // Создаем структуру для передачи данных в поток
        ThreadData* data = new ThreadData;
        data->connfd = connfd;

        // Создаем поток для обработки соединения
        pthread_t thread;
        if (pthread_create(&thread, nullptr, handle_client, data) != 0) {
            perror("pthread_create failed");
            close(connfd);
            delete data;
        }

        // Отсоединяем поток, чтобы он мог завершиться самостоятельно
        pthread_detach(thread);
    }

    close(sockfd);
    return 0;
}