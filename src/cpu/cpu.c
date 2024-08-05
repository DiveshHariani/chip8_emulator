#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <time.h>
#include <SDL.h>

#include "../display/display.h"

// Fontset
unsigned char fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

// initialize memory of size 4KB = 4096 Bytes
uint8_t memory[4096] = {0};

// initialize 16 8-bit registers
uint8_t V[16] = {0};
uint8_t DT = 0; // Delay timers
uint8_t ST = 0; // Sound timers

uint16_t I; // register used to store memory addresses

uint8_t SP = 0;           // Stack Pointer
uint16_t stack[16] = {0}; // Stack (stores return values from subroutine)

uint16_t PC; // Program Counter

int keyboard_state[16] = {0};

uint8_t drawf = 0;

uint8_t display_pixels[DISPLAY_WIDTH * DISPLAY_HEIGHT] = {0};

void init_cpu() {
    PC = 0x200;
    srand((unsigned int)time(NULL));
    memcpy(memory, fontset, sizeof(fontset));
}

void load_program(char* program_name) {
    FILE* f = fopen(program_name, "rb");

    if (f == NULL) { 
        printf("Error opening file\n");
        exit(100);
    }

    struct stat filestat;
    stat(program_name, &filestat);
    size_t filesize = filestat.st_size;
    size_t bytes_read = fread(memory + 0x200, 1, sizeof(memory) - 0x200, f);
    fclose(f);

    if (bytes_read != filesize) { 
        printf("Error reading file\n");
        exit(200); 
    }
}

