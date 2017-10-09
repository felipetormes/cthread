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

struct sJoinDependance {
	TCB_t *dependant;
	int dependsOn;
};

typedef struct sJoinDependance Dependance;

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

	AppendFila2(&control.all_threads, new_thread);

	AppendFila2(&control.able_threads, new_thread);

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

	TCB_t *running;

	int isRunning = 0;

	if(!FirstFila2(&executando)){

		running = (TCB_t*)GetAtIteratorFila2(&executando);

		if (running->tid == tid)

			isRunning = 1;//uma thread pode escutar por ela mesma ou deveria retornar um erro?
	}


	return isRunning || findTidAptos(&aptos, tid);// || findTidWait(&filaWait, tid) || findTidJoin(&filaJoin, tid);

}

int isAnotherTidWaiting(int tid) {

	int invalid = FirstFila2(&filaJoin);

	Dependance *element;

	while (!invalid){

		element = (Dependance*)GetAtIteratorFila2(&filaJoin);

		if (element->dependsOn == tid)

			return -1;

		invalid = NextFila2(&filaJoin);
	}

	return 0;
}

int cjoin(int tid) {

	if(!exists(tid) || isAnotherTidWaiting(tid)) {

		return -1;
	}

	if(FirstFila2(&executando)) {

		return -1;
	}

	TCB_t *current = (TCB_t*)GetAtIteratorFila2(&executando);

	DeleteAtIteratorFila2(&executando);

	Dependance *newDependance = malloc(sizeof(Dependance));
	newDependance->dependant = current;
	newDependance->dependsOn = tid;

	AppendFila2(&filaJoin, (void*)newDependance);

	if(FirstFila2(&aptos)) {

		return -1;
	}

	TCB_t *next = (TCB_t*)GetAtIteratorFila2(&aptos);

	DeleteAtIteratorFila2(&aptos);

	AppendFila2(&executando, next);

	swapcontext(&(current->context), &(next->context));

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

	if(FirstFila2(&executando)) {

		printf("Erro cyield, erro ou fila vazia\n");

		return -1;
	}

	TCB_t *toYield = (TCB_t*)GetAtIteratorFila2(&executando);

	InsertByPrio(&aptos, toYield);

	DeleteAtIteratorFila2(&executando);

	escalonador(toYield);

	return 0;
}
