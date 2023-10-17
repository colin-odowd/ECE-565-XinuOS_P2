/* resched.c - resched, resched_cntl */

#include <xinu.h>
#include <stdlib.h>

//#define DEBUG_CTXSW 

struct	defer	Defer;

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
	uint32 totaltickets = 0;
	uint32 lottery_processes = 0;

	/* If rescheduling is deferred, record attempt and return */
	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];
	old_pid = currpid;

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		
		ptold->prstate = PR_READY;
		if (ptold->user_process == TRUE)
		{
			insert_user(currpid, readylist_user, ptold->tickets);
		}
		else 
		{
			insert(currpid, readylist, ptold->prprio);
		}
		
	}

	/* Case 1: Process in System Queue (exlcuding NULL process) */
	if (firstkey(readylist) != (pid32)(0))
	{
		currpid = dequeue(readylist);
	}
	/* Case 2: Process in User Queue, perform lottery */
	else if ((isempty(readylist_user) == 0))
	{ 
		curr = firstid(readylist_user);
		while (curr != queuetail(readylist_user))
		{
			totaltickets += queuetab[curr].qkey;
			lottery_processes++;
			curr = queuetab[curr].qnext;
		}
		if (lottery_processes > 1)
		{

			winner = rand() % totaltickets;
			curr = firstid(readylist_user);
			while (curr != queuetail(readylist_user))
			{
				counter = counter + queuetab[curr].qkey;
				if (counter > winner) break;
				curr = queuetab[curr].qnext;
			}
			currpid = curr;
			queuetab[queuetab[currpid].qprev].qnext = queuetab[currpid].qnext;
			queuetab[queuetab[currpid].qnext].qprev = queuetab[currpid].qprev;
			queuetab[currpid].qnext = EMPTY;
			queuetab[currpid].qprev = EMPTY;	
		}
		else
		{
			currpid = dequeue(readylist_user);
		}
	}
	/* Case 3: No processes in either queue, execute NULL process  */
	else 
	{
		currpid = dequeue(readylist);
	}

	ptnew = &proctab[currpid];
	if (currpid != old_pid) ptnew->num_ctxsw++;
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		

	#ifdef DEBUG_CTXSW
		if (currpid != old_pid) kprintf("ctxsw::%d-%d\n", old_pid, currpid);
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
