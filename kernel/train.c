
#include <kernel.h>

PORT train_port;
WINDOW train_wnd;

#define WAIT_TICKS     25

//**************************
//run the train application
//**************************

void set_train_speed(char* speed)
{
	COM_Message msg;
	char buffer [128];
	msg.input_buffer = buffer;
	
	//	print output to console
	wprintf(&train_wnd, "Setting speed to: ");
	wprintf(&train_wnd, speed);
	wprintf(&train_wnd, "\n");
	msg.output_buffer = "L20Sx\015";
	msg.output_buffer[4] = *speed;
	msg.len_input_buffer = 0;
	wprintf(&train_wnd, msg.output_buffer);
	send(com_port,&msg);
	sleep(WAIT_TICKS);
}

void train_clear_mem_buffer()
{
	wprintf(&train_wnd, "Clearing S88 memory buffer\n");
	send_train_command("R\015", "", 0);
}

void train_set_switch(char* switchNum, char* position)
{
	//	switchNum may be any num char from '1' to '9'
	//	position may be 'G' or 'R' only
	if(switchNum < '1' || switchNum > '9')
	{
		wprintf(&train_wnd, "Invalid switch identifier\n");	
		return;
	}
	if(position != 'G' && position != 'R')
	{
		wprintf(&train_wnd, "Invalid switch position\n");	
		return;
	}
	
	wprintf(&train_wnd, "Setting switch ");
	wprintf(&train_wnd, &switchNum);
	wprintf(&train_wnd, " to ");
	wprintf(&train_wnd, &position);
	wprintf(&train_wnd,"\n");
	
	char switchCommand[] = "M##\015";
	switchCommand[1] = switchNum;
	switchCommand[2] = position;
		
	send_train_command(switchCommand, "", 0);
}

void send_train_command(char* outBuf, char* inBuf, int len_inBuf)
{
	COM_Message msg;
	
	//	configure message payload
	msg.input_buffer = inBuf;	
	msg.output_buffer = outBuf;
	msg.len_input_buffer = len_inBuf;
	
	//	message does not require a reply
	//	send requires a reply
	//	send message to com port, wait a safe amount of time
	send(com_port,&msg);
	//	debug print command message
	//wprintf(&train_wnd, msg.output_buffer);
	sleep(WAIT_TICKS);
}

void train_process(PROCESS self, PARAM param)
{

	COM_Message msg;
	char buffer [128];
	msg.input_buffer = buffer;
	
	//	clear train window
	clear_window(&train_wnd);
	
	//	clear S88 memory buffer first
	train_clear_mem_buffer();
	
	train_set_switch('1', 'G');
	train_set_switch('5', 'G');
	train_set_switch('9', 'R');
	
	set_train_speed("5");
	
	while(1);
}

void init_train(WINDOW* wnd)
{
	train_port = create_process (train_process, 6, 0, "Train process");
	
}
