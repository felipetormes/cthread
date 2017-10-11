#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"

#include "insert.c"

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _control_threads control = {.init = FALSE};

#define MEM 100000

FILA2 aptos;
FILA2 executando;

//bloqueados
FILA2 filaJoin;
FILA2 filaWait;

ucontext_t mainContext;
ucontext_t* returnContext;


int freeTidOnJoin(int id);
void escalonador(TCB_t *oldTCB);

int cidentify(char *name, int size){

  char* str ="\nAlfeu Uzai Tavares\nEduardo Bassani Chandelier\nFelipe Barbosa Tormes\n\n";

  memcpy(name, str, size);
	if (size == 0)
		name[0] = '\0';
	else
		name[size-1] = '\0';
	return 0;

}

TCB_t* getRemoveFirst(FILA2 *queue) {

	if(FirstFila2(queue)) {

		return NULL;
	}

	TCB_t *next = (TCB_t*)GetAtIteratorFila2(&aptos);

	DeleteAtIteratorFila2(&aptos);

	return next;
}

ucontext_t* createContext(ucontext_t* returnContext, void (*func)(), int *argc) {

	ucontext_t* newContext = (ucontext_t*)malloc(sizeof(ucontext_t));
	getcontext(newContext);
	newContext->uc_link= returnContext;
	newContext->uc_stack.ss_sp=malloc(MEM);
	newContext->uc_stack.ss_size=MEM;
	newContext->uc_stack.ss_flags=0;

	if(argc) {

		makecontext(newContext, (void (*)(void))func, 1, argc);

	} else {

		makecontext(newContext, (void (*)(void))func, 0);
	}

	return newContext;
}

void finishThread() {

	int invalid = FirstFila2(&executando);

	if(invalid) {

		printf("erro no initEndThreadContext\n");
	}

	TCB_t *executing = (TCB_t*)GetAtIteratorFila2(&executando);

	int toFreeTid = executing->tid;

	DeleteAtIteratorFila2(&executando);

	freeTidOnJoin(toFreeTid);

	//next: freeTidOnWait

	escalonador(executing);


}

void initEndThreadContext() {

	returnContext = createContext(0, (void*)&finishThread, NULL);
}

int ccreate(void* (*start)(void*), void *arg, int prio){
	TCB_t* new_thread;

	/* Check if internal variables was initialized */
	if(control.init == FALSE)
		init();

	/* Making thread context */
	new_thread = (TCB_t*) malloc(sizeof(TCB_t));
	getcontext(&new_thread->context);
	new_thread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	new_thread->context.uc_stack.ss_size = SIGSTKSZ;
	new_thread->context.uc_link = &control.ended_thread;
	makecontext(&new_thread->context, (void (*)(void))start, 1, arg);

	/* Changing TCB fields */

	LastFila2(&control.all_threads);
	TCB_t* last_tid = GetAtIteratorFila2(&control.all_threads);

	new_thread->tid = (last_tid->tid)+1;
	new_thread->state = PROCST_APTO;
	new_thread->prio = 0;

	AppendFila2(&control.all_threads, new_thread);

	InsertByPrio(&control.able_threads, new_thread);

	/* Return the Thread Identifier*/
	return new_thread->tid;
}

int freeTidOnJoin(int id){

	int invalid = FirstFila2(&filaJoin);

	Dependance *element;

	while (!invalid){

		element = (Dependance*)GetAtIteratorFila2(&filaJoin);

		if (element->dependsOn == id) {

			InsertByPrio(&aptos, element->dependant);

			DeleteAtIteratorFila2(&filaJoin);

			invalid = 1;

		} else {

			invalid = NextFila2(&filaJoin);
		}
	}
	return 0;
}

int findTidAptos(FILA2 *queue, int id){

	int invalid = FirstFila2(queue);

	TCB_t *element;

	while (!invalid){

		element = (TCB_t*)GetAtIteratorFila2(queue);

		if (element->tid == id) {

			return 1;

		} else {

			invalid = NextFila2(queue);
		}
	}
	return 0;
}


int exists(int tid){

	FirstFila2(&control.all_threads);

	do {
		TCB_t* thread =  GetAtIteratorFila2(&control.all_threads);
		if(thread->tid == tid) {
			return TRUE;
		}
	} while(NextFila2(&control.all_threads) == 0);

	return FALSE;

}

int isAnotherTidWaiting(int tid) {

	int invalid = FirstFila2(&control.join_threads);

	Dependance* element;

	while (!invalid){

		element = (Dependance*)GetAtIteratorFila2(&control.join_threads);

		if (element->dependsOn == tid)

			return TRUE;

		invalid = NextFila2(&control.join_threads);
	}

	return FALSE;
}

int cjoin(int tid) {

	printf("CJOIN THREAD %d - WAITING FOR THREAD %d\n\n", control.running_thread->tid, tid);

	if((exists(tid)==FALSE) || (isAnotherTidWaiting(tid)==TRUE)) {
		printf("THERE'S ALREADY A THREAD WAITING FOR THREAD %d OR THREAD %d DOESN'T EXIST\n\n", tid, tid);
		return FALSE;
	}

	if(control.running_thread->tid == tid) {

		return FALSE;
	}

	control.running_thread->state = PROCST_BLOQ;
	control.running_thread->prio = control.running_thread->prio + stopTimer();

	Dependance* newDependance = (Dependance*) malloc(sizeof(Dependance));
	newDependance->dependant = control.running_thread;
	newDependance->dependsOn = tid;

	AppendFila2(&control.join_threads, newDependance);

	FirstFila2(&control.join_threads);

	printf("BLOCKED THREADS\n");

	do {
		Dependance* hehe =  GetAtIteratorFila2(&control.join_threads);
		printf("TID: %d PRIO: %d - Waiting for TID: %d\n", hehe->dependant->tid, hehe->dependant->prio, hehe->dependsOn);
	} while(NextFila2(&control.join_threads) == 0);
	printf("\n");

	dispatcher();

	/*if(FirstFila2(&aptos)) {

		return -1;
	}

	TCB_t *next = (TCB_t*)GetAtIteratorFila2(&aptos);

	DeleteAtIteratorFila2(&aptos);

	AppendFila2(&executando, next);

	swapcontext(&(current->context), &(next->context));*/

	return 0;
}

void escalonador(TCB_t *oldTCB) {

	if(FirstFila2(&aptos)) {

		printf("erro no escalonador\n");

		return;
	}

	TCB_t *nextToExecute = (TCB_t*)GetAtIteratorFila2(&aptos);

	FirstFila2(&executando);

	AppendFila2(&executando, nextToExecute);

	DeleteAtIteratorFila2(&aptos);

	swapcontext(&(oldTCB->context), &(nextToExecute->context));
}

int cyield(void) {

	printf("CYIELD\n\n");

	FirstFila2(&control.able_threads);

	fprintf(stderr, "ABLE THREADS\n");

	do {
		TCB_t* hehe =  GetAtIteratorFila2(&control.able_threads);
		printf("TID: %d PRIO: %d\n", hehe->tid, hehe->prio);
	} while(NextFila2(&control.able_threads) == 0);
	printf("\n");

	dispatcher();
	return TRUE;
}
