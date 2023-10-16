#include <xinu.h>
#include <stdio.h>

void timed_execution(uint32 runtime){
	    while(proctab[currpid].runtime<runtime);
}

void sync_printf(char *fmt, ...)
{
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
}

int main() {
	pid32 prA, prB, prC;

	sync_printf("\n");

	sync_printf("=== TESTCASE 1::  1 process - context switches =================\n");	
	prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 100);
	set_tickets(prA, 50);
	resume(prA);
	receive();
	sleepms(50); //wait for user process to terminate	
	kprintf("\nprocess %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
	
	sync_printf("================================================================\n\n");	
	sync_printf("=== TESTCASE 2::  2 processes - context switches ===============\n");	
	prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 100);
	prB = create_user_process(timed_execution, 1024, "timed_execution", 1, 100);
	set_tickets(prA, 50);
	set_tickets(prB, 50);
	resume(prA);
	resume(prB);
	receive();
	receive();
	sleepms(50); //wait for user processes to terminate	
	kprintf("\nprocess %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
	
	sync_printf("================================================================\n\n");	

	sync_printf("=== TESTCASE 3::  3 processes - context switches ===============\n");	
	prA = create_user_process(timed_execution, 1024, "timed_execution", 1, 100);
	prB = create_user_process(timed_execution, 1024, "timed_execution", 1, 100);
	prC = create_user_process(timed_execution, 1024, "timed_execution", 1, 40);
	set_tickets(prA, 50);
	set_tickets(prB, 50);
	set_tickets(prC, 50);
	resume(prA);
	resume(prB);
	sleepms(50);	
	resume(prC);
	receive();
	receive();
	receive();
	sleepms(50); //wait for user processes to terminate	
	kprintf("\nprocess %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
	sync_printf("================================================================\n\n");	
	return OK;
}
