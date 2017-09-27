#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretótio "cthread"
#

CC=gcc
CFLAGS=-Wall -g
LIB_DIR=./lib/
INC_DIR=./include/
BIN_DIR=./bin/
SRC_DIR=./src/

all: $(BIN_DIR)cthread.o

$(BIN_DIR)cthread.o: $(SRC_DIR)cthread.c
			$(CC) $(CFLAGS) -c -o $(BIN_DIR)cthread.o -I$(INC_DIR) $(SRC_DIR)cthread.c

clean:
	rm -rf $(BIN_DIR)cthread.o
