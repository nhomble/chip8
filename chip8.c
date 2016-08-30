#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>

#include "chip8.h"
#include "font.h"
#include "ops.h"

void load_rom(struct chip8 *cpu, char *rom_path){
	FILE* file;
	int read;
	file = fopen(rom_path, "rb");
	if(!file){
		cpu->error = E_NOFILE;
		return;
	}
	fseek(file, 0L, SEEK_END);
	rewind(file);
	read = fread(cpu->memory + ROM_START, sizeof(unsigned char), MEMORY_SIZE, file);
	fclose(file);

	cpu->error = !read;
}

// fetch opcode from memory and put into opcode
static inline void fetch_opcode(struct chip8 *cpu){
	unsigned short op;
	unsigned short pc;

	pc = cpu->pc;
	op = (cpu->memory[pc] << 8) | cpu->memory[pc + 1];
	cpu->opcode = op;
}

// map opcode to function in ops.c and execute
static inline void execute_opcode(struct chip8* cpu){
	unsigned char k___, __kk, ___k;
	k___ = (unsigned char) ((cpu->opcode & 0xF000) >> 12);
	__kk = (unsigned char) (cpu->opcode & 0x00FF);
	___k = (unsigned char) (cpu->opcode & 0x000F);
	switch(k___){
		case 0:
			if(__kk == 0x00E0)
				cls(cpu);
			else if(__kk == 0x00EE)
				ret(cpu);
			else
				cpu->error = E_WHATOP;
			break;
		case 1:
			jp_addr(cpu);
			break;
		case 2:
			call_addr(cpu);
			break;
		case 3:
			se_vx_byte(cpu);
			break;
		case 4:
			sne_vx_byte(cpu);
			break;
		case 5:
			se_vx_vy(cpu);
			break;
		case 6:
			ld_vx_byte(cpu);
			break;
		case 7:
			add_vx_byte(cpu);
			break;
		// this is mux'd
		case 8:
			switch(___k){
				case 0:
					ld_vx_vy(cpu);
					break;
				case 1:
					or_vx_vy(cpu);
					break;
				case 2:
					and_vx_vy(cpu);
					break;
				case 3:
					xor_vx_vy(cpu);
					break;
				case 4:
					add_vx_vy(cpu);
					break;
				case 5:
					sub_vx_vy(cpu);
					break;
				case 6:
					shr_vx(cpu);
					break;
				case 7:
					subn_vx_vy(cpu);
					break;
				case 0xE:
					shl_vx(cpu);
					break;
				default:
					cpu->error = E_WHATOP;
			}
			break;
		case 9:
			sne_vx(cpu);
			break;
		case 0xA:
			ld_i_addr(cpu);
			break;
		case 0xB:
			jp_v0_addr(cpu);
			break;
		case 0xC:
			rnd_vx_byte(cpu);
			break;
		case 0xD:
			drw_vx_vy_nibble(cpu);
			break;
		// mux'd
		case 0xE:
			switch(__kk){
				case 0x9E:
					skp_vx(cpu);
					break;
				case 0xA1:
					sknp_vx(cpu);
					break;
				default:
					cpu->error = E_WHATOP;
			}
			break;
		// mux'd
		case 0xF:
			switch(__kk){
				case 0x07:
					ld_vx_dt(cpu);
					break;
				case 0x0A:
					ld_vx_k(cpu);
					break;
				case 0x15:
					ld_dt_vx(cpu);
					break;
				case 0x18:
					ld_st_vx(cpu);
					break;
				case 0x1E:
					add_i_vx(cpu);
					break;
				case 0x29:
					ld_f_vx(cpu);
					break;
				case 0x33:
					ld_b_vx(cpu);
					break;
				case 0x55:
					ld_i_vx(cpu);
					break;
				case 0x65:
					ld_vx_i(cpu);
					break;
				default:
					cpu->error = E_WHATOP;
			}
			break;
		default:
			cpu->error = E_WHATOP;
	}
}

static inline void do_sound_timer(struct chip8 *cpu){
}

static inline void update_timers(struct chip8 *cpu){
	if(cpu->delay_timer)
		cpu->delay_timer--;
	if(cpu->sound_timer){
		if(cpu->sound_timer == 1)
			do_sound_timer(cpu);
		cpu->sound_timer--;
	}
}

inline void clear_graphics(struct chip8* cpu){
	int i, j;
	for(i = 0; i < SCREEN_LENGTH; ++i)
		for(j = 0; j < SCREEN_WIDTH; ++j)
			cpu->gfx[i][j] = 0;
}

inline void clear_registers(struct chip8 *cpu){
	memset(cpu->V, 0, REGISTERS_SIZE);
}

inline void load_fontset(struct chip8 *cpu){
	memcpy(cpu->memory, font_set, FONT_SET_SIZE);
}

inline void clear_keys(struct chip8 *cpu){
	memset(cpu->keys, false, NUM_KEYS);
}

inline void get_keys(struct chip8 *cpu){
	int c;
	c = getch();
	clear_keys(cpu);
	/*
	123C
	456D
	789E
	A0BF
	*/
	switch(c){
		case 'Q':
			cpu->error = E_END;
			break;
		case '1':
			cpu->keys[0x1] = true;
			break;
		case '2':
			cpu->keys[0x2] = true;
			break;
		case '3':
			cpu->keys[0x3] = true;
			break;
		case 'q':
			cpu->keys[0x4] = true;
			break;
		case 'w':
			cpu->keys[0x5] = true;
			break;
		case 'e':
			cpu->keys[0x6] = true;
			break;
		case 'a':
			cpu->keys[0x7] = true;
			break;
		case 's':
			cpu->keys[0x8] = true;
			break;
		case 'd':
			cpu->keys[0x9] = true;
			break;
		case 'z':
			cpu->keys[0xA] = true;
			break;
		case 'x':
			cpu->keys[0x0] = true;
			break;
		case 'c':
			cpu->keys[0xB] = true;
			break;
		case 4:
			cpu->keys[0xC] = true;
			break;
		case 'r':
			cpu->keys[0xD] = true;
			break;
		case 'f':
			cpu->keys[0xE] = true;
			break;
		case 'v':
			cpu->keys[0xF] = true;
			break;
	}
}

void init_chip(struct chip8 *cpu){
	srand(time(NULL));	// random number generation
	(*cpu) = (struct chip8){0};	// should init things to 0
	cpu->pc				= PC_START;
	cpu->opcode			= 0;
	cpu->I				= 0;
	cpu->sp				= 0;
	cpu->delay_timer		= 0;
	cpu->sound_timer		= 0;
	cpu->should_draw  		= false;
	cpu->rand_seeded		= true;
	cpu->error			= 0;
	load_fontset(cpu);
	clear_registers(cpu);
	clear_graphics(cpu);
}

void do_cycle(struct chip8 *cpu){
	fetch_opcode(cpu);
	execute_opcode(cpu);
	update_timers(cpu);
}
