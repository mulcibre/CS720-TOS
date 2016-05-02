
#include <kernel.h>


PCB pcb[MAX_PROCS];
PCB *next_available_pcb;

PORT create_process (void (*ptr_to_new_proc) (PROCESS, PARAM),
		     int prio,
		     PARAM param,
		     char *name)
{
	MEM_ADDR     new_esp;
    PROCESS      new_proc;

    assert(next_available_pcb);

	//	set new process
	new_proc = next_available_pcb;
	next_available_pcb = new_proc->next;

	//	Initializes the elements of this PCB entry 
	new_proc->magic = MAGIC_PCB;
	new_proc->used = TRUE;
	new_proc->state = STATE_READY;
	new_proc->priority = prio;
	new_proc->first_port = NULL;
	new_proc->name = name;

	new_esp = 640 * 1024 - (new_proc - pcb) * 16 * 1024;

#define PUSH(x)    new_esp -= 4; \
                   poke_l (new_esp, (LONG) x);

    // Initialize the stack for the new process
    PUSH (param);		// First data
    PUSH (new_proc);		// Self
    PUSH (0);			// Dummy return address
    PUSH (ptr_to_new_proc);	// Entry point of new process
    PUSH (0);			// EAX
    PUSH (0);			// ECX
    PUSH (0);			// EDX
    PUSH (0);			// EBX
    PUSH (0);			// EBP
    PUSH (0);			// ESI
    PUSH (0);			// EDI

#undef PUSH

	//	Saves the stack pointer
	new_proc->esp = new_esp;
    
	//	Adds the new process to the ready queue 
	add_ready_queue(new_proc);

	return NULL;
}


PROCESS fork()
{
    // Dummy return to make gcc happy
    return (PROCESS) NULL;
}




void print_process(WINDOW* wnd, PROCESS p)
{
}

void print_all_processes(WINDOW* wnd)
{
}



void init_process()
{
    int i;

    // Initialize PCB array values
    for (i = 1; i < MAX_PROCS; i++) 
    {
		pcb [i].magic = 0;
		pcb [i].used = FALSE;

		if(i < MAX_PROCS - 1)
		{
			pcb [i].next = &pcb[i + 1];
		}
    }
    pcb [MAX_PROCS - 1].next = NULL;
    next_available_pcb = &pcb[1];

	//	define boot process
	pcb[0].magic = MAGIC_PCB;
	pcb[0].used = TRUE;
	pcb[0].state = STATE_READY;
	pcb[0].priority = 1;
	pcb[0].first_port = NULL;
	pcb[0].name = boot_name;
	active_proc = pcb;
}
