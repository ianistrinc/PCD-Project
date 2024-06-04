#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <time.h>
#include <pwd.h>


#define PORT 8080

int main()
{
    int serverFd, newSocket;
    struct sockaddr_in address;
    int opt = 1;
    int addrLen = sizeof(address);
    char buffer[1024] = {0};

    // Crearea socket-ului
    if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Eroare la crearea socket-ului");
        exit(EXIT_FAILURE);
    }

    // Setarea optiunilor socket-ului
    if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("Eroare la setarea optiunilor socket-ului");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Legarea socket-ului la adresa si portul specificat
    if (bind(serverFd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Eroare la legarea socket-ului la adresa si la portul specificat");
        exit(EXIT_FAILURE);
    }

    // Ascultarea conexiunilor
    if (listen(serverFd, 3) < 0)
    {
        perror("Eroare la ascultarea conexiunilor");
        exit(EXIT_FAILURE);
    }

    // Asteapta mai multe comenzi, adica server-ul poate primi de la client comenzile user si time la infinit, iar la primirea comenzii close se inchid ambele (asta am adaugat-o in plus fata de ce s-a cerut in problema pentru ca mi se pare mai practic asa)
    while (1)
    {
        // Asteptarea conexiunii de la client
        if ((newSocket = accept(serverFd, (struct sockaddr *)&address, (socklen_t *)&addrLen)) < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        // Procesare mesaje de la client
        while (1)
        {
            memset(buffer, 0, sizeof(buffer)); /* Se curata buffer-ul pentru a putea primi un nou mesaj de la client */
            read(newSocket, buffer, sizeof(buffer) - 1);

            // Procesarea mesajelor primite de la client si trimiterea raspunsului
            if (strcmp(buffer, "time") == 0)
            {
                time_t t = time(NULL);
                struct tm tm = *localtime(&t);
                sprintf(buffer, "Timpul local este: %02d:%02d:%02d", tm.tm_hour, tm.tm_min, tm.tm_sec);
            }
            else if (strcmp(buffer, "user") == 0)
            {
                struct passwd *pw = getpwuid(getuid());
                const char *username = pw->pw_name;
                sprintf(buffer, "username: %s", username);
            
            }
            else if (strcmp(buffer, "close") == 0)
            {
                sprintf(buffer, "CLOSE");
                send(newSocket, buffer, strlen(buffer), 0);
                printf("Raspuns trimis: %s\n", buffer);
                break;
            }
            else
            {
                sprintf(buffer, "Mesaj necunoscut");
            }

            
            // Trimiterea raspunsului
            send(newSocket, buffer, strlen(buffer), 0);
            printf("Raspuns trimis: %s\n", buffer);
        }

        // Inchiderea conexiunii cu clientul
        close(newSocket);
    }

    return 0;
}