#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

void print1() {

	FirstFila2(&control.able_threads);

	do {
		TCB_t* haha =  GetAtIteratorFila2(&control.able_threads);
		printf("TID: %d PRIO: %d\n", haha->tid, haha->prio);
	} while(NextFila2(&control.able_threads) == 0);

	cyield();

	//printf("%d\n", x*x );
}

void print() {

	FirstFila2(&control.able_threads);

	do {
		TCB_t* hihi =  GetAtIteratorFila2(&control.able_threads);
		printf("TID: %d PRIO: %d\n", hihi->tid, hihi->prio);
	} while(NextFila2(&control.able_threads) == 0);

	cyield();

	printf("%d\n", 99999);
}

int main(int argc, char const *argv[])
{
	int tidA = ccreate((void*)&print1, NULL, 0);
	int tidB = ccreate((void*)&print, NULL, 0);
	int tidC = ccreate((void*)&print, NULL, 0);
	printf("%d %d %d\n", tidA, tidB, tidC);

	printf("Running Thread: %d\n", control.running_thread->tid);

	FirstFila2(&control.able_threads);

	do {
		TCB_t* hehe =  GetAtIteratorFila2(&control.able_threads);
		printf("TID: %d PRIO: %d\n", hehe->tid, hehe->prio);
	} while(NextFila2(&control.able_threads) == 0);

	//NextFila2(&control.all_threads);

	//TCB_t* hehe =  GetAtIteratorFila2(&control.all_threads);

	//printf("PRIO: %d\n", hehe->prio);

	cyield();

	//printf("entre threads....\n");

	//cyield();

	return 0;
}
