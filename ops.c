#include <stdio.h>
#include <stdlib.h>

#include "ops.h"
#include "chip8.h"

unsigned char static inline LSB(unsigned char reg){
	return reg & 0x01;
}

unsigned char static inline MSB(unsigned char reg){
	return (reg >> 7) & 0x1;
}

unsigned short static inline NNN(struct chip8 *cpu){
	return cpu->opcode & 0x0FFF;
}

unsigned char static inline KK(struct chip8 *cpu){
	return cpu->opcode & 0x00FF;
}

unsigned char static inline N(struct chip8 *cpu){
	return cpu->opcode & 0x000F;
}

void static inline pc_step(struct chip8 *cpu){
	cpu->pc += 2;	
}

void static inline stack_pop(struct chip8 *cpu){
	cpu->sp--;
	cpu->pc = cpu->stack[cpu->sp & 0xF];
}

void static inline stack_push(struct chip8 *cpu){
	cpu->stack[cpu->sp & 0xF] = cpu->pc;
	cpu->sp++;
}

// clear the display
void cls(struct chip8 *cpu){
	clear_graphics(cpu);
	cpu->should_draw = true;
	pc_step(cpu);
}

// return from a subroutine
void ret(struct chip8 *cpu){
	stack_pop(cpu);
	pc_step(cpu);
}

// jump to a machine code routine at NNN
// ignored by modern interpreters!
void sys_addr(struct chip8 *cpu){
	cpu->pc = NNN(cpu);
}

// jump to location NNN
void jp_addr(struct chip8 *cpu){
	cpu->pc = NNN(cpu);
}

// call subroutine at nnn
void call_addr(struct chip8 *cpu){
	stack_push(cpu);
	cpu->pc = NNN(cpu);
}

// skip next instruction if Vx == kk
void se_vx_byte(struct chip8 *cpu){
	pc_step(cpu);
	if(Vx(cpu) == KK(cpu))
		pc_step(cpu);
}

// skip next instruction if Vx != kk
void sne_vx_byte(struct chip8 *cpu){
	pc_step(cpu);
	if(Vx(cpu) != KK(cpu))
		pc_step(cpu);
}

// skip next instruction if Vx == Vy
void se_vx_vy(struct chip8 *cpu){
	pc_step(cpu);
	if(Vx(cpu) == Vy(cpu))
		pc_step(cpu);
}

// load kk into Vx
void ld_vx_byte(struct chip8 *cpu){
	Vx(cpu) = KK(cpu);
	pc_step(cpu);
}

// add kk to vx
void add_vx_byte(struct chip8 *cpu){
	Vx(cpu) += KK(cpu);
	pc_step(cpu);
}

// store vy in vx
void ld_vx_vy(struct chip8 *cpu){
	Vx(cpu) = Vy(cpu);
	pc_step(cpu);
}

// Vx = Vx | Vy
void or_vx_vy(struct chip8 *cpu){
	Vx(cpu) |= Vy(cpu);
	pc_step(cpu);
}

// Vx = Vx & Vy
void and_vx_vy(struct chip8 *cpu){
	Vx(cpu) &= Vy(cpu);
	pc_step(cpu);
}

// vx ^= vy
void xor_vx_vy(struct chip8 *cpu){
	Vx(cpu) ^= Vy(cpu);
	pc_step(cpu);
}

// Vx += Vy, Vf for carry
void add_vx_vy(struct chip8 *cpu){
	unsigned int tmp;
	tmp = Vx(cpu) + Vy(cpu);
	Vf(cpu) = tmp > REGISTER_MAX_VAL;
	Vx(cpu) = tmp & 0xFF;
	pc_step(cpu);
}

// Vx -= Vy
void sub_vx_vy(struct chip8 *cpu){
	Vf(cpu) = Vx(cpu) >= Vy(cpu);
	Vx(cpu) -= Vy(cpu);
	pc_step(cpu);
}

// Vx = shift logical bit right
void shr_vx(struct chip8 *cpu){
	Vf(cpu) = LSB(Vx(cpu));
	Vx(cpu) >>= 1;
	pc_step(cpu);
}

// Vx = Vy - Vx
void subn_vx_vy(struct chip8 *cpu){
	Vf(cpu) = Vx(cpu) > Vy(cpu);
	Vx(cpu) = Vy(cpu) - Vx(cpu);
	pc_step(cpu);
}

// shift left vx
void shl_vx(struct chip8 *cpu){
	Vf(cpu) = MSB(Vx(cpu));
	Vx(cpu) <<= 1;
	pc_step(cpu);
}

