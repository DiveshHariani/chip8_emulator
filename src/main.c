#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <unistd.h>

#include "display/display.h"
#include "cpu/cpu.h"

extern uint8_t drawf;
extern uint8_t quitf;

int main(int argc, char** argv) {
    if (argc != 2) {
        printf("Invalid arguments!\n");
        return 1;
    }

    printf("Initializing Emulator...\n");
    init_cpu();
    init_display();
    printf("Emulator Initialized\n");

    printf("Loading Program... \n");
    char *program = argv[1];
    load_program(program);
    printf("Program loaded \n");
    

    while(quitf != 1) {
        emulate_cycle();
        eventHandler(keyboard_state);
        if(drawf) {
            display();
        }

        usleep(2000);   // To emulate clock speed of 500Hz
    }

    destroy_display();

    return 0;
}