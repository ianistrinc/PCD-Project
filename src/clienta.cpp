#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>

#define PORT 8080
const char* LOCK_FILE = "/tmp/clienta.lock"; // Define the path for the lock file

void remove_lock_file() {
    std::remove(LOCK_FILE);
}

int main()
{
    // Check if lock file exists
    std::ifstream lock_file_in(LOCK_FILE);
    if (lock_file_in.good()) {
        fprintf(stderr, "Admin client is already running.\n");
        return 1;
    }
    lock_file_in.close();

    // Create lock file
    std::ofstream lock_file_out(LOCK_FILE);
    if (!lock_file_out) {
        fprintf(stderr, "Unable to create lock file.\n");
        return 1;
    }
    lock_file_out << getpid(); // Write the current process ID to the lock file
    lock_file_out.close();

    // Ensure the lock file is removed on exit
    atexit(remove_lock_file);

    int sock = 0;
    struct sockaddr_in servAddr;
    char buffer[1024] = {0};
    char input[1024];

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Socket creation error");
        return -1;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);

    // Convert server address
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0)
    {
        perror("Invalid address");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Connection failed");
        return -1;
    }

    // Send initial identification message
    char initMessage[] = "Admin client";
    send(sock, initMessage, strlen(initMessage), 0);

    // Wait for initial server response
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = read(sock, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0'; // Null-terminate the received string
        printf("Received response: %s\n", buffer);
    }
    else if (bytesRead == 0)
    {
        printf("Server closed connection.\n");
        close(sock);
        return 0;
    }
    else
    {
        perror("Read error");
        close(sock);
        return -1;
    }

    // Wait for user input
    while (1)
    {
        printf("Enter command [close, message]: ");
        fflush(stdout);

        // Read user input
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character

        if (strcmp(input, "close") == 0)
        {
            printf("CLOSE\n");
            send(sock, input, strlen(input), 0);
            break;
        }

        // Send message
        send(sock, input, strlen(input), 0);
        printf("Message sent: %s\n", input);

        // Wait for response
        memset(buffer, 0, sizeof(buffer));
        bytesRead = read(sock, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0'; // Null-terminate the received string
           
        }
        else if (bytesRead == 0)
        {
            printf("Server closed connection.\n");
            break;
        }
        else
        {
            perror("Read error");
            break;
        }
    }

    // Close connection
    close(sock);
    return 0;
}
