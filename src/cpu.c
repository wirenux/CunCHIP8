#include "../includes/cpu.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define INCREMENT_I_AFTER_LD 0  // 1 = SCHIP mode

// intergrated font
static const uint8_t chip8_fontset[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init(Chip8 *chip) {
    memset(chip, 0, sizeof(Chip8));
    chip->pc = 0x200;
    chip->I = 0;
    chip->sp = 0;
    chip->draw_flag = false;

    // load font in memory
    memcpy(&chip->memory[0x50], chip8_fontset, sizeof(chip8_fontset));

    srand((unsigned) time(NULL));
}

int chip8_load_program(Chip8 *chip, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "❌ Connot open file %s\n", filename);
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (size + 0x200 > 4096) {
        fprintf(stderr, "❌ ROM file too large (%ld bytes)\n", size);
        fclose(file);
        return 1;
    }

    fread(&chip->memory[0x200], 1, size, file);
    fclose(file);

    printf("✅ ROM loaded (%ld byte) to 0x200\n", size);
    return 0;
}

void chip8_update_timers(Chip8 *chip) {
    if (chip->delay_timer > 0)
        chip->delay_timer--;
    if (chip->sound_timer > 0)
        chip->sound_timer--;
}

void chip8_emulation_cycle(Chip8 *chip) {
    uint16_t opcode = (chip->memory[chip->pc] << 8) | chip->memory[chip->pc + 1];
    uint8_t x = (opcode & 0x0F00) >> 8;
    uint8_t y = (opcode & 0x00F0) >> 4;
    uint8_t nn = opcode & 0x00FF;
    uint16_t nnn = opcode & 0x0FFF;

    switch (opcode & 0xF000) {
        case 0x0000:
            switch (opcode) {
                case 0x00E0: // CLS
                    memset(chip->gfx, 0, sizeof(chip->gfx));
                    chip->draw_flag = true;
                    chip->pc += 2;
                    break;

                case 0x00EE: // RET
                    if (chip->sp > 0) {
                        chip->sp--;
                        chip->pc = chip->stack[chip->sp];
                        // Note: No pc += 2 here because we saved the address 
                        // of the *next* instruction during the CALL.
                    } else {
                        fprintf(stderr, "Stack underflow at PC: 0x%03X\n", chip->pc);
                    }
                    break;
            }
            break;

        case 0x1000: // JP addr
            chip->pc = nnn;
            break;

        case 0x2000: // CALL nnn
            chip->stack[chip->sp] = chip->pc + 2; // Save NEXT instruction
            chip->sp++;
            chip->pc = nnn;                       // Jump to subroutine
            break;

        case 0x3000:
            if (chip->V[x] == nn) {
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;

        case 0x4000:
            if (chip->V[x] != nn) {
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;

        case 0x5000:
            if (chip->V[x] == chip->V[y]) {
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;

        case 0x6000: // LD Vx, byte
            chip->V[x] = nn;
            chip->pc += 2;
            break;

        case 0x7000: // ADD Vx, byte
            chip->V[x] += nn;
            chip->pc += 2;
            break;

        case 0x8000:
            switch (opcode & 0x000F) {
                case 0x0: chip->V[x] = chip->V[y]; break;
                case 0x1: chip->V[x] |= chip->V[y]; break;
                case 0x2: chip->V[x] &= chip->V[y]; break;
                case 0x3: chip->V[x] ^= chip->V[y]; break;
                case 0x4: {
                    uint16_t sum = chip->V[x] + chip->V[y];
                    chip->V[0xF] = (sum > 255) ? 1 : 0;
                    chip->V[x] = sum & 0xFF;
                    break;
                }
                case 0x5: {
                    chip->V[0xF] = (chip->V[x] > chip->V[y]) ? 1 : 0;
                    chip->V[x] -= chip->V[y];
                    break;
                }
                case 0x6: {
                    chip->V[0xF] = chip->V[x] & 0x1;
                    chip->V[x] >>= 1;
                    break;
                }
                case 0x7: {
                    chip->V[0xF] = (chip->V[y] > chip->V[x]) ? 1 : 0;
                    chip->V[x] = chip->V[y] - chip->V[x];
                    break;
                }
                case 0xE: {
                    chip->V[0xF] = (chip->V[x] & 0x80) >> 7;
                    chip->V[x] <<= 1;
                    break;
                }
                default:
                    printf("⚠️ Unknow 8XY? OPCODE : 0x%04X\n", opcode);
                    break;
            }
            chip->pc += 2;
            break;


        case 0x9000:
            if (chip->V[x] != chip->V[y]) {
                chip->pc += 4;
            } else {
                chip->pc += 2;
            }
            break;

        case 0xA000: // LD I, addr
            chip->I = nnn;
            chip->pc += 2;
            break;

        case 0xB000:
            chip->pc =chip->V[0]+ nnn;
            break;

        case 0xC000: // RND Vx, byte
            chip->V[x] = (rand() % 256) & nn;
            chip->pc += 2;
            break;

        case 0xD000: { // DRW Vx, Vy, nibble
            uint8_t vx = chip->V[x] % 64;
            uint8_t vy = chip->V[y] % 32;
            uint8_t height = opcode & 0x000F;
            chip->V[0xF] = 0;



            for (int yline = 0; yline < height; yline++) {
                uint8_t pixel = chip->memory[chip->I + yline];
                for (int xline = 0; xline < 8; xline++) {
                    if (pixel & (0x80 >> xline)) {
                        int px = (vx + xline) % 64;
                        int py = (vy + yline) % 32;
                        int index = px + (py * 64);
                        if (chip->gfx[index] == 1)
                            chip->V[0xF] = 1;
                        chip->gfx[index] ^= 1;
                    }
                }
            }
            chip->draw_flag = true;
            chip->pc += 2;
            break;
        }

        case 0xE000:
            switch (opcode & 0x00FF) {
                case 0x9E: // SKP Vx
                    if (chip->key[chip->V[x]]) chip->pc += 4;
                    else chip->pc += 2;
                    break;
                case 0xA1: // SKNP Vx
                    if (!chip->key[chip->V[x]]) chip->pc += 4;
                    else chip->pc += 2;
                    break;
            }
            break;


        case 0xF000:
            switch (opcode & 0x00FF) {
                case 0x07: chip->V[x] = chip->delay_timer; chip->pc+=2; break;
                case 0x0A: {
                    bool key_pressed = false;
                    for (int i = 0; i < 16; i++) {
                        if (chip->key[i]) {
                            chip->V[x] = i;
                            key_pressed = true;
                            break;
                        }
                    }
                    if (!key_pressed) return;
                    chip->pc += 2;
                    break;
                }
                case 0x15: chip->delay_timer = chip->V[x]; chip->pc+=2; break;
                case 0x18: chip->sound_timer = chip->V[x]; chip->pc+=2; break;
                case 0x1E: chip->I += chip->V[x]; chip->pc+=2; break;
                case 0x29: chip->I = 0x50 + (chip->V[x]&0xF)*5; chip->pc+=2; break;
                case 0x33:
                    chip->memory[chip->I] = chip->V[x]/100;
                    chip->memory[chip->I+1] = (chip->V[x]/10)%10;
                    chip->memory[chip->I+2] = chip->V[x]%10;
                    chip->pc += 2;
                    break;
                case 0x55: {
                    for (int i = 0; i <= x; i++)
                        chip->memory[chip->I + i] = chip->V[i];

                #if INCREMENT_I_AFTER_LD
                    chip->I += x + 1;
                #endif

                    chip->pc += 2;
                    break;
                }

                case 0x65: {
                    for (int i = 0; i <= x; i++)
                        chip->V[i] = chip->memory[chip->I + i];

                #if INCREMENT_I_AFTER_LD
                    chip->I += x + 1;
                #endif

                    chip->pc += 2;
                    break;
                }

                default:
                    printf("⚠️ Unknow FX OPCODE: 0x%04X\n", opcode);
                    chip->pc+=2;
                    break;
            }
            break;


        default:
            printf("⚠️ Unknow OPCODE: 0x%04X\n", opcode);
            chip->pc += 2;
            break;
    }
    // == Debug Mode: main.c ==
    if (debug) {
        printf("Opcode: %04X (PC=%03X)\n", opcode, chip->pc);
    }
}
