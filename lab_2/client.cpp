#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <unistd.h>

#define BUFFER_SIZE 1024

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Использование: " << argv[0] << " <Server IP> <Port>" << std::endl;
        return 1;
    }

    const char* serverIP = argv[1];

    int clientSocket;
    struct sockaddr_in serverAddress;
    char buffer[BUFFER_SIZE];

    // Создание TCP сокета SOCK_DGRAM -> SOCK_STREAM
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    // Настройка адреса сервера
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(atoi(argv[2]));
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);

    if (inet_pton(AF_INET, serverIP, &serverAddress.sin_addr) <= 0) {
        std::cerr << "Ошибка в IP адресе" << std::endl;
        close(clientSocket);
        return 1;
    }

    // Подключение к серверу
    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cerr << "Ошибка подключения к серверу" << std::endl;
        close(clientSocket);
        return 1;
    }

    // Цикл отправки данных
    for (int i = 1; i <= 5; ++i) { // Отправляем числа от 1 до 5
        std::string strData = std::to_string(i);
        const char* message = strData.c_str();

        std::cout << "Отправка числа " << i << " на сервер..." << std::endl;

        // Отправка данных на сервер
        if (send(clientSocket, message, strlen(message), 0) < 0) {
            std::cerr << "Ошибка при отправке данных" << std::endl;
            continue;
        }

        // Получение ответа от сервера
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE, 0);
        if (bytesReceived < 0) {
            std::cerr << "Ошибка при получении данных" << std::endl;
            continue;
        }

        buffer[bytesReceived] = '\0'; // Добавляем завершающий нуль для строки
        std::cout << "Ответ от сервера: " << buffer << std::endl;

        sleep(i); // Задержка в i секунд
    }

    close(clientSocket);
    return 0;
}