#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/cdata.h"
#include "../include/cthread.h"
#include "../include/support.h"

void print1() {

  printf("INITIALIZING THE EXECUTION OF THE THREAD %d\n\n", control.running_thread->tid);

	int x = 12;
	printf("%d\n", x);

  //cjoin(2); //dá deadlock

	cyield();

	printf("%d\n", x*x );

	cyield();

	printf("FINISHING THE EXECUTION OF THE THREAD %d\n\n", control.running_thread->tid);
}

void print() {

  printf("INITIALIZING THE EXECUTION OF THE THREAD %d\n\n", control.running_thread->tid);

  FirstFila2(&control.join_threads);

  Dependance* blocked =  GetAtIteratorFila2(&control.join_threads);

  cjoin(blocked->dependsOn);

	printf("%d\n", 99);

	cyield();

	printf("%d\n", 99999);

	cyield();

	printf("FINISHING THE EXECUTION OF THE THREAD %d\n\n", control.running_thread->tid);
}

int main(int argc, char const *argv[])
{

  printf("\nINITIALIZING THREAD MAIN\n\n");

	int tidA = ccreate((void*)&print1, NULL, 0);
	int tidB = ccreate((void*)&print, NULL, 0);

	printf("Running Thread: %d\n\n", control.running_thread->tid);

  FirstFila2(&control.able_threads);

  fprintf(stderr, "ABLE THREADS\n");

  do {
    TCB_t* hehe =  GetAtIteratorFila2(&control.able_threads);
    printf("TID: %d PRIO: %d\n", hehe->tid, hehe->prio);
  } while(NextFila2(&control.able_threads) == 0);
  printf("\n");

	cjoin(tidA);
	printf("BACK TO THREAD MAIN\n");

	cjoin(tidB);
	printf("segunda tem que ter termiando\n");

  printf("FINISHING THE EXECUTION OF THE THREAD MAIN\n\n");


	return 0;
}
