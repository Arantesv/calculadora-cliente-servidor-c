#!/bin/bash

# Encerra o script se qualquer comando falhar
set -e

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

# Navega para o diretório do script para que os caminhos relativos funcionem
cd "$(dirname "$0")"

SERVER_BIN="../bin/server"
CLIENT_BIN="../bin/client"
PORT=5555
HOST="127.0.0.1"

INPUT_FILE="test_cases.txt"
EXPECTED_OUTPUT_FILE="expected_output.txt"
ACTUAL_OUTPUT_FILE="actual_output.txt"

# Cria o arquivo com os casos de teste que o cliente irá enviar
cat > "$INPUT_FILE" << EOT
VERSION
HELP
ADD 10 5
100 / 4
SUB 7.5 10
-5 * 3
DIV 10 0
DIV 10
UNKNOWN 1 2
10 x 2
QUIT
EOT

# Cria o arquivo com os resultados exatos que esperamos receber do servidor
cat > "$EXPECTED_OUTPUT_FILE" << EOT
OK Calculadora Cliente-Servidor v1.0 | Desenvolvido por Vitor Arantes e Leonardo Patriani
OK Comandos: ADD/SUB/MUL/DIV A B | A op B | VERSION | HELP | QUIT
OK 15.000000
OK 25.000000
OK -2.500000
OK -15.000000
ERR EZDV divisao_por_zero
ERR EINV entrada_invalida
ERR EINV operacao_desconhecida
ERR EINV operacao_desconhecida
EOT


cleanup() {
    echo "Encerrando o servidor (PID: $SERVER_PID)..."
    kill "$SERVER_PID" 2>/dev/null || true
    # Apaga os arquivos temporários criados por este script
    rm -f "$INPUT_FILE" "$EXPECTED_OUTPUT_FILE" "$ACTUAL_OUTPUT_FILE"
}

# Garante que a função cleanup seja chamada quando o script terminar
trap cleanup EXIT

echo "Iniciando o servidor na porta $PORT..."
"$SERVER_BIN" "$PORT" >/dev/null 2>&1 &
SERVER_PID=$!
sleep 0.5

echo "Executando o cliente com os casos de teste..."
"$CLIENT_BIN" "$HOST" "$PORT" < "$INPUT_FILE" > "$ACTUAL_OUTPUT_FILE"

echo "Comparando a saída atual com a saída esperada..."
if diff -u "$EXPECTED_OUTPUT_FILE" "$ACTUAL_OUTPUT_FILE"; then
    echo -e "\n${GREEN}TODOS OS TESTES PASSARAM!${NC}\n"
else
    echo -e "\n${RED}FALHA NOS TESTES! A saída difere do esperado.${NC}\n"
    echo "Veja as diferenças acima."
    exit 1
fi
