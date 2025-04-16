#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <csignal>
#include <cstring>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

struct ClientInfo {
    int fd;
    int pipe_fd;
    pid_t pid;
};

std::vector<ClientInfo> clients;
std::map<pid_t, int> pipe_from_pid;

void reap_zombies(int) {
    while (waitpid(-1, nullptr, WNOHANG) > 0);
}

void broadcast(const std::string& message, int exclude_fd = -1) {
    for (const auto& client : clients) {
        if (client.fd != exclude_fd) {
            send(client.fd, message.c_str(), message.size(), 0);
        }
    }
}

int main() {
    signal(SIGCHLD, reap_zombies);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket");
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 0;
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(listen_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        return 1;
    }

    socklen_t len = sizeof(server_addr);
    getsockname(listen_fd, (sockaddr*)&server_addr, &len);
    std::cout << "Server started on port: " << ntohs(server_addr.sin_port) << std::endl;

    listen(listen_fd, MAX_CLIENTS);

    fd_set readfds;

    while (true) {
        FD_ZERO(&readfds);
        FD_SET(listen_fd, &readfds);
        int max_fd = listen_fd;

        for (const auto& client : clients) {
            FD_SET(client.pipe_fd, &readfds);
            if (client.pipe_fd > max_fd) max_fd = client.pipe_fd;
        }

        if (select(max_fd + 1, &readfds, nullptr, nullptr, nullptr) < 0) {
            perror("select");
            continue;
        }

        // Новое подключение
        if (FD_ISSET(listen_fd, &readfds)) {
            int client_fd = accept(listen_fd, nullptr, nullptr);
            if (client_fd < 0) {
                perror("accept");
                continue;
            }

            if (clients.size() >= MAX_CLIENTS) {
                std::string msg = "Server full. Try again later.\n";
                send(client_fd, msg.c_str(), msg.size(), 0);
                close(client_fd);
                continue;
            }

            int pipe_fd[2];
            if (pipe(pipe_fd) < 0) {
                perror("pipe");
                close(client_fd);
                continue;
            }

            pid_t pid = fork();
            if (pid < 0) {
                perror("fork");
                close(client_fd);
                close(pipe_fd[0]);
                close(pipe_fd[1]);
                continue;
            }

            if (pid == 0) {
                // child
                close(pipe_fd[0]);
                char buffer[BUFFER_SIZE];
                while (true) {
                    int bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
                    if (bytes <= 0) break;
                    buffer[bytes] = '\0';
                    std::string msg = "[client " + std::to_string(getpid()) + "]: " + buffer;
                    write(pipe_fd[1], msg.c_str(), msg.size());
                }
                close(client_fd);
                close(pipe_fd[1]);
                exit(0);
            } else {
                // parent
                close(pipe_fd[1]);
                fcntl(pipe_fd[0], F_SETFL, O_NONBLOCK); // pipe read non-block
                clients.push_back({client_fd, pipe_fd[0], pid});
                pipe_from_pid[pid] = pipe_fd[0];

                std::string welcome = "Client joined: pid " + std::to_string(pid) + "\n";
                broadcast(welcome);
            }
        }

        // Чтение из pipe'ов клиентов
        char buffer[BUFFER_SIZE];
        for (auto it = clients.begin(); it != clients.end(); ) {
            if (FD_ISSET(it->pipe_fd, &readfds)) {
                int bytes = read(it->pipe_fd, buffer, BUFFER_SIZE - 1);
                if (bytes <= 0) {
                    std::string msg = "Client left: pid " + std::to_string(it->pid) + "\n";
                    broadcast(msg);
                    close(it->fd);
                    close(it->pipe_fd);
                    pipe_from_pid.erase(it->pid);
                    it = clients.erase(it);
                    continue;
                }
                buffer[bytes] = '\0';
                broadcast(buffer);
            }
            ++it;
        }
    }

    close(listen_fd);
    return 0;
}
