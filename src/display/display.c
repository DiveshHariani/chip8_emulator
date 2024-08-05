#include <SDL_image.h>
#include <stdio.h>
#include <stdlib.h>

#include "display.h"

SDL_Window* window;
SDL_Renderer* renderer;
// int display_pixels[DISPLAY_HEIGHT][DISPLAY_WIDTH] = {0};

SDL_bool QUIT = SDL_FALSE;

/**
 * Keyboard Map -> Based on new keyboard layout
 */
SDL_Scancode keyboard_map[] = {
    SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, SDL_SCANCODE_4,
    SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, SDL_SCANCODE_8, SDL_SCANCODE_9,
    SDL_SCANCODE_A, SDL_SCANCODE_B, SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E,
    SDL_SCANCODE_F
};

uint8_t quitf = 0;

void init_display() {
    if (SDL_Init(SDL_INIT_EVENTS | SDL_INIT_VIDEO) < 0) {
        printf("Error initializing SDL %s", SDL_GetError());
        exit(1);
    }

    // Every pixel in the window is a square of size 10
    window = SDL_CreateWindow("Chip 8 Emulator", 
                                          SDL_WINDOWPOS_CENTERED, 
                                          SDL_WINDOWPOS_CENTERED, 
                                          DISPLAY_WIDTH * SCALING_FACTOR, 
                                          DISPLAY_HEIGHT * SCALING_FACTOR,
                                          SDL_WINDOW_OPENGL);
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    display();  // initial display
}

void destroy_display(void) {
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void eventHandler(int* keyboard_state) {
    SDL_Event e;
    if (SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_QUIT: {
                quitf = 1;
                break;
            }
            case SDL_KEYDOWN: {
                const Uint8* STATE = SDL_GetKeyboardState(NULL);
                printf("Key is pressed. Processing an event... \n");
                for (int i = 0; i < 16; i++) {
                    keyboard_state[i] = STATE[keyboard_map[i]];
                }
                printf("Keyboard processed \n");
                break;
            }
        }
    }
}

int display() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);  // Make Screen Black

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);   // Drawing in White
        
    for (int y = 0; y < 32; y++) {
        for (int x = 0; x < 64; x++) {
            if (display_pixels[x + (y * 64)]) {
                SDL_Rect rect;

                rect.x = x * 8;
                rect.y = y * 8;
                rect.w = 8;
                rect.h = 8;

                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }
    SDL_RenderPresent(renderer);
    return 0;
}