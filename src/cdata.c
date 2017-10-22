#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>

#include "../include/insert.h"
#include "../include/cthread.h"
#include "../include/support.h"
#include "../include/cdata.h"

void init(void){

	TCB_t* main_thread;
  FILA2 all_threads;
  FILA2 able_threads;
	FILA2 join_threads;

  CreateFila2(&all_threads);
  CreateFila2(&able_threads);
	CreateFila2(&join_threads);

	/* Init queues*/
	control.all_threads = all_threads;
	control.able_threads = able_threads;
	control.join_threads = join_threads;
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

	dispatcher();

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

void dispatcher(){
	TCB_t* next_thread;
	TCB_t* to_delete_thread;
	TCB_t* current_thread = control.running_thread;

	FirstFila2(&control.able_threads);
  next_thread = (TCB_t *) GetAtIteratorFila2(&control.able_threads);
	//printf("Running Thread: %d\n", current_thread->tid);
	//printf("Next Thread: %d\n\n", next_thread->tid);

  if(next_thread == NULL) {
    next_thread = current_thread;
  }

  if (current_thread->state == PROCST_EXEC) {
		/* If the next thread are the running thread do nothing*/
		if (next_thread != current_thread){
			/* Set state of old running thread as APTO and add to able threads*/
			current_thread->state = PROCST_APTO;
      current_thread->prio = current_thread->prio + stopTimer();
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

	else if(current_thread->state == PROCST_BLOQ) {

		if (next_thread->tid != current_thread->tid) {

			FirstFila2(&control.able_threads);
      DeleteAtIteratorFila2(&control.able_threads);
			FirstFila2(&control.able_threads);

			/*fprintf(stderr, "ABLE THREADS\n");

			do {
		    TCB_t* hehe =  GetAtIteratorFila2(&control.able_threads);
		    printf("TID: %d PRIO: %d\n", hehe->tid, hehe->prio);
		  } while(NextFila2(&control.able_threads) == 0);
			printf("\n");*/

			next_thread->state = PROCST_EXEC;
      startTimer();
			control.running_thread = next_thread;

			printf("Running Thread: %d\n", control.running_thread->tid);

			swapcontext(&current_thread->context, &next_thread->context);

		}
	}

	else if (current_thread->state == PROCST_TERMINO) {

		if(FirstFila2(&control.join_threads) == 0) {

			do {
				Dependance* joined_thread = GetAtIteratorFila2(&control.join_threads);
				//fprintf(stderr, "%d\n", joined_thread->dependsOn);
				if(joined_thread->dependsOn == current_thread->tid) {
					joined_thread->dependant->state = PROCST_APTO;
					InsertByPrio(&control.able_threads, joined_thread->dependant);
					DeleteAtIteratorFila2(&control.join_threads);
				}
			} while(NextFila2(&control.join_threads) == 0);
		}

		fprintf(stderr, "ABLE THREADS\n");

	  if(FirstFila2(&control.able_threads) == 0) {

	  	do {
	  		TCB_t* hehe =  GetAtIteratorFila2(&control.able_threads);
	  		printf("TID: %d PRIO: %d\n", hehe->tid, hehe->prio);
	  	} while(NextFila2(&control.able_threads) == 0);
	  	printf("\n");
	  }
	  else {
	    printf("THERE'S NO ABLE THREADS\n\n");
	  }

		FirstFila2(&control.all_threads);

		to_delete_thread = GetAtIteratorFila2(&control.all_threads);
		while(current_thread->tid != to_delete_thread->tid) {
			NextFila2(&control.all_threads);
			to_delete_thread = (TCB_t *) GetAtIteratorFila2(&control.all_threads);
		}
		DeleteAtIteratorFila2(&control.all_threads);

		FirstFila2(&control.able_threads);
	  next_thread = (TCB_t *) GetAtIteratorFila2(&control.able_threads);

		if (next_thread != current_thread) {
			control.running_thread = next_thread;
			control.running_thread->state = PROCST_EXEC;
			FirstFila2(&control.able_threads);
      DeleteAtIteratorFila2(&control.able_threads);
			printf("Running Thread: %d\n", control.running_thread->tid);

			swapcontext(&current_thread->context, &next_thread->context);
		}
	}
}
