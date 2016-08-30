#ifndef OPS_H
#define OPS_H

#define REGISTER_MAX_VAL ((1 << 8) - 1)

#define Vx(C) 	C->V[((C->opcode & 0x0F00) >> 8)]
#define Vy(C) 	C->V[((C->opcode & 0x00F0) >> 4)]
#define V0(C) 	C->V[0]
#define Vf(C) 	C->V[0xF]
#define I(C)  	C->I

struct chip8;	// placate gcc

void cls(struct chip8 *); 		// 00E0
void ret(struct chip8 *);		// 00EE

void sys_addr(struct chip8 *);		// 0NNN
void jp_addr(struct chip8 *);		// 1NNN
void call_addr(struct chip8 *);		// 2NNN
void se_vx_byte(struct chip8 *);	// 3xKK
void sne_vx_byte(struct chip8 *);	// 4xKK
void se_vx_vy(struct chip8 *);		// 5xy0
void ld_vx_byte(struct chip8 *);	// 6xKK
void add_vx_byte(struct chip8 *);	// 7xFF

void ld_vx_vy(struct chip8 *);		// 8xy0
void or_vx_vy(struct chip8 *);		// 8xy1
void and_vx_vy(struct chip8 *);		// 8xy2
void xor_vx_vy(struct chip8 *);		// 8xy3
void add_vx_vy(struct chip8 *);		// 8xy4
void sub_vx_vy(struct chip8 *);		// 8xy5
void shr_vx(struct chip8 *);		// 8xy6
void subn_vx_vy(struct chip8 *);	// 8xy7
void shl_vx(struct chip8 *);		// 8xyE

void sne_vx(struct chip8 *);		// 9xy0
void ld_i_addr(struct chip8 *);		// ANNN
void jp_v0_addr(struct chip8 *);	// BNNN
void rnd_vx_byte(struct chip8 *);	// CxKK
void drw_vx_vy_nibble(struct chip8 *);	// DxyN

void skp_vx(struct chip8 *);		// Ex9E
void sknp_vx(struct chip8 *);		// ExA1

void ld_vx_dt(struct chip8 *);		// Fx07
void ld_vx_k(struct chip8 *); 		// Fx0A
void ld_dt_vx(struct chip8 *);		// Fx15
void ld_st_vx(struct chip8 *);		// Fx18
void add_i_vx(struct chip8 *);		// Fx1E
void ld_f_vx(struct chip8 *);		// Fx29
void ld_b_vx(struct chip8 *);		// Fx33
void ld_i_vx(struct chip8 *);		// Fx55
void ld_vx_i(struct chip8 *);		// Fx65
#endif
