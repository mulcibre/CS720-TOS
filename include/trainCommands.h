#ifndef __TRAIN_COMMANDS__
#define __TRAIN_COMMANDS__

#define WAIT_TICKS     25

void set_train_speed(char* speed);
void train_clear_mem_buffer();
void train_set_switch(char* switchNum, char* position);
void switch_train_direction(int speed);

int get_status_of_contact(int contact);

void send_train_command(char* outBuf, char* inBuf, int len_inBuf);

#endif