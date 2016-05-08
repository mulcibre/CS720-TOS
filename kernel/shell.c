#include <kernel.h>
PORT shell_port;
WINDOW shell_wnd = {0, 9, 61, 16, 0, 0, 0xDC};
WINDOW train_wnd = {0, 0, 80, 8, 0, 0, ' '};
WINDOW pacMan_wnd = {60, 0, 61, 16, 0, 0, 0xDC};

int trainRunning = 0;
#define WAIT_TICKS     25
/*
	Command Parser
*/

//	states for parser
enum ParseState
{
	//	reading spaces before a potential command
	space1,		
	space2,
	space3,
	//	currently reading alphanumeric characters in a command (or string, maybe later)
	p1,
	p2,
	p2Str,
	p2StrEnd,
	p3,
	//	machine should terminate parsing
	end
};

void get_params(char* userIn, char* param1, char* param2, char* param3)
{
	//	use this state to control behaviour
	enum ParseState state = space1;	
	
	char* chr = '\0';
	int i = 0;
	int paramCurs = 0;
	
	/*
		Command parser state machine, that will separate the 3 possible parameters
		By it's nature, it will sanitize all illegal characters and elminate extra spaces
	*/
	while(state != end && userIn[i] != 0)
	{
		chr = userIn[i];
		
		if((chr >= '0' && chr <= '9') || (chr >= 'a' && chr <= 'z') || (chr >= 'A' && chr <= 'Z'))
		{
			//	add alphanumeric character to appropriate parameter buffer
			switch(state)
			{
				case space1:
					state = p1;
					paramCurs = 0;
				case p1:
					param1[paramCurs] = chr;
					break;
				case space2:
					state = p2;
					paramCurs = 0;
				case p2:
				//case p2Str:
					param2[paramCurs] = chr;
					break;
				case space3:
					state = p3;
					paramCurs = 0;
				case p3:
					param3[paramCurs] = chr;
					break;
				default:
					break;
			}
			paramCurs++;
		}
		else if(chr == ' ')
		{
			switch(state)
			{
				case space1:
				case space2:
				case space3:
					break;
				case p1:
					param1[paramCurs] = 0;
					state = space2;
					break;
				case p2:
					param2[paramCurs] = 0;
				case p2StrEnd:					
					state = space3;
					break;
				/*case p2Str:
					param2[paramCurs] = chr;
					paramCurs++;
					break;*/
				case p3:
					param3[paramCurs] = 0;
					state = end;
					break;	
				default:
					break;
			}
		}
		/*
		code for reading strings that doesn't work, no idea why
		when text is surrounded by quotation marks it doesn't get put into the param array for whatever reason
		else if(chr == '"')
		{
			switch(state)
			{
				case p2Str:
					state = p2StrEnd;
					wprintf(&shell_wnd, "\nstate p2StrEnd\n");
					break;
				case space2:
					state = p2Str;
					wprintf(&shell_wnd, "\nstate p2Str\n");
					break;
				default:
					break;
			}
		}*/
	i++;
	}
	/*
	//	debug stuff
	wprintf(&shell_wnd, "\nparameter 1: ");
	wprintf(&shell_wnd, param1);
	wprintf(&shell_wnd, "\nparameter 2: ");
	wprintf(&shell_wnd, param2);
	wprintf(&shell_wnd, "\nparameter 3: ");
	wprintf(&shell_wnd, param3);
	wprintf(&shell_wnd, "\n");
	*/
}

/*
	Train Controls (temporary)
*/

void set_train_speed_shell(char* speed)
{
	COM_Message msg;
	char buffer [128];
	msg.input_buffer = buffer;
	
	//	print output to console
	wprintf(&train_wnd, "Setting speed to: ");
	wprintf(&train_wnd, speed);
	wprintf(&train_wnd, "\n");
	
	//	configure message payload
	msg.output_buffer = "L20Sx\015";
	msg.output_buffer[4] = *speed;
	
	//	msg input buffer length MUST be 0 for payloads that do not have a response
	msg.len_input_buffer = 0;
	wprintf(&train_wnd, msg.output_buffer);
	send(com_port,&msg);
	
	sleep(WAIT_TICKS);
}

void train_clear_mem_buffer_shell()
{
	wprintf(&train_wnd, "Clearing S88 memory buffer\n");
	send_train_command("R\015", "", 0);
}

void train_switch_directions_shell(int speed)
{
	if(speed == 0)
	{
		wprintf(&train_wnd, "Changing train direction\n");
		send_train_command("L20D\015", "", 0);
	}
	else
	{
		wprintf(&train_wnd, "Train must be stopped during direction changes. Dare you risk Dr. Puders wrath?\n");		
	}
}

