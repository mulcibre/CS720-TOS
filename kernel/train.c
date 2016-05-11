
#include <kernel.h>

PORT train_port;
WINDOW train_wnd;

#define WAIT_TICKS     25
#define WAIT_FOR_ZAMBONI	15
#define CHECK_FOR_CC	3

char* trainSpeed = '0';

//**************************
//run the train application
//**************************

/*
		Train Control API
*/

void set_train_speed(char* speed)
{	
	//	print output to console
	//wprintf(&train_wnd, "Setting speed to: %s\n",speed);
	
	char speedCommand[] = "L20Sx\015";
	speedCommand[4] = *speed;
	send_train_command(speedCommand,"",0);
	
	//	update global train speed value
	trainSpeed = *speed;
}

void train_clear_mem_buffer()
{
	//	print this for debug, otherwise this is unnecessarily verbose
	//wprintf(&train_wnd, "Clearing S88 memory buffer\n");
	send_train_command("R\015", "", 0);
}

void train_switch_directions()
{
	if(trainSpeed == '0')
	{
		//wprintf(&train_wnd, "Changing train direction\n");
		send_train_command("L20D\015", "", 0);
	}
	else
	{
		wprintf(&train_wnd, "Train must be stopped during direction changes. Dare you risk Dr. Puders wrath?\n");		
	}
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
	
	//wprintf(&train_wnd, "Setting switch %s to %s\n", &switchNum, &position);
	
	char switchCommand[] = "M##\015";
	switchCommand[1] = switchNum;
	switchCommand[2] = position;
		
	send_train_command(switchCommand, "", 0);
}

int get_status_of_contact(char* trackSection)
{
	//	S88 memory MUST be cleared before checking the status of a track
	train_clear_mem_buffer();
	
	//	Track segment query can always be double digits e.g. 05
	char trackQueryCommand[] = "C00\015";
	
	//wprintf(&train_wnd, "param2: %s\n", trackSection);
	
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
	//wprintf(&train_wnd, "Probing track segment: %s", trackQueryCommand);
	
	return send_train_command(trackQueryCommand, "0\0", 3);
	
}

int send_train_command(char* outBuf, char* inBuf, int len_inBuf)
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
		//wprintf(&train_wnd, "Returned value from request: ");
		//wprintf(&train_wnd, &msg.input_buffer[1]);
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
		Train script
*/

//	initialize the track, set switch positions
void init_track()
{
	//	Set switches so that Zamboni will not crash
	train_set_switch('5', 'G');
	train_set_switch('8', 'G');
	train_set_switch('9', 'R');
	train_set_switch('4', 'G');
	train_set_switch('1', 'G');
	
}

/*	Returns:
	0 - no Zamboni
	1 - Zamboni circulating clockwise
	2 - Zamboni circulating Anti clockwise
*/
int get_zamboni_condition()
{
	int retVal = 0;
	int i = 0;
	
	wprintf(&train_wnd, "Searching for Zamboni");
	//	Watch for Zamboni. Counter should be large enough that Zomboni could make a complete circuit
	for(i = 0; i < 20; i++)
	{
		//	watch for zamboni on segment 10
		if(get_status_of_contact("10"))
		{
			retVal = 1;
			wprintf(&train_wnd, "\nFound Zamboni\n");
			break;
		}
		wprintf(&train_wnd, ".");
	}
	for(i = 0; i < 3; i++)
	{
		//	Now watch segment 7 for a short time. If Zamboni is spotted, path is Anti Clockwise
		if(get_status_of_contact("7"))
		{
			retVal = 2;
			break;
		}
	}
	
	if(retVal == 0)
	{
		wprintf(&train_wnd, "Zamboni not detected\n");
	}
	else if(retVal == 1)
	{
		wprintf(&train_wnd, "Zamboni traveling Clockwise\n");
		
		//	Switch zamboni direction
		set_zamboni_to_anti_clockwise();
	}
	else if(retVal == 2)
	{
		wprintf(&train_wnd, "Zamboni traveling AntiClockwise\n");
	}
	
	return retVal;	
}

