#ifndef __TRAIN_COMMANDS__
#define __TRAIN_COMMANDS__

void set_train_speed_new(WINDOW* wnd, char* speed);
void train_clear_mem_buffer();
void train_set_switch(WINDOW* wnd, char* switchNum, char* position);
void switch_train_direction(WINDOW* wnd);

int get_status_of_contact(WINDOW* wnd, int contact);

void send_train_command_new(WINDOW* wnd, char* outBuf, char* inBuf, int len_inBuf);

#endif