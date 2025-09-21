CC = gcc
CFLAGS = -Wall -Wextra -g -O2 -Iinclude

SRCDIR = src
INCDIR = include
BINDIR = bin

CLIENT_SRC = $(SRCDIR)/client.c
SERVER_SRC = $(SRCDIR)/server.c

CLIENT_BIN = $(BINDIR)/client
SERVER_BIN = $(BINDIR)/server

all: $(BINDIR) $(CLIENT_BIN) $(SERVER_BIN)

$(BINDIR):
	@mkdir -p $(BINDIR)

$(SERVER_BIN): $(SERVER_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_SRC)
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

server: $(SERVER_BIN)

client: $(CLIENT_BIN)

clean:
	@echo "Limpando arquivos compilados..."
	@rm -rf $(BINDIR)	

test: all
	@echo "Execução de testes iniciada..."
	@chmod +x ./tests/run_tests.sh
	@./tests/run_tests.sh
