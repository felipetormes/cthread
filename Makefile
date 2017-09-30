all: libcthread clean

libcthread:
	gcc -m32 -c src/cthread.c -Wall
	ar crs lib/libcthread.a cthread.o bin/support.o

clean:
	rm *.o

