#
# Makefile para nossos programas de teste
# 

CC=gcc
LIB_DIR=../lib

all: identify createOneThread

identify: identify.c $(LIB_DIR)/libcthread.a
	$(CC) -m32 -o identifyExe identify.c -L$(LIB_DIR) -lcthread -Wall

createOneThread: createOneThread.c $(LIB_DIR)/libcthread.a
	$(CC) -m32 -o createOneThreadExe createOneThread.c -L$(LIB_DIR) -lcthread -Wall

clean:
	rm createOneThread identify *.o
