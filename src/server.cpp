#include <iostream>
#include <string>
#include <thread>
#include <vector>
#include <map>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <mutex>
#include <atomic>
#include <signal.h>
#include <cstring> // Include for memset
#include <sys/wait.h> // Include for waitpid

#define PORT 8080

std::mutex clients_mutex;
std::map<int, std::thread> clientThreads;
std::atomic<bool> running(true);
int serverFd = 0;

void broadcastMessage(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clientThreads) {
        send(client.first, message.c_str(), message.size(), 0);
    }
}

void closeAllSockets() {
    std::lock_guard<std::mutex> lock(clients_mutex);
    for (const auto& client : clientThreads) {
        close(client.first);
    }
}

void deserializeInput(const char input[], std::string& path, std::string& option) {
    size_t len = strlen(input);
    if (len < 3) {
        std::cerr << "Input is too short to contain a valid option." << std::endl;
        return;
    }

    // Option is the last two characters
    option = std::string(input + len - 2);

    // Path is the rest of the string
    path = std::string(input, len - 3);
}

void executeCommand(const std::string& command, const std::string& path) {
    pid_t pid = fork();
    if (pid == 0) {  // Child process
        execl(command.c_str(), command.c_str(), path.c_str(), (char*)NULL);
        perror("execl failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {  // Parent process
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "Command executed successfully: " << command << std::endl;
        } else {
            std::cerr << "Command execution failed: " << command << std::endl;
        }
    } else {
        perror("fork failed");
    }
}

void handleClient(int clientSocket, const std::string& clientType) {
    char buffer[1024] = {0};
    std::string welcomeMessage = "Welcome to the server! You are connected as " + clientType + ".\n";
    send(clientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0);

    while (running) {
        memset(buffer, 0, sizeof(buffer));
        int bytesRead = read(clientSocket, buffer, 1024);
        if (bytesRead > 0) {
            std::string message(buffer, bytesRead);
            std::cout << clientType << ": " << message << std::endl;

            if (clientType == "Admin client" && message == "close") {
                std::cout << "Close command received. Stopping server." << std::endl;
                running = false;
                broadcastMessage("Server is shutting down.\n");
                closeAllSockets();
                close(serverFd);
                exit(0);
            } else {
                std::string path, option;
                deserializeInput(buffer, path, option);

                if (option == "-c") {
                    executeCommand("./contur", path);
                } else if (option == "-g") {
                    executeCommand("./canny", path);
                } else if (option == "-r") {
                    executeCommand("./rotate", path);
                } else {
                    std::cerr << "Invalid option received: " << option << std::endl;
                }

                send(clientSocket, buffer, bytesRead, 0); // Echo back the message
            }
        } else if (bytesRead == 0) {
            std::cout << clientType << " disconnected." << std::endl;
            break;
        }
    }

    close(clientSocket);
    std::lock_guard<std::mutex> lock(clients_mutex);
    clientThreads.erase(clientSocket);
}

void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received. Shutting down gracefully..." << std::endl;
    running = false;
    broadcastMessage("Server is shutting down.\n");
    closeAllSockets();
    close(serverFd);
    exit(0);
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(serverFd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    while (running) {
        int newSocket;
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) >= 0) {
            char buffer[1024] = {0};
            read(newSocket, buffer, 1024);
            std::string clientType(buffer);
            std::cout << "New " << clientType << " connected" << std::endl;
            std::lock_guard<std::mutex> lock(clients_mutex);

            clientThreads[newSocket] = std::thread(handleClient, newSocket, clientType);
            clientThreads[newSocket].detach();
        }
    }

    std::cout << "Closing server..." << std::endl;
    for (auto& th : clientThreads) {
        if (th.second.joinable()) {
            th.second.join();
        }
    }

    close(serverFd);
    std::cout << "Server shut down gracefully." << std::endl;

    return 0;
}
