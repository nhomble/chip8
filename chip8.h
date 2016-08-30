#ifndef CHIP8_H
#define CHIP8_H

#define MEMORY_SIZE 4096
#define REGISTERS_SIZE 16
#define STACK_SIZE 16

#define ROM_SIZE 4096
#define ROM_START 0x200

#define SCREEN_WIDTH 32
#define SCREEN_LENGTH 64

#define SPRITE_HEIGHT 5
#define SPRITE_WIDTH 4

#define NUM_KEYS 16

#define PC_START 0x200

#define true 1
#define false 0

#define E_END 999
#define E_WHATOP -2
#define E_DEPOP -3
#define E_LROM -4
#define E_NOFILE -5

struct chip8{
	unsigned short opcode;
	unsigned short I;					// index register
	unsigned short pc;					// program counter
	unsigned char delay_timer;
	unsigned char sound_timer;
	unsigned short sp;					// stack pointer
	unsigned char should_draw;
	unsigned char rand_seeded;
	unsigned char memory[MEMORY_SIZE];
	unsigned char V[REGISTERS_SIZE];			// registers
	unsigned char gfx[SCREEN_LENGTH][SCREEN_WIDTH];		// graphics (ncurses)
	unsigned short stack[STACK_SIZE];
	unsigned char keys[NUM_KEYS];				// boolean, is it pressed
	int error;
};

void clear_graphics(struct chip8 *cpu);
void clear_registers(struct chip8 *cpu);
void load_rom(struct chip8 *cpu, char *path);
void init_chip(struct chip8 *cpu);
void get_keys(struct chip8 *cpu);
void do_cycle(struct chip8 *cpu);

#endif
