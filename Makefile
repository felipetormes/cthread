all: libcthread

libcthread:
	gcc -m32 -c src/cthread.c src/cdata.c -Wall
	ar crs lib/libcthread.a cthread.o cdata.o bin/support.o

clean:
	rm *.o lib/libcthread.a
