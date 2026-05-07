#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "emulator.h"

uint8_t init_memory[180] = {
    // ORIGINAL
    0xF0, 0x90, 0x90, 0x90, 0xF0, 0x20, 0x60, 0x20, 0x20, 0x70, 0xF0, 0x10,
    0xF0, 0x80, 0xF0, 0xF0, 0x10, 0xF0, 0x10, 0xF0, 0x90, 0x90, 0xF0, 0x10,
    0x10, 0xF0, 0x80, 0xF0, 0x10, 0xF0, 0xF0, 0x80, 0xF0, 0x90, 0xF0, 0xF0,
    0x10, 0x20, 0x40, 0x40, 0xF0, 0x90, 0xF0, 0x90, 0xF0, 0xF0, 0x90, 0xF0,
    0x10, 0xF0, 0xF0, 0x90, 0xF0, 0x90, 0x90, 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    0xF0, 0x80, 0x80, 0x80, 0xF0, 0xE0, 0x90, 0x90, 0x90, 0xE0, 0xF0, 0x80,
    0xF0, 0x80, 0xF0, 0xF0, 0x80, 0xF0, 0x80, 0x80,

    // SUPER
    0x3C, 0x7E, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0xC3, 0x7E, 0x3C, 0x18, 0x38,
    0x58, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C, 0x3E, 0x7F, 0xC1, 0x01,
    0x01, 0x1F, 0x7E, 0xC0, 0xFF, 0xFF, 0x3E, 0x7F, 0xC1, 0x01, 0x1F, 0x1F,
    0x01, 0xC1, 0x7F, 0x3E, 0x03, 0x0F, 0x1B, 0x33, 0x63, 0xFF, 0xFF, 0x03,
    0x03, 0x03, 0xFF, 0xFF, 0xC0, 0xC0, 0xFC, 0xFE, 0x03, 0x03, 0xFF, 0xFF,
    0x1C, 0x3E, 0x60, 0xC0, 0xFC, 0xFE, 0xC3, 0xC3, 0x7E, 0x3C, 0xFF, 0xFF,
    0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x60, 0x60, 0x3C, 0x7E, 0xC3, 0xC3,
    0x7E, 0x7E, 0xC3, 0xC3, 0x7E, 0x3C, 0x3C, 0x7E, 0xC3, 0xC3, 0x7F, 0x3F,
    0x03, 0x03, 0x7E, 0x3C
};

void emulator_init(Emulator *emulator) {
    // Emulator
    emulator->original = TRUE;
    emulator->shader = TRUE;
    emulator->pc_init = 0x200;
    srand(time(NULL));

    // Quirks
    emulator->quirk_shift = FALSE;
    emulator->quirk_increment_i_by_register = FALSE;
    emulator->quirk_no_increment_i = FALSE;
    emulator->quirk_wrap = FALSE;
    emulator->quirk_jump = FALSE;
    emulator->quirk_blank = FALSE;
    emulator->quirk_logic = FALSE;

    // Memory
    memset(emulator->memory, 0, sizeof(emulator->memory));

    for(int i = 0; i < 180; i++) {
        emulator->memory[i] = init_memory[i];
    }

    // Registers
    memset(emulator->registers, 0, sizeof(emulator->registers));
    emulator->pc = emulator->pc_init;
    emulator->i = 0;

    // Stack
    memset(emulator->stack, 0, sizeof(emulator->stack));
    emulator->sp = 0;

    // I/O
    memset(emulator->keypad, 0, sizeof(emulator->keypad));
    memset(emulator->keypad_memory, 0, sizeof(emulator->keypad_memory));
    emulator->key_pressed = 0;

    memset(emulator->display, 0, sizeof(emulator->display));
    emulator->display_rendered = FALSE;

    // Timers
    emulator->delay_timer = 0;
    emulator->sound_timer = 0;
}

void emulator_restart(Emulator *emulator) {
    // Registers
    memset(emulator->registers, 0, sizeof(emulator->registers));
    emulator->pc = emulator->pc_init;
    emulator->i = 0;

    // Stack
    memset(emulator->stack, 0, sizeof(emulator->stack));
    emulator->sp = 0;

    // I/O
    memset(emulator->keypad, 0, sizeof(emulator->keypad));
    memset(emulator->keypad_memory, 0, sizeof(emulator->keypad_memory));
    emulator->key_pressed = 0;

    memset(emulator->display, 0, sizeof(emulator->display));
    emulator->display_rendered = FALSE;

    // Timers
    emulator->delay_timer = 0;
    emulator->sound_timer = 0;
}

