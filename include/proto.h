#ifndef PROTO_H
#define PROTO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define DEFAULT_PORT 5050
#define BUF_SIZE 1024
#define MAX_CLIENTS FD_SETSIZE // Máximo de clientes baseado no select()

// Função para encerrar o programa em caso de erro fatal
// Imprime a mensagem de erro fornecida e o erro do sistema (errno)
static inline void die(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}

#endif