void set_zamboni_to_anti_clockwise()
{
	//	Set switches so that Zamboni will reverse direction
	train_set_switch('1', 'R');
	train_set_switch('2', 'R');
	train_set_switch('7', 'G');
	
	//	detect Zamboni going through bypass, and returning to outer loop
	while(!get_status_of_contact("12"));
	while(!get_status_of_contact("7"));
	
	//	return outer loop switch to original setting
	train_set_switch('1', 'G');
}

/*	Returns:
	0 - config 1 and 2
	1 - config 3
	2 - config 4	
*/
int get_train_wagon_config()
{
	if(get_status_of_contact("8") && get_status_of_contact("2"))
	{
		wprintf(&train_wnd, "Train config 1,2 detected\n");
		return 0;	
	}
	else if(get_status_of_contact("5"))
	{
		if(get_status_of_contact("11"))
		{
			wprintf(&train_wnd, "Train config 3 detected\n");
			return 1;
		}
		else if(get_status_of_contact("16"))
		{
			wprintf(&train_wnd, "Train config 4 detected\n");
			return 2;
		}
	}
	else
	{
		wprintf(&train_wnd, "Train detection not successful\n");
		return -1;	
	}
}

void runConfig1or2(int zamboniConfig)
{
	//	Set switches for wagon pickup
	train_set_switch('3', 'G');
	train_set_switch('2', 'G');

	wprintf(&train_wnd, "Waiting for Zamboni to pass wagon\n");
	//	wait for zamboni to go by wagon
	if(zamboniConfig)
	{
		while(!get_status_of_contact("3"));
	}

	set_train_speed("5");
	
	//	set switch so zamboni will be trapped in loop
	train_set_switch('8', 'R');

	//	set switch so train can get wagon
	train_set_switch('4', 'R');
	wprintf(&train_wnd, "Trapping Zamboni in small loop\n");
	
	//	slow down train once it's close to the wagon
	while(!get_status_of_contact("6"));
	while(get_status_of_contact("6"));
	//	Won't need this on actual train set
	//set_train_speed("4");

	//	detect pickup of wagon
	while(!get_status_of_contact("1"));
	set_train_speed("0");
	train_set_switch('4', 'G');

	wprintf(&train_wnd, "Ensure Zamboni is trapped\n");
	//	make sure zamboni is in loop
	if(zamboniConfig)
	{
		while(!get_status_of_contact("12"));
	}

	wprintf(&train_wnd, "Heading home\n");
	//	send the train home
	train_switch_directions();
	set_train_speed("5");

	train_set_switch('5', 'R');
	train_set_switch('6', 'R');

	//	wait for arrival and stop
	while(!get_status_of_contact("7"));
	while(get_status_of_contact("7"));
	set_train_speed("0");
	wprintf(&train_wnd, "Home Safe!\n");

	//	restore outer track config
	train_set_switch('5', 'G');
}

void runConfig3(int zamboniConfig)
{
	//	init switches
	train_set_switch('7', 'G');
	train_set_switch('3', 'R');
	train_set_switch('2', 'R');
	
	//	Waiting for Zamboni
	if(zamboniConfig)
	{
		wprintf(&train_wnd, "Waiting for Zamboni\n");
		while(!get_status_of_contact("4"));
	}
	
	//	put train on outer loop, head for wagon
	wprintf(&train_wnd, "Turning around\n");
	set_train_speed("5");
	while(!get_status_of_contact("6"));
	set_train_speed("0");
	train_switch_directions();
	set_train_speed("5");
	wprintf(&train_wnd, "Getting wagon\n");
	
	//	Let Zamboni pass track segment 13 before setting switch
	if(zamboniConfig)
	{
		while(!get_status_of_contact("10"));
	}
	train_set_switch('8', 'R');
	while(get_status_of_contact("11"));
	set_train_speed("0");
	train_switch_directions();
	train_set_switch('8', 'G');
	
	//	Waiting for Zamboni
	if(zamboniConfig)
	{
		wprintf(&train_wnd, "Waiting for Zamboni\n");
		while(!get_status_of_contact("7"));
	}
	
	//	train back to home
	wprintf(&train_wnd, "Going home\n");
	set_train_speed("5");
	//	wait for zamboni to pass home, before setting outer loop switch
	if(zamboniConfig)
	{
		while(!get_status_of_contact("4"));
	}
	train_set_switch('4', 'R');
	while(!get_status_of_contact("6"));
	while(get_status_of_contact("6"));
	set_train_speed("0");
	wprintf(&train_wnd, "Home Safe!\n");
	
	//Return outer loop continuity
	train_set_switch('4', 'G');
}

