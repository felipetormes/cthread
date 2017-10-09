#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

void print1(void *pointer) {

	int x = *(int*)pointer;
	printf("%d\n", x);

	cyield();

	printf("%d\n", x*x );
}

void print() {

	printf("%d\n", 99);

	cyield();

	printf("%d\n", 99999);
}

int main(int argc, char const *argv[])
{
	int x = 12;
	int tidA = ccreate((void*)&print1, (void*)&x, 0);
	int tidB = ccreate((void*)&print, NULL, 0);
	printf("%d %d\n", tidA, tidB);

	FirstFila2(&control.all_threads);

	//NextFila2(&control.all_threads);

	TCB_t* hehe =  GetAtIteratorFila2(&control.all_threads);

	printf("TID: %d\n", hehe->tid);

	cyield();

	printf("entre threads....\n");

	cyield();

	return 0;
}
