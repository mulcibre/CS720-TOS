#include <kernel.h>
//#include <trainCommands.h>

PORT shell_port;
WINDOW shell_wnd = {0, 9, 61, 16, 0, 0, 0xDC};
WINDOW train_wnd = {0, 0, 80, 8, 0, 0, ' '};
WINDOW pacMan_wnd = {60, 0, 61, 16, 0, 0, 0xDC};

int trainRunning = 0;
#define WAIT_TICKS     25

/*
	Command Table
*/

struct table_entry {
	char *cmdname;
	int (*cmd)();
};

int nothingcommand() { return 0; }

int showProcessesCommand(char* param1, char* param2)
{
	print_all_processes(&shell_wnd);
	return 1;
}

int clearWindowCommand(char* param1, char* param2)
{
	clear_window(&shell_wnd);
	return 1;
}

int showHelpCommand(char* param1, char* param2)
{
	showHelp(&shell_wnd);
	return 1;
}

int trainCommand(char* param1, char* param2)
{
	if(stringCompare(param1, "clear") == 0 && stringCompare(param2, "") == 0)
			{
				train_clear_mem_buffer();
				return 0;
			}
			//	train speed setting
			else if(stringCompare(param1, "speed") == 0)
			{
				if(param2[1] == 0 && param2[0] >= '0' && param2[0] <= '5')
				{
					set_train_speed(&param2[0]);
					return 0;
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
				train_set_switch(param2[0], param2[1]);
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
					   	if(get_status_of_contact(param2) == 0)
						{
							wprintf(&shell_wnd, "train not detected\n");
						}
					   	else
					  	{
					   		wprintf(&shell_wnd, "train found on track ");
					   		wprintf(&shell_wnd, param2);
					   		wprintf(&shell_wnd, "\n");
					   	}
					   return 0;
				   }
			}
			else if(stringCompare(param1, "rev") == 0)
			{
				wprintf(&shell_wnd, "Reversing train direction\n");
				train_switch_directions();
				//switch_train_direction(&train_wnd);
				return 0;
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
	return 1;
}

int pacmanCommand(char* param1, char* param2)
{
	wprintf(&shell_wnd, "Pacman Executing\n");
	init_pacman(&pacMan_wnd,4);
	return 1;
}

struct table_entry cmds[] = 
{
  	{"ps", showProcessesCommand},
  	{"clear", clearWindowCommand},
  	{"help", showHelpCommand},
  	{"train", trainCommand},
  	{"pacman", pacmanCommand},
  	{"", 0}
};

int (*getfn(const char *word))() {
  struct table_entry* p;
  for (p = cmds; p->cmd; ++p) {
    if (!stringCompare(word, p->cmdname)) {
      return p->cmd;
    }
  }
  wprintf(&shell_wnd, "Command not found\n");
  return nothingcommand;
}

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
	
 	//	Command
 	int (*func)(param1, param2);

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

int executeCommand(char* command, char* param1, char* param2)
{
	int (*func)(param1, param2) = getfn(command);
  	(*func)(param1, param2);
	return 0;
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
	wprintf(&shell_wnd, "train rev        ----    reverse train direction, speed must be 0\n");
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