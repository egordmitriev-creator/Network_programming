#include <iostream>
#include <vector>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>

#define PORT 0 // 0 означает, что система сама выберет свободный порт

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    fd_set readfds;
    int max_sd;
    std::vector<int> client_sockets;

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка сокета
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Привязка сокета
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Прослушивание сокета
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Получение номера порта
    getsockname(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
    std::cout << "Server is listening on port: " << ntohs(address.sin_port) << std::endl;

    while (true) {
        FD_ZERO(&readfds); // Очищает набор файловых дескрипторов readfds. Это необходимо перед каждым вызовом select, чтобы начать с чистого набора.
        FD_SET(server_fd, &readfds); // Добавляет дескриптор серверного сокета (server_fd) в набор readfds. Это позволяет select отслеживать новые подключения на серверном сокете.
        max_sd = server_fd;

        // Добавление клиентских сокетов в fd_set
        for (auto sd : client_sockets) {
            FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        // Ожидание активности на одном из сокетов
        int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
        if ((activity < 0) && (errno != EINTR)) {
            std::cout << "select error" << std::endl;
        }

        // Если есть новое подключение
        if (FD_ISSET(server_fd, &readfds)) {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            std::cout << "New connection, socket fd is " << new_socket << std::endl;
            client_sockets.push_back(new_socket);
        }

        // Проверка данных от клиентов
        for (auto it = client_sockets.begin(); it != client_sockets.end(); ++it) {
            int sd = *it;
            if (FD_ISSET(sd, &readfds)) {
                char buffer[1024] = {0};
                int valread = read(sd, buffer, 1024);
                if (valread == 0) {
                    // Клиент отключился
                    getpeername(sd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
                    std::cout << "Host disconnected, ip " << inet_ntoa(address.sin_addr) << ", port " << ntohs(address.sin_port) << std::endl;
                    close(sd);
                    client_sockets.erase(it--);
                } else {
                    // Вывод полученных данных
                    std::cout << "Received: " << buffer << std::endl;
                }
            }
        }
    }

    return 0;
}