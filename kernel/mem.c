
#include <kernel.h>


void poke_b (MEM_ADDR addr, BYTE value)
{
	BYTE* ptr = (BYTE*) addr;
	*ptr = value;
}

void poke_w (MEM_ADDR addr, WORD value)
{	
	WORD* ptr = (WORD*) addr;
	*ptr = value;
}

void poke_l (MEM_ADDR addr, LONG value)
{
	LONG* ptr = (LONG*) addr;
	*ptr = value;
}

BYTE peek_b (MEM_ADDR addr)
{
	BYTE* ptr = (BYTE*) addr;
	return *ptr;
}

WORD peek_w (MEM_ADDR addr)
{
	WORD* ptr = (WORD*) addr;
	return *ptr;
}

LONG peek_l (MEM_ADDR addr)
{
	LONG* ptr = (LONG*) addr;
	return *ptr;
}


