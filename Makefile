all: libcthread

libcthread: cthread cdata
	ar crs lib/libcthread.a bin/cthread.o bin/cdata.o bin/support.o

cthread: src/cthread.c
	gcc -m32 -c -o bin/cthread.o src/cthread.c

cdata: src/cdata.c
	gcc -m32 -c -o bin/cdata.o src/cdata.c

clean:
	rm bin/cthread.o bin/cdata.o lib/libcthread.a