void train_set_switch_shell(char* switchNum, char* position)
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

int get_status_of_contact_shell(char* trackSection)
{
	//	S88 memory MUST be cleared before checking the status of a track
	train_clear_mem_buffer_shell();
	
	//	Track segment query can always be double digits e.g. 05
	char trackQueryCommand[] = "C00\015";
	
	wprintf(&train_wnd, "param2: ");
	wprintf(&train_wnd, trackSection);
	wprintf(&train_wnd, "\n");
	
	//	single digit input, between 1 and 9
	if((trackSection[0] >= '1' && trackSection[0] <= '9') && trackSection[1] == 0)
	{
		//	single digit condition succeeded
		trackQueryCommand[2] = trackSection[0];
	}
	else if(trackSection[0] == '1' && (trackSection[1] >= '0' && trackSection[1] <= '6'))
	{
		//	double digit condition succeeded
		trackQueryCommand[1] = trackSection[0];
		trackQueryCommand[2] = trackSection[1];
	}
	else
	{
		//	no success conditions	
		wprintf(&train_wnd, "Invalid track id entered, must be from 1-16\n");
		return -1;		
	}
	wprintf(&train_wnd, "command: ");
	wprintf(&train_wnd, trackQueryCommand);
	wprintf(&train_wnd, "\n");
	return send_train_command_shell(trackQueryCommand, "0\0", 3);
	
}

int send_train_command_shell(char* outBuf, char* inBuf, int len_inBuf)
{
	COM_Message msg;
	int retVal = 0;
	//	configure message payload
	msg.input_buffer = inBuf;	
	msg.output_buffer = outBuf;
	msg.len_input_buffer = len_inBuf;
	
	//	message does not require a reply
	//	send requires a reply
	//	send message to com port, wait a safe amount of time
	send(com_port,&msg);
	
	if(len_inBuf)
	{
		wprintf(&train_wnd, "Returned value from request: ");
		wprintf(&train_wnd, &msg.input_buffer[1]);
		wprintf(&train_wnd, "\n");
	}
	
	//	return value in input buffer, used for probing track sections
	if(msg.input_buffer[1] == '1')
	{
		retVal = 1;
	}
	
	//	debug print command message
	//wprintf(&train_wnd, msg.output_buffer);
	sleep(WAIT_TICKS);
	return retVal;
}


/*
	Shell Process with Command Loop
*/

void shell_process(PROCESS self, PARAM param)
{
	clear_window(&shell_wnd);
	//print_all_processes(&shell_wnd);
	
	//testStringCompare();
	
	char inBuf[128] = "";
	char param1[128] = "\0";
	char param2[128] = "\0";
	char param3[128] = "\0";
	int bufSize = sizeof(inBuf)/sizeof(char);
	int bufCurs = 0;
	int i;
	
	char ch;
 	Keyb_Message msg; 	
	
	while (1) {
		//	reset vars after each command is processed
		ch = 0;
		bufCurs = 0;
		for(i = 0; i < 128; i++)
		{
			inBuf[i] = 0;
			param1[i] = 0;
			param2[i] = 0;
			param3[i] = 0;
		}
		//	output command prompt
		wprintf(&shell_wnd, "SamOS >>> ");
		
	 	//read command from keyboard, terminate upon reading newline
		while(ch != 13)
		{
			//	check for buffer overflow
			if(bufCurs > 126)
			{
				wprintf(&shell_wnd, "\nTrying to overflow the buffer? Try again, Bub\n");
				break;
			}
			
			msg.key_buffer = &ch;
 			send(keyb_port, &msg);
		 	
			//	cover case of backspace
			if(ch == '\b')
			{
				//	do nothing if buffer is empty
				if(bufCurs == 0)
				{
					continue;
				}
				else
				{
				inBuf[bufCurs] = 0;
				bufCurs--;
				}
			}
			else if(ch != 13)
			{
				//	commit entered character to buffer, increment cursor
				inBuf[bufCurs] = ch;
				bufCurs++;
			}
			//	output entered character to console
			output_char(&shell_wnd, ch);
		}
		//	input buffer debug
		//wprintf(&shell_wnd,inBuf);
		get_params(inBuf,param1,param2,param3);
		
		executeCommand(param1,param2,param3);
	}
} 

/*
	Command Tree
*/

