#include "proto.h"

// Variável global para controlar o loop principal (para o signal handler)
volatile sig_atomic_t running = 1;

void handle_sigint(int sig)
{
    (void)sig; // Evita warning de 'unused parameter'
    printf("\n[SERVIDOR] Recebido sinal de interrupção. Desligando...\n");
    running = 0;
}

void process_request(const char *req, char *res)
{
    char op_str[16];
    char op_char;
    double a, b, result;

    // 1. Checa por comandos 'HELP' ou 'VERSION':
    if (strcmp(req, "HELP") == 0)
    {
        sprintf(res, "OK Comandos: ADD/SUB/MUL/DIV A B | A op B | VERSION | HELP | QUIT\n");
        return;
    }
    if (strcmp(req, "VERSION") == 0)
    {
        sprintf(res, "OK Calculadora Cliente-Servidor v1.0 | Desenvolvido por Vitor Arantes e Leonardo Patriani\n");
        return;
    }

    // 2. Tenta fazer o parsing no formato de PREFIXO (ex: "ADD 10 2")
    if (sscanf(req, "%15s %lf %lf", op_str, &a, &b) == 3)
    {
        if (strcmp(op_str, "ADD") == 0)
        {
            result = a + b;
        }
        else if (strcmp(op_str, "SUB") == 0)
        {
            result = a - b;
        }
        else if (strcmp(op_str, "MUL") == 0)
        {
            result = a * b;
        }
        else if (strcmp(op_str, "DIV") == 0)
        {
            if (b == 0.0)
            {
                sprintf(res, "ERR EZDV divisao_por_zero\n");
                return;
            }
            result = a / b;
        }
        else
        {
            sprintf(res, "ERR EINV operacao_desconhecida\n");
            return;
        }

        sprintf(res, "OK %.6f\n", result);
        return;
    }

    // 3. Se o formato de prefixo falhou, tenta o formato INFIXO (ex: "10 + 2")
    if (sscanf(req, "%lf %c %lf", &a, &op_char, &b) == 3)
    {
        switch (op_char)
        {
        case '+':
            result = a + b;
            break;
        case '-':
            result = a - b;
            break;
        case '*':
            result = a * b;
            break;
        case '/':
            if (b == 0.0)
            {
                sprintf(res, "ERR EZDV divisao_por_zero\n");
                return;
            }
            result = a / b;
            break;
        default:
            sprintf(res, "ERR EINV operacao_desconhecida\n");
            return;
        }

        sprintf(res, "OK %.6f\n", result);
        return;
    }

    // 4. Se nenhum dos formatos funcionou, retorna erro
    sprintf(res, "ERR EINV entrada_invalida\n");
}

int main(int argc, char **argv)
{
    int port = DEFAULT_PORT;
    if (argc > 2)
    {
        fprintf(stderr, "Uso: %s [<porta>]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 2)
    {
        port = atoi(argv[1]);
        if (port <= 0 || port > 65535)
        {
            fprintf(stderr, "Porta inválida: %d\n", port);
            return EXIT_FAILURE;
        }
    }

    // Configura o handler para SIGINT (Ctrl+C)
    signal(SIGINT, handle_sigint);

    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
        die("socket");

    // Permite reutilizar o endereço local rapidamente (evita "Address already in use")
    int opt = 1;
    if (setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        perror("setsockopt");
    }

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons((uint16_t)port);

    if (bind(listen_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
        die("bind");
    if (listen(listen_fd, 10) < 0)
        die("listen");

    printf("[SERVIDOR] Ouvindo na porta %d\n", port);

    int clients[MAX_CLIENTS];
    for (int i = 0; i < MAX_CLIENTS; i++)
        clients[i] = -1;

    fd_set all_fds, read_fds;
    FD_ZERO(&all_fds);
    FD_SET(listen_fd, &all_fds);
    int max_fd = listen_fd;

    char buf[BUF_SIZE], response[BUF_SIZE];

    while (running)
    {
        read_fds = all_fds;

        // select() aguarda por atividade em algum dos sockets
        int nready = select(max_fd + 1, &read_fds, NULL, NULL, NULL);
        if (nready < 0)
        {
            if (errno == EINTR)
                continue; // Interrompido por sinal, continuar
            die("select");
        }

        // 1. Checa por novas conexões
        if (FD_ISSET(listen_fd, &read_fds))
        {
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            int conn_fd = accept(listen_fd, (struct sockaddr *)&cliaddr, &clilen);
            if (conn_fd < 0)
            {
                perror("accept");
            }
            else
            {
                char ip_str[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &cliaddr.sin_addr, ip_str, sizeof(ip_str));
                printf("[SERVIDOR] Nova conexão de %s:%d (fd=%d)\n", ip_str, ntohs(cliaddr.sin_port), conn_fd);

                int i;
                for (i = 0; i < MAX_CLIENTS; i++)
                {
                    if (clients[i] < 0)
                    {
                        clients[i] = conn_fd;
                        break;
                    }
                }

                if (i == MAX_CLIENTS)
                {
                    fprintf(stderr, "[SERVIDOR] Muitos clientes. Rejeitando.\n");
                    close(conn_fd);
                }
                else
                {
                    FD_SET(conn_fd, &all_fds);
                    if (conn_fd > max_fd)
                        max_fd = conn_fd;
                }
            }
        }

        // 2. Checa por dados dos clientes existentes
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            int client_fd = clients[i];
            if (client_fd < 0)
                continue;

            if (FD_ISSET(client_fd, &read_fds))
            {
                ssize_t n = recv(client_fd, buf, sizeof(buf) - 1, 0);

                if (n <= 0)
                { // Conexão fechada ou erro
                    if (n < 0)
                        perror("recv");
                    printf("[SERVIDOR] Cliente (fd=%d) desconectou.\n", client_fd);
                    close(client_fd);
                    FD_CLR(client_fd, &all_fds);
                    clients[i] = -1;
                }
                else
                {
                    buf[n] = '\0';
                    // Remove \r\n ou \n do final da string, se houver
                    buf[strcspn(buf, "\r\n")] = 0;

                    printf("[SERVIDOR] Recebido de fd=%d: \"%s\"\n", client_fd, buf);

                    process_request(buf, response);

                    printf("[SERVIDOR] Enviando para fd=%d: \"%s\"", client_fd, response);
                    send(client_fd, response, strlen(response), 0);
                }
            }
        }
    }

    // Desligamento
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] >= 0)
            close(clients[i]);
    }
    close(listen_fd);
    printf("[SERVIDOR] Servidor desligado.\n");
    return 0;
}