// skip next instructino if Vx != Vy
void sne_vx(struct chip8 *cpu){
	pc_step(cpu);
	if(Vx(cpu) != Vy(cpu))
		pc_step(cpu);
}

// set I to nnn
void ld_i_addr(struct chip8 *cpu){
	I(cpu) = NNN(cpu);
	pc_step(cpu);
}

// jump to location nnn + V0
void jp_v0_addr(struct chip8 *cpu){
	cpu->pc = V0(cpu) + NNN(cpu);
}

// set vx to a random byte and'd with kk
// expect srand to be called at init
void rnd_vx_byte(struct chip8 *cpu){
	Vx(cpu) = rand() & KK(cpu);
	pc_step(cpu);
}

// display n-byte sprite starting at memory location I at (Vx, Vy), set Vf = collision
void drw_vx_vy_nibble(struct chip8 *cpu){
	int i, j, x, y;
	unsigned char pixel;
	
	Vf(cpu) = false; // reset
	for(i = 0; i < N(cpu); ++i){
		pixel = cpu->memory[I(cpu) + i];
		// j is just a bitmask to iterate of the byte from memory
		for(j = 0; j < 8; j++){
			if(pixel & (0x80 >> j)){
				x = (Vx(cpu) + j) % SCREEN_LENGTH;
				y = (Vy(cpu) + i) % SCREEN_WIDTH;
				// any collisions
				if(cpu->gfx[x][y])
					Vf(cpu) = true;
				cpu->gfx[x][y] ^= true;
			}
		}
	}
	cpu->should_draw = true;
	pc_step(cpu);
}

// skip next instruction if key(Vx) is pressed
void skp_vx(struct chip8 *cpu){
	pc_step(cpu);
	get_keys(cpu);
	if(cpu->keys[Vx(cpu)])
		pc_step(cpu);
}

// skip next instruction if key(Vx) is not pressed
void sknp_vx(struct chip8 *cpu){
	pc_step(cpu);
	get_keys(cpu);
	if(!(cpu->keys[Vx(cpu)]))
		pc_step(cpu);
}

// set Vx to delay timer
void ld_vx_dt(struct chip8 *cpu){
	Vx(cpu) = cpu->delay_timer;
	pc_step(cpu);
}

// wait for a key press - so don't touch pc until we get what we want
// store key press in vx
void ld_vx_k(struct chip8 *cpu){
	int i;
	get_keys(cpu);
	for(i = 0; i < NUM_KEYS; ++i){
		if(cpu->keys[i]){
			Vx(cpu) = i;
			pc_step(cpu);
		}
	}
}

// set delay timer to vx
void ld_dt_vx(struct chip8 *cpu){
	cpu->delay_timer = Vx(cpu);
	pc_step(cpu);
}

// set sound timer
void ld_st_vx(struct chip8 *cpu){
	cpu->sound_timer = Vx(cpu);
	pc_step(cpu);
}

// add to I, Vx
void add_i_vx(struct chip8 *cpu){
	// undocumented mentioned in reddit/wiki
	Vf(cpu) = (I(cpu) + Vx(cpu)) > 0xFFFF;
	I(cpu) += Vx(cpu);
	pc_step(cpu);
}

// set I for location of sprite for digit  Vx
void ld_f_vx(struct chip8 *cpu){
	I(cpu) = Vx(cpu) * SPRITE_HEIGHT;
	pc_step(cpu);
}

// store bdc representation of Vx in I, I+1, I+2
// shamelessly stolen from arnsa
void ld_b_vx(struct chip8 *cpu){
	cpu->memory[I(cpu)] = Vx(cpu) / 100;
	cpu->memory[I(cpu) + 1] = (Vx(cpu) / 10) % 10;
	cpu->memory[I(cpu) + 2] = Vx(cpu) % 10;
	pc_step(cpu);
}

// store registers 0 to x in memory starting at I
void ld_i_vx(struct chip8 *cpu){
	unsigned int i;
	for(i = 0; i <= Vx(cpu); ++i)
		cpu->memory[I(cpu) + i] = cpu->V[i];
	pc_step(cpu);
}

// read registers 0 to x in memory starting at I
void ld_vx_i(struct chip8 *cpu){
	unsigned int i;
	for(i = 0; i <= Vx(cpu); ++i)
		cpu->V[i] = cpu->memory[I(cpu) + i];
	pc_step(cpu);
}