int executeCommand(char* command, char* param1, char* param2)
{
		//	Some basic commands
		if(stringCompare(command, "ps") == 0)
		{
			print_all_processes(&shell_wnd);
			return 0;
		}
		else if(stringCompare(command, "clear") == 0)
		{
			clear_window(&shell_wnd);
			return 0;
		}
		else if(stringCompare(command, "help") == 0)
		{
			showHelp();
			return 0;
		}
		//	Train related commands
		else if(stringCompare(command, "train") == 0)
		{
			if(stringCompare(param1, "clear") == 0 && stringCompare(param2, "") == 0)
			{
				train_clear_mem_buffer_shell();
			}
			//	train speed setting
			else if(stringCompare(param1, "speed") == 0)
			{
				if(param2[1] == 0 && param2[0] >= '0' && param2[0] <= '5')
				{
					set_train_speed_shell(&param2[0]);
				}
				else
				{
					wprintf(&shell_wnd, "Invalid Speed Selection\n");
					return -1;
				}
			}
			else if(stringCompare(param1, "switch") == 0)
			{
				if(param2[2] != 0)
				{
					wprintf(&shell_wnd, "Invalid switch command format\n");
					wprintf(&shell_wnd, "Must be of form train switch #C\n");
					wprintf(&shell_wnd, "Where # is [1-9] and C is 'R' or 'G'\n");	
					return -1;
				}
				if(param2[0] < '1' || param2[0] > '9')
				{
					wprintf(&shell_wnd, "Invalid switch identifier\n");	
					return -1;
				}
				if(param2[1] != 'G' && param2[1] != 'R')
				{
					wprintf(&shell_wnd, "Invalid switch position\n");	
					return -1;
				}
				train_set_switch_shell(param2[0], param2[1]);
				return 0;
			}
			else if(stringCompare(param1, "see") == 0)
			{
				//	Only allow second parameter length of 2
				if(strlen(param2) > 2 || param2[0] == 0)
				   {
						wprintf(&shell_wnd, "Invalid parameter 2 length\n");	
						return -1;
				   }
				   else
				   {
					   	if(!get_status_of_contact_shell(param2))
						{
							wprintf(&shell_wnd, "train not detected\n");
						}
					   	else
					  	{
					   		wprintf(&shell_wnd, "train found on track ");
					   		wprintf(&shell_wnd, param2);
					   		wprintf(&shell_wnd, "\n");
					   	}
				   }
			}
			else if(stringCompare(param1, "run") == 0 && stringCompare(param2, "") == 0)
			{
				wprintf(&shell_wnd, "Train Executing\n");
				trainRunning = 0;
				init_train(&train_wnd);
				return 0;
			}
			else
			{
				wprintf(&shell_wnd, "See 'help' for list of train commands\n");	
				return -1;
			}
		}
		else if(stringCompare(command, "pacman") == 0)
		{
			wprintf(&shell_wnd, "Pacman Executing\n");
			init_pacman(&pacMan_wnd,4);
			return 0;
		}
		else
		{
			wprintf(&shell_wnd, "Invalid Command. Enter 'help' for list of commands\n");
			return -1;
		}
}

int stringCompare(char* str1, char* str2)
{
	//	Note: case sensitive
	int cursor = 0;
	while(str1[cursor] != 0 || str2[cursor] != 0)
	{
		//	debug output
		//output_char(&shell_wnd, str1[cursor]);
		if(str1[cursor] != str2[cursor])
		{
			return -1;	
		}
		cursor++;
	}
	//	if both strings end simultaneously and all characters matched, return 0
	return 0;
}

void testStringCompare()
{
	if(stringCompare("cat","cat") == 0)
	{
		wprintf(&shell_wnd, "strings same pass\n");
	}
	else
	{
		wprintf(&shell_wnd, "strings same fail\n");
	}
	if(stringCompare("cate","cat") == -1)
	{
		wprintf(&shell_wnd, "strings diff pass\n");
	}
	else
	{
		wprintf(&shell_wnd, "strings diff fail\n");
	}	
}

void showHelp()
{
	wprintf(&shell_wnd, "\nPossible Commands:\n");
	wprintf(&shell_wnd, "help             ----    shows this message\n");
	wprintf(&shell_wnd, "ps               ----    displays all active processes\n");
	wprintf(&shell_wnd, "clear            ----    clears console window\n");
	wprintf(&shell_wnd, "pacman           ----    initializes pacman process\n");
	wprintf(&shell_wnd, "train run        ----    run train script\n");
	wprintf(&shell_wnd, "train speed #    ----    set train to speed, # must be 0-5\n");
	wprintf(&shell_wnd, "train clear      ----    clear train buffer\n");
	wprintf(&shell_wnd, "train switch #C  ----    set switch to position\n");
	wprintf(&shell_wnd, "                         # must be 1-9, C must be G or R\n");
	wprintf(&shell_wnd, "train see ##     ----    check track state, ## must be 0-16\n\n");
}

void init_shell()
{
	shell_port = create_process (shell_process, 6, 0, "Shell process");
    //resign();
}