# Calculadora Cliente-Servidor em C

> Uma aplicação cliente-servidor para execução de cálculos matemáticos remotos, desenvolvida em C com sockets TCP/IP em um ambiente Linux.

Este projeto implementa uma calculadora onde múltiplos clientes podem se conectar a um servidor concorrente para realizar as quatro operações matemáticas básicas.

## Funcionalidades

* **Cliente e Servidor TCP/IP:** Comunicação usando sockets IPv4.
* **Protocolo Flexível:** Aceita operações em formato de prefixo (`ADD 10 5`) e infixo (`10 + 5`).
* **Comandos de Utilidade:** Inclui comandos `VERSION` e `HELP` para informações e ajuda.
* **Servidor Concorrente:** Capaz de atender múltiplos clientes simultaneamente usando a chamada de sistema `select()`.
* **Parsing Robusto:** Valida a entrada do usuário e retorna códigos de erro claros para operações inválidas ou divisões por zero.
* **Build Automatizado:** `Makefile` completo para compilar o projeto e limpar os artefatos.
* **Testes Automatizados:** Testes em Shell Script para validar o comportamento do servidor e garantir a resposta correta.

## Estrutura do Projeto

```
.
├── Makefile              # Automatiza a compilação e os testes
├── README.md             # Este arquivo
├── bin/                  # Diretório para os executáveis (criado pelo make)
│   ├── client
│   └── server
├── include/
│   └── proto.h           # Cabeçalho com definições comuns
├── src/
│   ├── client.c          # Código fonte do cliente
│   └── server.c          # Código fonte do servidor
└── tests/
    ├── test_runner.sh             # Script de teste (espera sucesso)
    └── test_failure_example.sh    # Script de teste (espera falha)
```

## Compilação e Execução

### Pré-requisitos

* Um ambiente Linux ou um subsistema como o WSL no Windows.
* Compilador C (`gcc`).

### Compilação

Na pasta raiz do projeto, execute o seguinte comando:

```bash
# Para compilar o cliente e o servidor
make all
```

### Execução

A aplicação precisa de dois terminais: um para o servidor e outro para o cliente.

**1. No primeiro terminal, inicie o servidor:**

```bash
# Iniciar na porta padrão (5050)
./bin/server

# Ou iniciar em uma porta específica
./bin/server 8000
```
O servidor ficará em execução, aguardando por conexões.

**2. No segundo terminal, inicie o cliente e conecte-se ao servidor:**

```bash
# Conectar ao servidor local (localhost) na porta 5050
./bin/client 127.0.0.1 5050

# Conectar ao servidor na porta 8000
./bin/client 127.0.0.1 8000
```

## Protocolo de Comunicação

### Formato das Requisições

O cliente pode enviar os seguintes comandos, terminados por uma nova linha (`\n`):

* **Cálculo (Prefixo):** `OP A B` onde `OP` é `ADD`, `SUB`, `MUL`, `DIV`.
* **Cálculo (Infixo):** `A op B` onde `op` é `+`, `-`, `*`, `/`.
* **Versão:** `VERSION`
* **Ajuda:** `HELP`
* **Sair:** `QUIT` (encerra o cliente)

### Formato das Respostas

O servidor sempre responde com uma única linha:

* **Sucesso:** `OK <resultado>`
* **Erro:** `ERR <codigo> <mensagem>`
    * `EINV`: Entrada Inválida (formato ou operação desconhecida).
    * `EZDV`: Divisão por Zero.
    * `ESRV`: Erro interno do servidor (não utilizado nesta implementação).

### Exemplo de Interação

```
# Cliente envia:
ADD 20 22
# Servidor responde:
OK 42.000000

# Cliente envia:
100 / 3
# Servidor responde:
OK 33.333333

# Cliente envia:
DIV 5 0
# Servidor responde:
ERR EZDV divisao_por_zero
```

### Decisões de Projeto e Limitações Conhecidas

* **Calculadora de Operação Única**
    A calculadora foi projetada para ser um processador simples de requisições, operando em um formato `A <op> B`. Ela não é um parser de expressões matemáticas completas e, portanto, não avalia a precedência de operadores (ex.: em `10 + 5 * 2`, a multiplicação não seria executada primeiro). Cada linha é tratada como uma única operação independente.

* **Comando `QUIT` e Desconexão do Servidor**
    O comando `QUIT` não é uma instrução para o servidor, mas sim um comando para que o **próprio cliente** encerre a conexão. O servidor, por sua vez, é simples e não guarda informações sobre os clientes, para ele, `QUIT` é apenas um comando desconhecido. Da mesma forma, se o servidor for desligado (com `Ctrl+C`), o cliente não é notificado imediatamente. Ele só descobre que a conexão foi perdida na próxima vez que tenta enviar um comando. Nesse momento, a tentativa de envio falha e a mensagem `[CLIENTE] Servidor encerrou a conexão.` aparece.

    Uma alternativa em um projeto mais complexo seria o servidor reconhecer `QUIT`, responder com uma mensagem de despedida (`OK Encerrando conexão`) e fechar sua ponta da conexão.

* **Modelo de Concorrência com `select()`**
    Escolhemos `select()` por ser a abordagem mais eficiente para este projeto. Ele permite que um único processo do servidor gerencie todos os clientes, consumindo o mínimo de memória e recursos do sistema. As alternativas, como criar uma cópia inteira do servidor (`fork`) para cada nova conexão, seriam um exagero para uma tarefa tão rápida e simples como a de uma calculadora.

## Testes Automatizados

O sistema de testes consiste em um único script executável `tests/run_tests.sh`. Ao ser executado, ele realiza os seguintes passos:

1.  **Gera Arquivos Temporários:** Cria dinamicamente um arquivo de entrada (`test_cases.txt`) com uma lista predefinida de comandos e um arquivo de saída (`expected_output.txt`) com os resultados exatos que o servidor deve retornar.
2.  **Inicia o Servidor:** Roda o processo do servidor em segundo plano.
3.  **Executa o Cliente:** Roda o cliente, usando o arquivo de entrada como script de comandos e salvando toda a sua saída em um arquivo de resultado (`actual_output.txt`).
4.  **Compara os Resultados:** Utiliza o comando `diff` para comparar o resultado obtido (`actual_output.txt`) com o resultado esperado (`expected_output.txt`).
5.  **Reporta o Status:** Informa se todos os testes passaram (se os arquivos forem idênticos) ou se houve falha (mostrando as diferenças encontradas).
6.  **Limpeza:** Ao final, o servidor é encerrado e todos os arquivos temporários criados são deletados.

### Como Executar os Testes

Para rodar executar o script, utilize o `Makefile` enviando o seguinte comando na raiz do projeto:

```bash
make test
```

Este comando garante que o projeto seja compilado, se necessário, e em seguida executa o script de teste.

## Autores

* **Vitor Arantes Vendramini - RA: 10417759**
* **Leonardo Patriani Cardoso - RA: 10417188**