#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sstream>

#define PORT 8080 // Definește portul pe care serverul va asculta

int main()
{
    int sock = 0;
    struct sockaddr_in servAddr;
    char buffer[1024] = {0}; // Buffer pentru a stoca mesajele primite de la server
    char input[1024];        // Buffer pentru a stoca mesajele introduse de utilizator

    // Creare socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Eroare la crearea socket-ului");
        return -1;
    }

    servAddr.sin_family = AF_INET;   // Setează familia de adrese la IPv4
    servAddr.sin_port = htons(PORT); // Setează portul

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

    // Trimite mesajul initial de identificare catre server
    char initMessage[] = "Client 1";
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

    fd_set readfds;         // Set de descriptori pentru select
    struct timeval timeout; // Structura pentru timeout

    // Afiseaza prompt-ul o data inainte de bucla
    printf("Introdu mesajul [format: <image_path> <option> sau scrie 'exit' pentru a iesi]: ");
    fflush(stdout);

    // Bucla principala pentru comunicare
    while (1)
    {
        FD_ZERO(&readfds);              // Curata setul de descriptori
        FD_SET(STDIN_FILENO, &readfds); // Adauga stdin (intrarea standard) in set
        FD_SET(sock, &readfds);         // Adauga socket-ul in set

        timeout.tv_sec = 1; // Timeout de 1 secunda
        timeout.tv_usec = 0;

        // Foloseste select pentru a astepta evenimente pe socket sau stdin
        int activity = select(sock + 1, &readfds, NULL, NULL, &timeout);

        if (activity < 0)
        {
            perror("Eroare la select");
            break;
        }

        // Daca serverul a trimis un mesaj
        if (FD_ISSET(sock, &readfds))
        {
            memset(buffer, 0, sizeof(buffer));                  // Reseteaza buffer-ul
            bytesRead = read(sock, buffer, sizeof(buffer) - 1); // Citeste mesajul de la server
            if (bytesRead > 0)
            {
                buffer[bytesRead] = '\0'; // Termina string-ul primit
                if (strcmp(buffer, "Server is shutting down.") == 0)
                {
                    printf("%s\n", buffer);
                    break; // Iese din bucla daca serverul se inchide
                }
                printf("Raspuns primit: %s\n", buffer);
                // Afiseaza prompt-ul din nou dupa ce primeste un raspuns valid
                printf("Introdu mesajul [format: <image_path> <option> sau scrie 'exit' pentru a iesi]: ");
                fflush(stdout);
            }
            else if (bytesRead == 0)
            {
                printf("Serverul a inchis conexiunea.\n");
                break; // Iese din bucla daca serverul inchide conexiunea
            }
            else
            {
                perror("Eroare la citire");
                break; // Iese din bucla daca apare o eroare la citire
            }
        }

        // Daca utilizatorul a introdus un mesaj
        if (FD_ISSET(STDIN_FILENO, &readfds))
        {
            fgets(input, sizeof(input), stdin); // Citeste input-ul de la utilizator
            input[strcspn(input, "\n")] = 0;    // Elimina caracterul de newline

            if (strcmp(input, "exit") == 0)
            {
                printf("Iesire...\n");
                break; // Iese din bucla daca utilizatorul introduce 'exit'
            }

            // Verifica daca formatul input-ului este corect
            std::istringstream iss(input);
            std::string imagePath, option;
            iss >> imagePath >> option;

            // Verifica daca calea imaginii si optiunea sunt valide
            if (imagePath.empty() || option.empty() ||
                (option != "-c" && option != "-g" && option != "-r"))
            {
                printf("Input invalid. Folositi formatul '<image_path> <option>' unde option este -c, -g, sau -r.\n");
                // Afiseaza prompt-ul din nou pentru input invalid
                printf("Introdu mesajul [format: <image_path> <option> sau scrie 'exit' pentru a iesi]: ");
                fflush(stdout);
                continue; // Continua bucla daca input-ul este invalid
            }

            // Trimite mesajul catre server
            send(sock, input, strlen(input), 0);
            printf("Mesaj trimis: %s\n", input);

            // Afiseaza prompt-ul din nou dupa trimiterea unui mesaj valid
            printf("Introdu mesajul [format: <image_path> <option> sau scrie 'exit' pentru a iesi]: ");
            fflush(stdout);
        }
    }

    // Inchide conexiunea
    close(sock);
    return 0;
}
