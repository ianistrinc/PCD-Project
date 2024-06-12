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
#include <cstring>    // Include pentru memset
#include <sys/wait.h> // Include pentru waitpid

#define PORT 8080

std::mutex clients_mutex;
std::map<int, std::thread> clientThreads;
std::atomic<bool> running(true);
int serverFd = 0;

/**
 * Functie pentru a trimite un mesaj tuturor clientilor conectati
 */
void broadcastMessage(const std::string &message)
{
    std::lock_guard<std::mutex> lock(clients_mutex); // Asigura accesul sincronizat la clientThreads
    for (const auto &client : clientThreads)
    {
        send(client.first, message.c_str(), message.size(), 0); // Trimite mesajul catre fiecare client
    }
}

/**
 * Functie pentru a inchide toate socket-urile clientilor conectati
 */
void closeAllSockets()
{
    std::lock_guard<std::mutex> lock(clients_mutex); // Asigura accesul sincronizat la clientThreads
    for (const auto &client : clientThreads)
    {
        close(client.first); // Inchide fiecare socket al clientilor
    }
}

/**
 * Functie pentru a deserializa input-ul primit de la client
 * Returneaza true daca input-ul este valid, altfel false
 */
bool deserializeInput(const char input[], std::string &path, std::string &option)
{
    size_t len = strlen(input); // Determina lungimea input-ului
    if (len < 4)                // Verifica daca lungimea este mai mica de 4 caractere
    {
        return false;
    }

    // Optiunea este reprezentata de ultimele doua caractere
    option = std::string(input + len - 2);

    // Calea este restul string-ului
    path = std::string(input, len - 3);

    // Verifica daca optiunea este valida
    if (option != "-c" && option != "-g" && option != "-r")
    {
        return false;
    }

    return true;
}

/**
 * Functie pentru a executa un command folosind execl
 * Creeaza un proces copil pentru a rula command-ul
 */
void executeCommand(const std::string &command, const std::string &path)
{
    pid_t pid = fork(); // Creeaza un proces copil
    if (pid == 0)
    {                                                                        // Proces copil
        execl(command.c_str(), command.c_str(), path.c_str(), (char *)NULL); // Ruleaza command-ul in procesul copil
        perror("execl failed");                                              // Afiseaza un mesaj de eroare daca execl esueaza
        exit(EXIT_FAILURE);                                                  // Inchide procesul copil
    }
    else if (pid > 0)
    { // Proces parinte
        int status;
        waitpid(pid, &status, 0); // Asteapta finalizarea procesului copil
        if (WIFEXITED(status))    // Verifica daca procesul copil a terminat cu succes
        {
            std::cout << "Command executed successfully: " << command << std::endl;
        }
        else
        {
            std::cerr << "Command execution failed: " << command << std::endl;
        }
    }
    else
    {
        perror("fork failed"); // Afiseaza un mesaj de eroare daca fork esueaza
    }
}

/**
 * Functie pentru a gestiona comunicarea cu un client
 * Se ocupa de primirea mesajelor si executarea comenzilor
 */
void handleClient(int clientSocket, const std::string &clientType)
{
    char buffer[1024] = {0};
    std::string welcomeMessage = "Welcome to the server! You are connected as " + clientType + ".\n";
    send(clientSocket, welcomeMessage.c_str(), welcomeMessage.size(), 0); // Trimite un mesaj de bun venit clientului

    while (running)
    {
        memset(buffer, 0, sizeof(buffer));                // Reseteaza buffer-ul
        int bytesRead = read(clientSocket, buffer, 1024); // Citeste mesajul de la client
        if (bytesRead > 0)
        {
            std::string message(buffer, bytesRead);
            std::cout << clientType << ": " << message << std::endl;

            if (clientType == "Admin client" && message == "close")
            {
                std::cout << "Close command received. Stopping server." << std::endl;
                running = false;
                broadcastMessage("Server is shutting down.\n");
                closeAllSockets();
                close(serverFd);
                exit(0); // Inchide serverul
            }
            else
            {
                std::string path, option;
                if (deserializeInput(buffer, path, option)) // Deserializa input-ul clientului
                {
                    if (option == "-c")
                    {
                        executeCommand("./contur", path); // Executa command-ul contur
                    }
                    else if (option == "-g")
                    {
                        executeCommand("./canny", path); // Executa command-ul canny
                    }
                    else if (option == "-r")
                    {
                        executeCommand("./rotate", path); // Executa command-ul rotate
                    }
                }
                else
                {
                    std::string errorMessage = "Invalid input. Use format '<image_path> <option>' where option is -c, -g, or -r.\n";
                    send(clientSocket, errorMessage.c_str(), errorMessage.size(), 0); // Trimite un mesaj de eroare clientului
                }

                send(clientSocket, buffer, bytesRead, 0); // Trimite inapoi mesajul primit (echo)
            }
        }
        else if (bytesRead == 0)
        {
            std::cout << clientType << " disconnected." << std::endl; // Afiseaza un mesaj cand clientul se deconecteaza
            break;
        }
    }

    close(clientSocket);                             // Inchide socket-ul clientului
    std::lock_guard<std::mutex> lock(clients_mutex); // Asigura accesul sincronizat la clientThreads
    clientThreads.erase(clientSocket);               // Sterge clientul din lista clientThreads
}

/**
 * Functie pentru a gestiona semnalele (ex: SIGINT)
 * Opreste serverul in mod corespunzator
 */
void signalHandler(int signum)
{
    std::cout << "Interrupt signal (" << signum << ") received. Shutting down gracefully..." << std::endl;
    running = false;
    broadcastMessage("Server is shutting down.\n");
    closeAllSockets();
    close(serverFd);
    exit(0); // Inchide serverul
}

int main()
{
    signal(SIGINT, signalHandler);  // Asociaza functia signalHandler cu semnalul SIGINT
    signal(SIGTERM, signalHandler); // Asociaza functia signalHandler cu semnalul SIGTERM

    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed"); // Afiseaza un mesaj de eroare daca socket-ul esueaza
        exit(EXIT_FAILURE);      // Inchide serverul
    }

    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt"); // Afiseaza un mesaj de eroare daca setsockopt esueaza
        exit(EXIT_FAILURE);   // Inchide serverul
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed"); // Afiseaza un mesaj de eroare daca bind esueaza
        exit(EXIT_FAILURE);    // Inchide serverul
    }

    if (listen(serverFd, 3) < 0)
    {
        perror("listen");   // Afiseaza un mesaj de eroare daca listen esueaza
        exit(EXIT_FAILURE); // Inchide serverul
    }

    std::cout << "Server is listening on port " << PORT << std::endl;

    while (running)
    {
        int newSocket;
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) >= 0)
        {
            char buffer[1024] = {0};
            read(newSocket, buffer, 1024); // Citeste mesajul de la noul client
            std::string clientType(buffer);
            std::cout << "New " << clientType << " connected" << std::endl; // Afiseaza un mesaj cand un nou client se conecteaza
            std::lock_guard<std::mutex> lock(clients_mutex);                // Asigura accesul sincronizat la clientThreads

            clientThreads[newSocket] = std::thread(handleClient, newSocket, clientType); // Creeaza un nou thread pentru client
            clientThreads[newSocket].detach();                                           // Detaseaza thread-ul
        }
    }

    std::cout << "Closing server..." << std::endl;
    for (auto &th : clientThreads)
    {
        if (th.second.joinable())
        {
            th.second.join(); // Asteapta terminarea thread-urilor
        }
    }

    close(serverFd); // Inchide socket-ul serverului
    std::cout << "Server shut down gracefully." << std::endl;

    return 0;
}