void emulate_cycle() {
    drawf = 0;

    int instruction = memory[PC] << 8 | memory[PC + 1];
    int x = (instruction & 0x0F00) >> 8;
    int y = (instruction & 0x00F0) >> 4;

    switch (instruction & 0xF000) {
        case 0x0000: {
            printf("0x0000 instruction\n");
            int nnn = instruction & 0x00FF;
            if (nnn == 0x00E0) {
                // clear the display
                for (int i = 0; i < DISPLAY_HEIGHT * DISPLAY_WIDTH; i++) {
                    display_pixels[i] = 0;
                }
            } else if (nnn == 0x00EE) {
                // return from subroutine
                PC = stack[SP--];
            } else {
                // jump to machine code routine at nnn
            }
            PC += 2;
            break;
        }
        case 0x1000: {
            printf("0x1000 instruction\n");
            int nnn = instruction & 0x0FFF;
            PC = nnn;
            break;
        }
        case 0x2000: {
            printf("0x2000 instruction\n");
            int nnn = instruction & 0x0FFF;
            // Call subroutine at nnn
            SP++;
            stack[SP] = PC;
            PC = nnn;
            break;
        }
        case 0x3000: {
            printf("0x3000 instruction\n");
            int kk = instruction & 0x00FF;
            if (V[x] == kk) {
                // Skip the instruction i.e. increment PC by 2
                PC += 2;
            }
            PC += 2;
            break;
        }
        case 0x4000: {
            printf("0x4000 instruction\n");
            int kk = instruction & 0x00FF;
            if (V[x] != kk) {
                // Skip the instruction i.e. increment PC by 2
                PC += 2;
            }
            PC += 2;
            break;
        }
        case 0x5000: {
            printf("0x5000 instruction\n");
            if (V[x] == V[y]) {
                // Skip the instruction i.e. increment PC by 2
                PC += 2;
            }
            PC += 2;
            break;
        }
        case 0x6000: {
            printf("0x6000 instruction\n");
            uint8_t kk = instruction & 0x00FF;
            V[x] = kk;
            PC += 2;
            break;
        }
        case 0x7000: {
            printf("0x7000 instruction\n");
            uint8_t kk = instruction & 0x00FF;
            V[x] = V[x] + kk;
            PC += 2;
            break;
        }
        case 0x8000: {
            printf("0x8000 instruction\n");
            int var = instruction & 0x000F;
            switch (var) {
                case 0x0000:
                    // 8xy0 => SET Vx = Vy
                    V[x] = V[y];
                    break;
                case 0x0001:
                    // 8xy1 => VX = Vx OR Vy
                    V[x] = V[x] | V[y];
                    break;
                case 0x0002:
                    // 8xy2 => Vx = Vx AND Vy
                    V[x] = V[x] & V[y];
                    break;
                case 0x0003:
                    // 8xy3 => Vx = Vx XOR Vy
                    V[x] = V[x] ^ V[y];
                    break;
                case 0x0004:
                    // 8xy4 => Vx = Vx + Vy
                    V[0xF] = ((V[x] + V[y]) > 0xFF) ? 1 : 0;
                    V[x] = V[x] + V[y];
                    break;
                case 0x0005:
                    // 8xy5 => Vx = Vx - Vy
                    V[0xF] = (V[x] > V[y]) ? 1 : 0;
                    V[x] = V[x] - V[y];
                    break;
                case 0x0006:
                    // 8xy6 => If the LSB of Vx is 1, then VF is set to 1, otherwise 0. Then Vx is divided by 2
                    V[0xF] = V[x] & 1;
                    V[x] >>= 1;
                    break;
                case 0x0007:
                    // 8xy7 => Set Vx = Vy - Vx, SET VF = NOT borrow
                    if (V[y] > V[x])    V[0xF] = 1;
                    else    V[0xF] = 0;
                    V[x] = V[y] - V[x];
                    break;
                case 0x000E:
                    // If the MSBit of Vx is 1, Set VF to 1, otherwise to 0. Then Vx is multiplied by 2
                    V[0xF] = (V[x] >> 7) & 0x1;
                    V[x] <<= 1;
                    break;
            }
            printf("Going to next instruction.\n");
            PC += 2;
            break;
        }
        case 0x9000: {
            printf("0x9000 instruction\n");
            // 9xy0 => Skip instruction if Vx != Vy
            if (V[x] != V[y]) {
                // Skip instruction, increment PC by 2
                PC += 2;
            }
            PC += 2;
            break;
        }
        case 0xA000: {
            printf("0xA000 instruction\n");
            // Annn, I = nnn
            uint16_t nnn = instruction & 0x0FFF;
            I = nnn;
            PC += 2;
            break;
        }
        case 0xB000: {
            printf("0xB000 instruction\n");
            // Bnnn => Jump to location nnn + V0
            uint16_t nnn = instruction & 0x0FFF;
            PC = nnn + V[0];
            break;
        }
        case 0xC000: {
            printf("0xC000 instruction\n");
            // Cxkk - Set Vx = random byte AND kk
            uint8_t kk = instruction & 0x00FF;
            uint16_t random_byte = rand() % 256;
            V[x] = random_byte & kk;
            PC += 2;
            break;
        }
        case 0xD000: {
            printf("0xD000 instruction\n");
            // Dxyn => Display n-bytes from memory, starting at address I. These bytes
            // are then displayed as sprites on screen at coordinates (Vx, Vy). Sprites
            // are XORed onto the existing screen. If this causes any pixels to be erased,
            // VF is set to 1, otherwise it is set to 0. If the sprite is positioned so part
            // of it is outside the coordinates of the display, it wraps arount to the
            // opposite end of the screen
            
            drawf = 1;

            uint16_t height = instruction & 0x000F;
            uint16_t px;

            V[0xF] = 0;

            for (int yline = 0; yline < height; yline++) {
                px = memory[I + yline];

                for (int xline = 0; xline < 8; xline++) {
                    if ((px & (0x80 >> xline)) != 0) {
                        if (display_pixels[(V[x] + xline + ((V[y] + yline) * 64))] ==
                            1) {
                            V[0xF] = 1;
                        }
                        display_pixels[V[x] + xline + ((V[y] + yline) * 64)] ^= 1;
                    }
                }
            }

            PC += 2;
            break;
        }
        case 0xE000: {
            printf("0xE000 instruction\n");
            int kk = instruction & 0x00FF;
            if (kk == 0x9E) {
                if (keyboard_state[V[x]]) {
                    PC += 2;
                }
            } else if (kk == 0xA1) {
                if (!keyboard_state[V[x]]) {
                    PC += 2;
                }
            }
            PC += 2;
            break;
        }
        case 0xF000: {
            printf("0xF000 instruction\n");
            int var = instruction & 0x00FF;
            switch(var) {
                case 0x07: {
                    // Fx07 => Set Vx = delay timer value.
                    V[x] = DT;
                    PC += 2;
                    break;
                }
                case 0x0A: {
                    // Fx0A => Wait for a key press, store the value of the key in Vx
                    for (int i = 0; i < 16; i++) {
                        if(keyboard_state[i]) {
                            V[x] = i;
                            PC += 2;
                            break;
                        }
                    }
                    break;
                }
                case 0x15: {
                    // Fx15 => Set delay timer = Vx.
                    DT = V[x];
                    PC += 2;
                    break;
                }
                case 0x18: {
                    // Fx18 => Set sound timer = Vx
                    ST = V[x];
                    PC += 2;
                    break;
                }
                case 0x1E: {
                    // Fx1E => Set I = I + Vx.
                    I = I + V[x];
                    PC += 2;
                    break;
                }
                case 0x29: {
                    // Fx29 => Set I = location of sprite for digit Vx.
                    I = V[x] * 5;   // For sprite of 5 bytes
                    PC += 2;
                    break;
                }
                case 0x33: {
                    // Fx33 => Store BCD representation of Vx in memory locations I, I+1, and I+2.
                    memory[I] = (V[x] % 1000) / 100;
                    memory[I + 1] = (V[x] % 100) / 10;
                    memory[I + 2] = V[x] % 10;

                    PC += 2;
                    break;
                }
                case 0x55: {
                    // Fx55 => Store registers V0 through Vx in memory starting at location I.
                    for (int i = 0; i <= x; i++) {
                        memory[I + i] = V[i];
                    }

                    PC += 2;
                    break;
                }
                case 0x65: {
                    // Fx65 => Read registers V0 through Vx from memory starting at location I.
                    for(int i = 0; i <= x; i++) {
                        V[i] = memory[I + i];
                    }
                    PC += 2;
                    break;
                }
            }
        }
    }

    if (DT > 0) DT--;
    if (ST > 0) {
        puts("BEEEEPPPPP");
        ST--;
    }
}