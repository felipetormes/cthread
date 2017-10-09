#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/cthread.h"
#include "../include/support.h"
#include "../include/cdata.h"

void init(void){

	TCB_t* main_thread;
  FILA2 all_threads;
  FILA2 able_threads;

  CreateFila2(&all_threads);
  CreateFila2(&able_threads);

	/* Init queues*/
	control.all_threads = all_threads;
	control.able_threads = able_threads;
	control.init = TRUE;

	/* Create main thread with TID = 0*/
	main_thread = (TCB_t*)malloc(sizeof(TCB_t));
	main_thread->state = PROCST_EXEC;
  startTimer();
	main_thread->tid = 0;
  main_thread->prio = 0;

	/* Insert main thread in all_threads*/
	AppendFila2(&control.all_threads, main_thread);

	/* Create end functions to threads*/
	getcontext(&control.ended_thread);
	control.ended_thread.uc_link = NULL;
	control.ended_thread.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	control.ended_thread.uc_stack.ss_size = SIGSTKSZ;
	makecontext(&control.ended_thread, (void (*)(void))ended_thread, 0);

	/* Set Main thread as running*/
	control.running_thread = main_thread;

	/* Create context to main thread*/
	getcontext(&main_thread->context);
	main_thread->context.uc_link = &control.ended_thread;
	main_thread->context.uc_stack.ss_sp = (char*) malloc(SIGSTKSZ);
	main_thread->context.uc_stack.ss_size = SIGSTKSZ;
}

void ended_thread(void){
	/* Save context to next callback*/
	getcontext(&control.ended_thread);

	control.running_thread->state = PROCST_TERMINO;

	//dispatcher();

}

void dispatcher(){
	TCB_t* next_thread;
	TCB_t* current_thread = control.running_thread;

	FirstFila2(&control.able_threads);
  next_thread = (TCB_t *) GetAtIteratorFila2(&control.able_threads);
	//printf("Next Thread: %d\n", next_thread->tid);

  if(next_thread == NULL) {
    next_thread = current_thread;
  }

  if (current_thread->state == PROCST_EXEC){
		/* If the next thread are the running thread do nothing*/
		if (next_thread != current_thread){
			/* Set state of old running thread as APTO and add to able threads*/
			current_thread->state = PROCST_APTO;
      current_thread->prio = stopTimer();
			FirstFila2(&control.able_threads);
      DeleteAtIteratorFila2(&control.able_threads);
      InsertByPrio(&control.able_threads, current_thread);

      next_thread->state = PROCST_EXEC;
      startTimer();
  		control.running_thread = next_thread;
			printf("Running Thread: %d\n", control.running_thread->tid);
			/* Swapping context to new thread*/
			swapcontext(&current_thread->context, &next_thread->context);
		}
	}
}