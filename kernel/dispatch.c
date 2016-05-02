
#include <kernel.h>

#include "disptable.c"


PROCESS active_proc;


/*
 * Ready queues for all eight priorities.
 */
PCB *ready_queue [MAX_READY_QUEUES];




/*
 * add_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is put the ready queue.
 * The appropiate ready queue is determined by p->priority.
 */

void add_ready_queue (PROCESS proc)
{
	//	Changes the state of process p to ready
	proc->state = STATE_READY;
	assert(proc->priority > 0 && proc->priority < 8);

	//	Process p is added to the ready queue
	//	at the tail of the double linked list for the appropriate priority level.
	//	p->priority determines to which priority level the process is added in the ready queue 
	if(ready_queue[proc->priority] == NULL)
	{
		ready_queue[proc->priority] = proc;
		proc->next = proc->prev = proc;
	}
	else
	{
		//	already at least one item in the priority queue
		//  set old tail's next pointer to point to proc
		proc->prev = ready_queue[proc->priority]->prev;
		proc->next = ready_queue[proc->priority];
		
		ready_queue[proc->priority]->prev->next = proc;
		ready_queue[proc->priority]->prev = proc;
	}
}



/*
 * remove_ready_queue
 *----------------------------------------------------------------------------
 * The process pointed to by p is dequeued from the ready
 * queue.
 */

void remove_ready_queue (PROCESS proc)
{
	if (proc->next == proc) 
	{
		//	there was only one process on this priority level
		ready_queue [proc->priority] = NULL;
	} 
	else 
	{
		//	make appropriate pointer corrections
		ready_queue [proc->priority] = proc->next;
		proc->next->prev = proc->prev;
		proc->prev->next = proc->next;
    }
}



/*
 * dispatcher
 *----------------------------------------------------------------------------
 * Determines a new process to be dispatched. The process
 * with the highest priority is taken. Within one priority
 * level round robin is used.
 */

PROCESS dispatcher()
{
}



/*
 * resign
 *----------------------------------------------------------------------------
 * The current process gives up the CPU voluntarily. The
 * next running process is determined via dispatcher().
 * The stack of the calling process is setup such that it
 * looks like an interrupt.
 */
void resign()
{
}



/*
 * init_dispatcher
 *----------------------------------------------------------------------------
 * Initializes the necessary data structures.
 */

void init_dispatcher()
{
	//	zero out the ready queue
	int i;
    for (i = 0; i < MAX_READY_QUEUES; i++)
    {
		ready_queue [i] = NULL;
	}
	add_ready_queue (active_proc);
}