void runConfig4(int zamboniConfig)
{
	int on13 = 0;
	
	//	init switches
	train_set_switch('9', 'G');
	
	//	put train on outer loop, head for wagon
	wprintf(&train_wnd, "Turning around\n");
	set_train_speed("5");
	while(!get_status_of_contact("6"));
	set_train_speed("0");
	train_switch_directions();
	set_train_speed("5");
	
	//	redirect zamboni, wait at 3
	if(zamboniConfig)
	{
		wprintf(&train_wnd, "Redirecting Zamboni\n");
		while(get_status_of_contact("6"));
		train_set_switch('4', 'R');
		train_set_switch('3', 'G');

		wprintf(&train_wnd, "Waiting for Zamboni at track 3\n");
		while(!get_status_of_contact("3"));
		set_train_speed("0");
		while(!get_status_of_contact("14"));
		set_train_speed("5");
	}
	
	//	turn train around, connect to wagon
	wprintf(&train_wnd, "Getting the wagon\n");
	while(!get_status_of_contact("14"));
	set_train_speed("0");
	train_switch_directions();
	
	//	check to see that train is on track 13
	//	this determines how long the train should reverse on the spur
	if(get_status_of_contact("13"))
	{
		on13 = 1;
		wprintf(&train_wnd, "Stopped to reverse on track 13\n");
	}
	else
	{
		wprintf(&train_wnd, "Stopped to reverse on track 14\n");	
	}
	
	set_train_speed("5");
	while(get_status_of_contact("14"));
	
	//	carriage is on a spur, so wait a bit before stopping
	//	boolean will protect against inadvertent run-offs
	if(on13)
	{
		sleep(80);
	}
	else
	{
		sleep(5);	
	}
	set_train_speed("0");
	train_switch_directions();
	
	//	Wait for zamboni
	if(zamboniConfig)
	   {
			wprintf(&train_wnd, "Waiting for Zamboni");
			while(!get_status_of_contact("7"))
			{
				wprintf(&train_wnd, ".");
			}
		wprintf(&train_wnd, "\n");
	   }
	
	//	Go home
	wprintf(&train_wnd, "Going home\n");
	set_train_speed("5");
	train_set_switch('4', 'G');
	train_set_switch('3', 'R');
	//	wait for zamboni to pass before throwing switch
	if(zamboniConfig)
	   {
			wprintf(&train_wnd, "Waiting for Zamboni to pass home plate\n");
			while(!get_status_of_contact("4"));   
	   }
	train_set_switch('4', 'R');
	while(!get_status_of_contact("6"));
	while(get_status_of_contact("6"));
	set_train_speed("0");
	wprintf(&train_wnd, "Home Safe!\n");
	train_set_switch('4', 'G');
}

void get_wagon(int trainConfig, int zamboniConfig)
{
	if(trainConfig == 0)
	{
		runConfig1or2(zamboniConfig);		
	}
	if(trainConfig == 1)
	{
		runConfig3(zamboniConfig);		
	}
	if(trainConfig == 2)
	{
		runConfig4(zamboniConfig);		
	}
}

void train_process(PROCESS self, PARAM param)
{

	COM_Message msg;
	char buffer [128];
	msg.input_buffer = buffer;
	
	//	clear train window
	clear_window(&train_wnd);
	
	//	set up track to ensure safety of trains
	init_track();
	
	//	determine disposition of Zamboni
	int zamboniConfig = get_zamboni_condition();
	//int zamboniConfig = 0;
	
	//	determine config of train/wagon
	int trainConfig = get_train_wagon_config();
	
	get_wagon(trainConfig, zamboniConfig);
	
	while(1);
}

void init_train(WINDOW* wnd)
{
	train_port = create_process (train_process, 6, 0, "Train process");
}
