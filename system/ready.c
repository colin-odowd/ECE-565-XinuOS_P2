/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready system process list */
qid16 	readylist_user;		/* Index of ready user process list */

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	if (prptr->user_process == TRUE)
	{
		insert(pid, readylist_user, prptr->prprio);
	}
	else 
	{
		insert(pid, readylist, prptr->prprio);
	}
	
	resched();
	return OK;
}
