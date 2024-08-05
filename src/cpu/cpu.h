#ifndef CPU_H_

extern uint8_t drawf;
extern uint8_t quit;
extern int keyboard_state[16];

void init_cpu();
void load_program(char*);
void emulate_cycle();

#endif