#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sstream>

#define PORT 8080

int main()
{
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
    char initMessage[] = "Client 1";
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

    fd_set readfds;
    struct timeval timeout;

    // Print prompt once before the loop
    printf("Enter message [format: <image_path> <option> or type 'exit' to quit]: ");
    fflush(stdout);

    // Wait for user input
    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;

        int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("Select error");
            break;
        }

        if (FD_ISSET(sock, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));
            bytesRead = read(sock, buffer, sizeof(buffer) - 1);
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0'; // Null-terminate the received string
                if (strcmp(buffer, "Server is shutting down.") == 0)
                {
                    printf("%s\n", buffer);
                    break;
                }
                printf("Received response: %s\n", buffer);
                // Print prompt again after receiving a valid response
                printf("Enter message [format: <image_path> <option> or type 'exit' to quit]: ");
                fflush(stdout);
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

        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            fgets(input, sizeof(input), stdin);
            input[strcspn(input, "\n")] = 0; // Remove newline character

            if (strcmp(input, "exit") == 0)
            {
                printf("Exiting...\n");
                break;
            }

            // Check if input format is correct
            std::istringstream iss(input);
            std::string imagePath, option;
            iss >> imagePath >> option;

            if (imagePath.empty() || option.empty() || 
                (option != "-c" && option != "-g" && option != "-r"))
            {
                printf("Invalid input. Use format '<image_path> <option>' where option is -c, -g, or -r.\n");
                // Print prompt again for invalid input
                printf("Enter message [format: <image_path> <option> or type 'exit' to quit]: ");
                fflush(stdout);
                continue;
            }

            // Send message
            send(sock, input, strlen(input), 0);
            printf("Message sent: %s\n", input);

            // Print prompt again after sending a valid message
       
            fflush(stdout);
        }
    }

    // Close connection
    close(sock);
    return 0;
}
