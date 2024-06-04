#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080

int main()
{
    int sock = 0;
    struct sockaddr_in servAddr;
    char buffer[1024] = {0};
    char input[1024];

    // Crearea socketului
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la crearea socket-ului");
        return -1;
    }

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(PORT);

    // Convertirea adresei IP a serverului din formatul de string la formatul de structura
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0)
    {
        perror("Adresa invalida");
        return -1;
    }

    // Conectarea la server
    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Eroare la conectarea la server");
        return -1;
    }

    // Astepare input de la utilizator
    while (1)
    {
        printf("Introduceti comanda [user, time, close]: ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; /*  elimin caracterul pentru noua linie, dupa ce introduc comanda si dau enter se adauga si "\n" la final si nu ia comenda cum trebuie */

        if (strcmp(input, "close") == 0)
        {
            printf("CLOSE\n");
            break;
        }

        // Trimiterea mesajului
        send(sock, input, strlen(input), 0);
        printf("Mesaj trimis: %s\n", input);

        // Asteptare raspuns
        memset(buffer, 0, sizeof(buffer));
        read(sock, buffer, sizeof(buffer) - 1);
        printf("Răspuns primit: %s\n", buffer);
    }

    // Inchidere conexiune
    close(sock);
    return 0;
}

// primul comm