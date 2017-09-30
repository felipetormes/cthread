#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"

#include "insert.c"

#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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


int freeTidOnJoin(FILA2 *queue, int id);
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

	// int toFreeTid = executing->tid;

	DeleteAtIteratorFila2(&executando);


	// invalid = FirstFila2(&aptos);
	
	// if(invalid) {

	// 	printf("erro no initEndThreadContext 2");
	// }
	
	

	// TCB_t *element = (TCB_t*)GetAtIteratorFila2(&aptos);

	// AppendFila2(&executando, element);

	// freeTidOnJoin(&filaJoin, toFreeTid);


	// printf("Acabou...escalona...\n");

	//livera quem ta escutando ele

	escalonador(executing);


}

void initEndThreadContext() {

	returnContext = createContext(0, (void*)&finishThread, NULL);
}

int ccreate (void *(*start)(void *), void *arg, int why) {

	static int firstTime = 1; 
	static int tid_count = 0;

	if(firstTime) {

		firstTime=0;

		CreateFila2(&aptos);
		CreateFila2(&executando);
		CreateFila2(&filaJoin);
		CreateFila2(&filaWait);

		if (getcontext(&mainContext)){
			
			printf("Erro criando contexto para a thread %d\n", tid_count);
			return -1;
		}

		TCB_t *mainTCB = malloc(sizeof(TCB_t));

		mainTCB->tid = tid_count++;
		mainTCB->state = PROCST_EXEC;
		mainTCB->prio = 0;
		mainTCB->context = mainContext;

		AppendFila2(&executando, (void*)mainTCB);

		initEndThreadContext();
	}

	ucontext_t* newContext = createContext(returnContext, (void*)start, arg);

	TCB_t *newTCB = malloc(sizeof(TCB_t));

	newTCB->tid = tid_count;
	newTCB->state = PROCST_APTO;
	newTCB->prio = 0;
	newTCB->context = *newContext;

	InsertByPrio(&aptos, newTCB);

	return tid_count++;
}
/*
int freeTidOnJoin(FILA2 *queue, int id){

	int invalid = FirstFila2(queue);

	Dependance *element;

	while (!invalid){

		element = (Dependance*)GetAtIteratorFila2(queue);

		if (element->dependant->tid == id) {

			InsertByPrio(&aptos, element->dependant);

			DeleteAtIteratorFila2(queue);
			invalid = NextFila2(queue);
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

	if (!exists(tid) || isAnotherTidWaiting(tid)) {
	
		return -1;
	}

	if (FirstFila2(&executando)) {

		return -1;
	}

	TCB_t *current = (TCB_t*)GetAtIteratorFila2(&executando);

	if (DeleteAtIteratorFila2(&executando)) {

		return -1;
	}

	Dependance *newDependance = malloc(sizeof(Dependance));
	newDependance->dependant = current;
	newDependance->dependsOn = tid;

	if (AppendFila2(&filaJoin, (void*)newDependance)) {

		return -1;
	}

	if(FirstFila2(&aptos)) {

		return -1;
	}

	TCB_t *next = (TCB_t*)GetAtIteratorFila2(&aptos);

	if (DeleteAtIteratorFila2(&aptos)){
	
		printf("Erro deletando funcao na fila de aptos\n");
	}
	
	if (AppendFila2(&executando, next)){
	
		printf("Erro colocando processo na fila de executando\n");
	}

	swapcontext(&(current->context), &(next->context));

	printf("ok join %d\n", tid);
	
	return 0;
}
*/
void escalonador(TCB_t *oldTCB) {

	if(FirstFila2(&aptos)) {

		printf("erro no escalonador\n");
	}

	TCB_t *nextToExecute = (TCB_t*)GetAtIteratorFila2(&aptos);

	AppendFila2(&executando, nextToExecute);

	DeleteAtIteratorFila2(&aptos);

	swapcontext(&(oldTCB->context), &(nextToExecute->context));
}

int cyield(void) {

	if(FirstFila2(&executando)) {

		printf("erro cyield\n");
	}

	TCB_t *toYield = (TCB_t*)GetAtIteratorFila2(&executando);

	AppendFila2(&aptos, toYield);

	DeleteAtIteratorFila2(&executando);

	escalonador(toYield);

	return 0;//return error
}