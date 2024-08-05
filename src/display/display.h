#ifndef DISPLAY_H

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32
#define SCALING_FACTOR 8

extern uint8_t display_pixels[DISPLAY_HEIGHT * DISPLAY_WIDTH];
extern int keyboard_state[16];

void init_display();
void eventHandler(int*);
void destroy_display();
int display();

#endif