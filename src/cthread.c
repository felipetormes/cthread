#include "../include/cthread.h"
#include "../include/cdata.h"
#include "../include/support.h"
#include <signal.h>

#include "insert.c"
#include <ucontext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct _control_threads control = {.init = FALSE};

int cidentify(char *name, int size){

  /* Check if internal variables was initialized */
	if(control.init == FALSE)
		init();

  char* str ="\nAlfeu Uzai Tavares\nEduardo Bassani Chandelier\nFelipe Barbosa Tormes\n\n";

  memcpy(name, str, size);
	if (size == 0)
		name[0] = '\0';
	else
		name[size-1] = '\0';
	return 0;

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

int cjoin(int tid) {

  /* Check if internal variables was initialized */
	if(control.init == FALSE)
		init();

  #if DEBUG == TRUE
	printf("CJOIN THREAD %d - WAITING FOR THREAD %d\n\n", control.running_thread->tid, tid);
  #endif

	if((exists(tid)==FALSE) || (isAnotherTidWaiting(tid)==TRUE)) {

    #if DEBUG == TRUE
		printf("THERE'S ALREADY A THREAD WAITING FOR THREAD %d OR THREAD %d DOESN'T EXIST\n\n", tid, tid);
    #endif

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
  AppendFila2(&control.blocked_threads, control.running_thread);

	dispatcher();

	return 0;
}

int cyield(void) {

  /* Check if internal variables was initialized */
	if(control.init == FALSE)
		init();

  #if DEBUG == TRUE
	printf("CYIELD\n\n");
  #endif

	dispatcher();
	return TRUE;
}

int csem_init(csem_t *sem, int count) {

	/* Check if internal variables was initialized */
	if(control.init == FALSE)
		init();

  #if DEBUG == TRUE
  printf("CSEM_INIT\n");
  printf("Initializing semaphore with %d resources to be used.\n\n", count);
  #endif

	/* Set count of the semaphore variable */
	sem->count = count;
	/* Allocate FIFO */
	sem->fila = (PFILA2) malloc(sizeof(PFILA2));
	/* If it allocated ... */
	if (sem->fila == NULL)
		return FALSE;
	/* Create FIFO, if it returned 0, it was successfully created */
	if(CreateFila2(sem->fila) == 0)
		return TRUE;
	return FALSE;
}

int cwait(csem_t *sem){

	/* Check if internal variables was initialized */
	if(control.init == FALSE)
		init();

  #if DEBUG == TRUE
  printf("CWAIT\n");
  printf("There are still %d resources of this semaphore available.\n\n", sem->count);
  #endif

	/* Test count to see if it still has available resources */
	if(sem->count <= 0){
	    /* Put running thread in the semaphore FIFO */
		AppendFila2(sem->fila, (void *)control.running_thread);
		/* Change state to blocked */
    	control.running_thread->state = PROCST_BLOQ;
      control.running_thread->prio = control.running_thread->prio + stopTimer();
    	sem->count--;
      AppendFila2(&control.blocked_threads, control.running_thread);

      #if DEBUG == TRUE
      printf("Thread %d change state to blocked.\n\n", control.running_thread->tid);
      #endif

    	dispatcher();
        return TRUE;
	}
	/* If it has available resources it needs to decrement count and keep running the thread*/
	else {
		sem->count--;

    #if DEBUG == TRUE
    printf("Thread %d keep running.\n\n", control.running_thread->tid);
    #endif

  }
	return TRUE;
}

int csignal(csem_t *sem){

	/* Check if internal variables was initialized */
	if(control.init == FALSE)
		init();

  #if DEBUG == TRUE
  printf("CSIGNAL\n");
  #endif

	/* Goes to the first item in the FIFO*/
	FirstFila2(sem->fila);
		TCB_t *freeTCB;
		/* Get the first item in FIFO that will be freed*/
		freeTCB = GetAtIteratorFila2(sem->fila);
		if(freeTCB != NULL){

      /* Increments count to free resources*/
    	sem->count++;

      #if DEBUG == TRUE
      printf("One resource's been released. Thread %d change state to able\n", freeTCB->tid);
      printf("Now there are still %d resources of this semaphore available.\n\n", sem->count);
      #endif

		    /* Delete item from FIFO, thread is not blocked by the semaphore anymore*/
			DeleteAtIteratorFila2(sem->fila);
			/* Change state to 'able' */
			freeTCB->state = PROCST_APTO;
			/* Put the thread in the ables queue so it can run again*/
			InsertByPrio(&control.able_threads, freeTCB);
      /* Delete thread from blocked threads queue */
      TCB_t *temp;
      FirstFila2(&control.blocked_threads);
      temp = GetAtIteratorFila2(&control.blocked_threads);
      while(temp->tid != freeTCB->tid) {
        NextFila2(&control.blocked_threads);
        temp = GetAtIteratorFila2(&control.blocked_threads);
      }
      DeleteAtIteratorFila2(&control.blocked_threads);
		}
		else{
		   /* There was no resource to be released*/

       #if DEBUG == TRUE
       printf("There was no resource to be released at this semaphore\n\n");
       printf("Now all %d resources of this semaphore are available.\n\n", sem->count+1);
       #endif

		   return FALSE;
		}
	return TRUE;
}