int emulator_load_rom(Emulator *emulator, const char *rom) {
    FILE *file = fopen(rom, "rb");

    if(file == NULL) {
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_s = ftell(file);
    rewind(file);

    if(file_s > (4096 - 0x200)) {
        fclose(file);
        return 1;
    }

    fread(&emulator->memory[emulator->pc_init], 1, file_s, file);

    fclose(file);
    return 0;
}

int emulator_load_file(Emulator *emulator, const char *rom) {
    char buffer[10];
    FILE *file = fopen(rom, "r");

    if(file == NULL) {
        return 1;
    }

    fseek(file, 0, SEEK_END);
    long file_s = ftell(file);
    rewind(file);

    if(file_s > (4096 - 0x200)) {
        fclose(file);
        return 1;
    }

    int i = 0;

    while(fgets(buffer, 3, file)) {
        if(strcmp(buffer, "\n") != 0) {
            emulator->memory[emulator->pc + i] = strtol(buffer, NULL, 16);
            i++;
        }
    }

    fclose(file);
    return 0;
}

void emulator_step(Emulator *emulator) {
    // Fetch
    uint16_t op_code = (emulator->memory[emulator->pc] << 8) | emulator->memory[emulator->pc + 1];
    uint8_t x = (op_code & 0x0F00) >> 8;
    uint8_t y = (op_code & 0x00F0) >> 4;

    // Debug
    //emulator_debug(emulator);

    // Step
    emulator->pc += 2;
    
    // Decode
    switch ((op_code & 0xF000) >> 12) {
    case 0x1:
        emulator->pc = op_code & 0x0FFF;
        break;
    case 0xB:
        if(emulator->quirk_jump) {
            emulator->pc = ((op_code & 0x0FFF) + emulator->registers[x]) & 0x0FFF;
        }else {
            emulator->pc = ((op_code & 0x0FFF) + emulator->registers[0]) & 0x0FFF;
        }
        break;
    case 0x2:
        emulator->stack[emulator->sp] = emulator->pc;
        emulator->sp++;

        emulator->pc = op_code & 0x0FFF;
        break;
    case 0x0:
        if((op_code & 0x00FF) == 0xEE) {
            emulator->sp--;
            emulator->pc = emulator->stack[emulator->sp];
        }else if((op_code & 0x00FF) == 0xE0) {
            emulator->display_rendered = FALSE;
            memset(emulator->display, 0, sizeof(emulator->display));
        }else if((op_code & 0x00FF) == 0x00) {

        }else if((op_code & 0x00FF) == 0xFE) {
            emulator->original = TRUE;
            memset(emulator->display, 0, sizeof(emulator->display));
        }else if((op_code & 0x00FF) == 0xFF) {
            emulator->original = FALSE;
            memset(emulator->display, 0, sizeof(emulator->display));
        }else if((op_code & 0x00F0) == 0x00C0) {
            uint8_t screen_width = (emulator->original)? 64: 128;
            uint8_t screen_height = (emulator->original)? 32: 64;
            
            uint16_t slide = (op_code & 0x000F) * screen_width;

            if(slide < screen_width * screen_height) {
                memmove(emulator->display + slide, emulator->display, screen_width * screen_height - slide);
                memset(emulator->display, 0, slide);
            }else {
                memset(emulator->display, 0, screen_width * screen_height);
            }
        }else if((op_code & 0x00FF) == 0x00FB) {
            uint8_t screen_width = (emulator->original)? 64: 128;
            uint8_t screen_height = (emulator->original)? 32: 64;

            for(int i = 0; i < screen_height; i++) {
                uint8_t *row = &emulator->display[i * screen_width];
                memmove(row + 4, row, screen_width - 4);
                memset(row, 0, 4);
            }
        }else if((op_code & 0x00FF) == 0x00FC) {
            uint8_t screen_width = (emulator->original)? 64: 128;
            uint8_t screen_height = (emulator->original)? 32: 64;

            for(int i = 0; i < screen_height; i++) {
                uint8_t *row = &emulator->display[i * screen_width];
                memmove(row, row + 4, screen_width - 4);
                memset(row + (screen_width - 4), 0, 4);
            }
        }
        break;
    case 0x3:
        if(emulator->registers[x] == (op_code & 0x00FF)) {
            emulator->pc += 2;
        }
        break;
    case 0x4:
        if(emulator->registers[x] != (op_code & 0x00FF)) {
            emulator->pc += 2;
        }
        break;
    case 0x5:
        if(emulator->registers[x] == emulator->registers[y]) {
            emulator->pc += 2;
        }
        break;
    case 0x9:
        if(emulator->registers[x] != emulator->registers[y]) {
            emulator->pc += 2;
        }
        break;
    case 0xE:
        if((op_code & 0x00FF) == 0x9E) {
            if(emulator->keypad[emulator->registers[x]] == 1) {
                emulator->pc += 2;
            }
        }else if((op_code & 0x00FF) == 0xA1) {
            if(emulator->keypad[emulator->registers[x]] == 0) {
                emulator->pc += 2;
            }
        }
        break;
    case 0x6:
        emulator->registers[x] = op_code & 0x00FF;
        break;
    case 0xC:
        emulator->registers[x] = (rand() % 256) & (op_code & 0x00FF);
        break;
    case 0x7:
        emulator->registers[x] += op_code & 0x00FF;
        break;
    case 0x8:
        if((op_code & 0x000F) == 0x0) {
            emulator->registers[x] = emulator->registers[y];
        }else if((op_code & 0x000F) == 0x1) {
            if(emulator->quirk_logic) {
                emulator->registers[x] |= emulator->registers[y];
                emulator->registers[0xF] = 0;
            }else {
                emulator->registers[x] |= emulator->registers[y];
            }
        }else if((op_code & 0x000F) == 0x2) {
            if(emulator->quirk_logic) {
                emulator->registers[x] &= emulator->registers[y];
                emulator->registers[0xF] = 0;
            }else {
                emulator->registers[x] &= emulator->registers[y];
            }
        }else if((op_code & 0x000F) == 0x3) {
            if(emulator->quirk_logic) {
                emulator->registers[x] ^= emulator->registers[y];
                emulator->registers[0xF] = 0;
            }else {
                emulator->registers[x] ^= emulator->registers[y];
            }
        }else if((op_code & 0x000F) == 0x4) {
            uint16_t sum = emulator->registers[x] + emulator->registers[y];

            emulator->registers[x] = sum & 0xFF;
            emulator->registers[0xF] = (sum > 0xFF)? 1: 0;
        }else if((op_code & 0x000F) == 0x5) {
            uint8_t op0 = emulator->registers[x];
            uint16_t sub = op0 - emulator->registers[y];

            emulator->registers[x] = sub;
            emulator->registers[0xF] = (op0 >= emulator->registers[y])? 1: 0;
        }else if((op_code & 0x000F) == 0x6) {
            if(emulator->quirk_shift) {
                uint8_t op0 = emulator->registers[x];
                emulator->registers[x] >>= 1;
                emulator->registers[0xF] = op0 & 0x01;
            }else {
                uint8_t op0 = emulator->registers[y];
                emulator->registers[x] = op0 >> 1;
                emulator->registers[0xF] = op0 & 0x01;
            }
        }else if((op_code & 0x000F) == 0x7) {
            uint8_t op1 = emulator->registers[y];
            uint16_t sub = op1 - emulator->registers[x];

            emulator->registers[x] = sub;
            emulator->registers[0xF] = (op1 >= emulator->registers[x])? 1: 0;
        }else if((op_code & 0x000F) == 0xE) {
            if(emulator->quirk_shift) {
                uint8_t op0 = emulator->registers[x];
                emulator->registers[x] <<= 1;
                emulator->registers[0xF] = (op0 & 0x80) >> 0x07;
            }else {
                uint8_t op0 = emulator->registers[y];
                emulator->registers[x] = op0 << 1;
                emulator->registers[0xF] = (op0 & 0x80) >> 0x07;
            }
        }
        break;
    case 0xF:
        if((op_code & 0x00FF) == 0x07) {
            emulator->registers[x] = emulator->delay_timer;
        }else if((op_code & 0x00FF) == 0x0A) {
            for(int i = 0; i < 0x10; i++) {
                if(emulator->keypad[i]) {
                    emulator->keypad_memory[i] = emulator->keypad[i];
                    emulator->key_pressed = i;
                    break;
                }
            }

            if(emulator->keypad[emulator->key_pressed] == 0 && emulator->keypad_memory[emulator->key_pressed] == 1) {
                emulator->registers[x] = emulator->key_pressed;
                emulator->keypad_memory[emulator->key_pressed] = 0;
                emulator->key_pressed = 0;
                
            }else {
                emulator->pc -= 2;
            }
        }else if((op_code & 0x00FF) == 0x15) {
            emulator->delay_timer = emulator->registers[x];
        }else if((op_code & 0x00FF) == 0x18) {
            emulator->sound_timer = emulator->registers[x];
        }else if((op_code & 0x00FF) == 0x1E) {
            emulator->i += emulator->registers[x];
        }else if((op_code & 0x00FF) == 0x29) {
            emulator->i = (emulator->registers[x] & 0x0F) * 5;
        }else if((op_code & 0x00FF) == 0x33) {
            emulator->memory[emulator->i & 0x0FFF] = emulator->registers[x] / 100;
            emulator->memory[(emulator->i + 1) & 0x0FFF] = (emulator->registers[x] / 10) % 10;
            emulator->memory[(emulator->i + 2) & 0x0FFF] = emulator->registers[x] % 10;
        }else if((op_code & 0x00FF) == 0x55) {
            for(int i = 0; i <= x; i++) {
                emulator->memory[emulator->i + i] = emulator->registers[i];
            }

            if(!emulator->quirk_no_increment_i) {
                if(emulator->quirk_increment_i_by_register) {
                    emulator->i += (x + 1);
                }else {
                    emulator->i += x;
                }
            }
        }else if((op_code & 0x00FF) == 0x65) {
            for(int i = 0; i <= x; i++) {
                emulator->registers[i] = emulator->memory[i + emulator->i];
            }

            if(!emulator->quirk_no_increment_i) {
                if(emulator->quirk_increment_i_by_register) {
                    emulator->i += (x + 1);
                }else {
                    emulator->i += x;
                }
            }
        }else if((op_code & 0x00FF) == 0xFF) {
            emulator->original = FALSE;
            memset(emulator->display, 0, sizeof(emulator->display));
        }else if((op_code & 0x00FF) == 0xFE) {
            emulator->original = TRUE;
            memset(emulator->display, 0, sizeof(emulator->display));
        }
        break;
    case 0xA:
        emulator->i = op_code & 0x0FFF;
        break;
    case 0xD:
        emulator->registers[0xF] = 0;

        if(emulator->quirk_blank) {
            if(emulator->display_rendered == TRUE) {
                emulator->pc -= 2;
                return;
            }

            uint8_t screen_width = (emulator->original)? 64: 128;
            uint8_t screen_height = (emulator->original)? 32: 64;
            
            uint8_t height = op_code & 0x000F;

            if(!emulator->original && (op_code & 0x000F) == 0x0) {
                height = 16;
            }

            for(int row = 0; row < height; row++) {
                int bytes = (!emulator->original && (op_code & 0x000F) == 0x0)? 2: 1;

                for(int b = 0; b < bytes; b++) {
                    uint8_t byte = emulator->memory[emulator->i + row * bytes + b];

                    for(int bit = 0; bit < 8; bit++) {
                        uint8_t pixel = (byte & (0x80 >> bit))? 1: 0;

                        if(pixel) {
                            if(!emulator->quirk_wrap) {
                                uint16_t screen_x = ((emulator->registers[x] + (b * 8)) % screen_width) + bit;
                                uint16_t screen_y = (emulator->registers[y] % screen_height) + row;

                                uint16_t i = (screen_y * screen_width) + screen_x;

                                if (screen_x < screen_width && screen_y < screen_height) {
                                    if (emulator->display[i] == 1) {
                                        emulator->registers[0xF] = 1;
                                    }

                                    emulator->display[i] ^= 1;
                                }
                            }else {
                                uint16_t screen_x = (emulator->registers[x] + (b * 8) + bit) % screen_width;
                                uint16_t screen_y = (emulator->registers[y] + row) % screen_height;

                                uint16_t i = (screen_y * screen_width) + screen_x;
                                if (emulator->display[i] == 1) {
                                    emulator->registers[0xF] = 1;
                                }

                                emulator->display[i] ^= 1;
                            }
                        }
                    }
                }
            }

            emulator->display_rendered = TRUE;
        }else {
            uint8_t screen_width = (emulator->original)? 64: 128;
            uint8_t screen_height = (emulator->original)? 32: 64;
            
            uint8_t height = op_code & 0x000F;

            if(!emulator->original && (op_code & 0x000F) == 0x0) {
                height = 16;
            }

            for(int row = 0; row < height; row++) {
                int bytes = (!emulator->original && (op_code & 0x000F) == 0x0)? 2: 1;

                for(int b = 0; b < bytes; b++) {
                    uint8_t byte = emulator->memory[emulator->i + row * bytes + b];

                    for(int bit = 0; bit < 8; bit++) {
                        uint8_t pixel = (byte & (0x80 >> bit))? 1: 0;

                        if(pixel) {
                            if(!emulator->quirk_wrap) {
                                uint16_t screen_x = ((emulator->registers[x] + (b * 8)) % screen_width) + bit;
                                uint16_t screen_y = (emulator->registers[y] % screen_height) + row;

                                uint16_t i = (screen_y * screen_width) + screen_x;

                                if (screen_x < screen_width && screen_y < screen_height) {
                                    if (emulator->display[i] == 1) {
                                        emulator->registers[0xF] = 1;
                                    }

                                    emulator->display[i] ^= 1;
                                }
                            }else {
                                uint16_t screen_x = (emulator->registers[x] + (b * 8) + bit) % screen_width;
                                uint16_t screen_y = (emulator->registers[y] + row) % screen_height;

                                uint16_t i = (screen_y * screen_width) + screen_x;
                                if (emulator->display[i] == 1) {
                                    emulator->registers[0xF] = 1;
                                }

                                emulator->display[i] ^= 1;
                            }
                        }
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}

void emulator_debug(Emulator *emulator) {
    printf("-------------------------\n");
    printf("OPCODE: %02x-%02x PC: %02x I: %02x\n", emulator->memory[emulator->pc], emulator->memory[emulator->pc + 1], emulator->pc, emulator->i);
    printf("R00: %02x R01: %02x R02: %02x R03: %02x R04: %02x\n", emulator->registers[0], emulator->registers[1], emulator->registers[2] ,emulator->registers[3] ,emulator->registers[4]);
    printf("R05: %02x R06: %02x R07: %02x R08: %02x R09: %02x\n", emulator->registers[5], emulator->registers[6], emulator->registers[7] ,emulator->registers[8] ,emulator->registers[9]);
    printf("R10: %02x R11: %02x R12: %02x R13: %02x R14: %02x\n", emulator->registers[10], emulator->registers[11], emulator->registers[12] ,emulator->registers[13] ,emulator->registers[14]);
    printf("R15: %02x\n\n", emulator->registers[15]);

    printf("S00: %02x S01: %02x S02: %02x S03: %02x S04: %02x\n", emulator->stack[0], emulator->stack[1], emulator->stack[2] ,emulator->stack[3] ,emulator->stack[4]);
    printf("S05: %02x S06: %02x S07: %02x S08: %02x S09: %02x\n", emulator->stack[5], emulator->stack[6], emulator->stack[7] ,emulator->stack[8] ,emulator->stack[9]);
    printf("S10: %02x S11: %02x S12: %02x S13: %02x S14: %02x\n", emulator->stack[10], emulator->stack[11], emulator->stack[12] ,emulator->stack[13] ,emulator->stack[14]);
    printf("S15: %02x SP: %02x\n\n", emulator->stack[15], emulator->sp);

    printf("[00 + I]: %02x [01 + I]: %02x [02 + I]: %02x [03 + I]: %02x [04 + I]: %02x\n", emulator->memory[0 + emulator->i], emulator->memory[1 + emulator->i], emulator->memory[2 + emulator->i] ,emulator->memory[3 + emulator->i] ,emulator->memory[4 + emulator->i]);
    printf("[05 + I]: %02x [06 + I]: %02x [07 + I]: %02x [08 + I]: %02x [09 + I]: %02x\n", emulator->memory[5 + emulator->i], emulator->memory[6 + emulator->i], emulator->memory[7 + emulator->i] ,emulator->memory[8 + emulator->i] ,emulator->memory[9 + emulator->i]);
    printf("[10 + I]: %02x [11 + I]: %02x [12 + I]: %02x [13 + I]: %02x [14 + I]: %02x\n", emulator->memory[10 + emulator->i], emulator->memory[11 + emulator->i], emulator->memory[12 + emulator->i] ,emulator->memory[13 + emulator->i] ,emulator->memory[14]);
    printf("[15 + I]: %02x\n\n", emulator->memory[15], emulator->sp);
}