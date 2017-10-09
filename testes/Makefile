#
# Makefile para nossos programas de teste
#

CC=gcc
LIB_DIR=../lib

all: identify createOneThread simpleCJoinTest

identify: identify.c $(LIB_DIR)/libcthread.a
	$(CC) -m32 -o identify identify.c -L$(LIB_DIR) -lcthread -Wall

createOneThread: createOneThread.c $(LIB_DIR)/libcthread.a
	$(CC) -m32 -o createOneThread createOneThread.c -L$(LIB_DIR) -lcthread -Wall

simpleCJoinTest: simpleCJoinTest.c $(LIB_DIR)/libcthread.a
	$(CC) -m32 -o simpleCJoinTest simpleCJoinTest.c -L$(LIB_DIR) -lcthread -Wall


clean:
	rm createOneThread identify *.o
