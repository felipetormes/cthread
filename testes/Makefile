CC=gcc
INC_DIR=../include/
BIN_DIR=../bin/

all: identify

identify: identify.c $(BIN_DIR)cthread.o
		$(CC) -o identify.out identify.c $(INC_DIR)cthread.h $(BIN_DIR)cthread.o -Wall



clean:
		rm -r *.out
