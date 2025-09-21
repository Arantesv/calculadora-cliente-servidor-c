#include "proto.h"

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <ip-servidor> <porta>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    if (port <= 0 || port > 65535)
    {
        fprintf(stderr, "Porta inválida: %s\n", argv[2]);
        return EXIT_FAILURE;
    }

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        die("socket");

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons((uint16_t)port);

    if (inet_pton(AF_INET, server_ip, &servaddr.sin_addr) <= 0)
    {
        fprintf(stderr, "Endereço IP inválido: %s\n", server_ip);
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        die("connect");

    fprintf(stderr, "[CLIENTE] Conectado a %s:%d. Digite 'QUIT' para sair.\n", server_ip, port);

    char send_buf[BUF_SIZE];
    char recv_buf[BUF_SIZE];

    while (fgets(send_buf, sizeof(send_buf), stdin) != NULL)
    {
        // Envia a requisição
        if (send(sockfd, send_buf, strlen(send_buf), 0) < 0)
        {
            perror("send");
            break;
        }

        // Comando QUIT: encerra o cliente localmente
        if (strncmp(send_buf, "QUIT", 4) == 0)
        {
            fprintf(stderr, "[CLIENTE] Encerrando.\n");
            break;
        }

        // Recebe a resposta
        ssize_t n = recv(sockfd, recv_buf, sizeof(recv_buf) - 1, 0);
        if (n <= 0)
        {
            if (n < 0)
            {
                perror("recv");
            }
            else
            {
                printf("[CLIENTE] Servidor encerrou a conexão.\n");
            }
            break;
        }

        recv_buf[n] = '\0';
        printf("%s", recv_buf);
    }

    close(sockfd);
    return 0;
}