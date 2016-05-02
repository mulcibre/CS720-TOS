#include <kernel.h>
PORT shell_port;
WINDOW shell_wnd = {0, 9, 61, 16, 0, 0, 0xDC};

void shell_process(PROCESS self, PARAM param)
{
	clear_window(&shell_wnd);
	//print_all_processes(&shell_wnd);
	
	char inBuf[128] = "";
	int bufSize = sizeof(inBuf)/sizeof(char);
	int bufCurs = 0;
	
	char ch;
 	Keyb_Message msg; 
	
	while (1) {
		//	reset vars after each command is entered
		ch = 0;
		
		//	output command prompt
		wprintf(&shell_wnd, "SamOS >>> ");
	 	//read command from keyboard, terminate upon reading newline
		while(ch != 13)
		{
			msg.key_buffer = &ch;
 			send(keyb_port, &msg);
		 	output_char(&shell_wnd, ch);
			
		}
	}
} 


void init_shell()
{
	shell_port = create_process (shell_process, 6, 0, "Shell process");
    //resign();
}