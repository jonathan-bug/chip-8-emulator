#ifndef EMULATOR_H
#define EMULATOR_H
#define TRUE 1
#define FALSE 0

#include <stdint.h>

typedef struct {
    // Emulator
    uint8_t original;
    uint8_t shader;
    uint16_t pc_init;

    // Quirks
    uint8_t quirk_shift;
    uint8_t quirk_increment_i_by_register;  // ORIGINAL == TRUE
    uint8_t quirk_no_increment_i;           // ORIGINAL == FALSE
    uint8_t quirk_wrap;
    uint8_t quirk_jump;
    uint8_t quirk_blank;
    uint8_t quirk_logic;

    // Memory
    uint8_t memory[4096];

    // Registers
    uint8_t registers[16];
    uint16_t pc;
    uint16_t i;

    // Stack
    uint16_t stack[16];
    uint8_t sp;

    // I/O
    uint8_t keypad[16];
    uint8_t keypad_memory[16];
    uint8_t key_pressed;

    uint8_t display[128 * 64];
    uint8_t display_rendered;

    // Timers
    uint8_t delay_timer;
    uint8_t sound_timer;
} Emulator;

void emulator_init(Emulator *);
int emulator_load_rom(Emulator *, const char *);
int  emulator_load_file(Emulator *, const char *);
void emulator_step(Emulator *);
void emulator_debug(Emulator *);
void emulator_restart(Emulator *);

#endif