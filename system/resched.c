/* resched.c - resched, resched_cntl */

#include <xinu.h>
#include <stdlib.h>

#define DEBUG_CTXSW

struct	defer	Defer;

uint32 	totaltickets;	/* Total ticket count for all user processes */

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	pid32  old_pid;
	uint32 counter = 0;
	uint32 winner = 0;
	qid16  curr;

	/* If rescheduling is deferred, record attempt and return */
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];
	old_pid = currpid;

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		
		ptold->runtime++;

		if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		/* Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	if (isempty(readylist)) 
	{
		// winner: use some call to a random number generator to
		// get a value, between 0 and the total # of tickets
		winner = rand() % totaltickets;
		curr = firstid(readylist_user);
		while (curr != EMPTY)
		{
			counter = counter + queuetab[curr].qkey;
			if (counter > winner) break;
			curr = queuetab[curr].qnext;
		}
	}

	/* Force context switch to highest priority ready process */

	currpid = dequeue(readylist);
	ptnew = &proctab[currpid];
	ptnew->num_ctxsw++;
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/

	#ifdef DEBUG_CTXSW
		kprintf("ctxsw::%d-%d\n", old_pid, currpid);
	#endif

	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
