#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fstream>

#define PORT 8080
const char *LOCK_FILE = "/tmp/clienta.lock"; // Defineste calea pentru fisierul de blocare

// Functie pentru a elimina fisierul de blocare
void remove_lock_file()
{
    std::remove(LOCK_FILE);
}

int main()
{
    // Verifica daca fisierul de blocare exista
    std::ifstream lock_file_in(LOCK_FILE);
    if (lock_file_in.good())
    {
        fprintf(stderr, "Admin client ruleaza deja.\n");
        return 1;
    }
    lock_file_in.close();

    // Creeaza fisierul de blocare
    std::ofstream lock_file_out(LOCK_FILE);
    if (!lock_file_out)
    {
        fprintf(stderr, "Nu se poate crea fisierul de blocare.\n");
        return 1;
    }
    lock_file_out << getpid(); // Scrie ID-ul procesului curent in fisierul de blocare
    lock_file_out.close();

    // Asigura eliminarea fisierului de blocare la iesire
    atexit(remove_lock_file);

    int sock = 0;
    struct sockaddr_in servAddr;
    char buffer[1024] = {0};
    char input[1024];

    // Creare socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la crearea socket-ului");
        return -1;
    }

    servAddr.sin_family = AF_INET;   // Seteaza familia de adrese la IPv4
    servAddr.sin_port = htons(PORT); // Seteaza portul

    // Converteste adresa serverului de la text la binar
    if (inet_pton(AF_INET, "127.0.0.1", &servAddr.sin_addr) <= 0)
    {
        perror("Adresa invalida");
        return -1;
    }

    // Conectare la server
    if (connect(sock, (struct sockaddr *)&servAddr, sizeof(servAddr)) < 0)
    {
        perror("Conexiune esuata");
        return -1;
    }

    // Trimite mesajul initial de identificare
    char initMessage[] = "Admin client";
    send(sock, initMessage, strlen(initMessage), 0);

    // Asteapta raspunsul initial de la server
    memset(buffer, 0, sizeof(buffer));
    int bytesRead = read(sock, buffer, sizeof(buffer) - 1);
    if (bytesRead > 0)
    {
        buffer[bytesRead] = '\0'; // Termina string-ul primit cu null
        printf("Raspuns primit: %s\n", buffer);
    }
    else if (bytesRead == 0)
    {
        printf("Serverul a inchis conexiunea.\n");
        close(sock); // Inchide socket-ul
        return 0;
    }
    else
    {
        perror("Eroare la citire");
        close(sock); // Inchide socket-ul
        return -1;
    }

    // Asteapta input-ul utilizatorului
    while (1)
    {
        printf("Introdu comanda [close, message]: ");
        fflush(stdout);

        // Citeste input-ul de la utilizator
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Elimina caracterul de newline

        if (strcmp(input, "close") == 0)
        {
            printf("CLOSE\n");
            send(sock, input, strlen(input), 0); // Trimite comanda de inchidere
            break;
        }

        // Trimite mesajul catre server
        send(sock, input, strlen(input), 0);
        printf("Mesaj trimis: %s\n", input);

        // Asteapta raspunsul de la server
        memset(buffer, 0, sizeof(buffer));
        bytesRead = read(sock, buffer, sizeof(buffer) - 1);
        if (bytesRead > 0)
        {
            buffer[bytesRead] = '\0'; // Termina string-ul primit cu null
            printf("Raspuns primit: %s\n", buffer);
        }
        else if (bytesRead == 0)
        {
            printf("Serverul a inchis conexiunea.\n");
            break;
        }
        else
        {
            perror("Eroare la citire");
            break;
        }
    }

    // Inchide conexiunea
    close(sock);
    return 0;
}
