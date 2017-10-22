/*
 * cdata.h: arquivo de inclus�o de uso apenas na gera��o da libpithread
 *
 * Esse arquivo pode ser modificado. ENTRETANTO, deve ser utilizada a TCB fornecida
 *
 * Vers�o de 11/09/2017
 *
 */
#ifndef __cdata__
#define __cdata__
#include "ucontext.h"

#define	PROCST_CRIACAO	0
#define	PROCST_APTO	1
#define	PROCST_EXEC	2
#define	PROCST_BLOQ	3
#define	PROCST_TERMINO	4

#define TRUE 0
#define FALSE -1

#include "support.h"

/* Os campos "tid", "state", "prio" e "context" dessa estrutura devem ser mantidos e usados convenientemente
   Pode-se acrescentar outros campos AP�S os campos obrigat�rios dessa estrutura
*/
typedef struct s_TCB {
	int		tid; 		// identificador da thread
	int		state;		// estado em que a thread se encontra
					// 0: Cria��o; 1: Apto; 2: Execu��o; 3: Bloqueado e 4: T�rmino
	unsigned 	int		prio;		// prioridade da thread (higest=0; lowest=3)
	ucontext_t 	context;	// contexto de execu��o da thread (SP, PC, GPRs e recursos)

	/* Se necess�rio, pode-se acresecentar campos nessa estrutura A PARTIR DAQUI! */


} TCB_t;

struct _control_threads{
    /* Verify if the main thread was initialized */
    int init;
    /* Queue of all threads */
    FILA2 all_threads;
    /* Queue of able threads */
    FILA2 able_threads;
		/* Queue of joined threads */
		FILA2 join_threads;
    /* Current thread pointer running */
    TCB_t* running_thread;
	/* UCP to delete a thread that end */
	ucontext_t ended_thread;
};

extern struct _control_threads control;

struct sJoinDependance {
	TCB_t *dependant;
	int dependsOn;
};

typedef struct sJoinDependance Dependance;

void init(void);
void ended_thread(void);
int exists(int tid);
int isAnotherTidWaiting(int tid);
void dispatcher();

#endif
