#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ops.h"
#include "chip8.h"

static void test_stack(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0x1234;
	call_addr(&cpu);
	assert(cpu.sp == 1);
	assert(cpu.stack[0] == 0);
	assert(cpu.pc == 0x234);

	cpu.opcode = 0x4567;
	call_addr(&cpu);
	assert(cpu.sp == 2);
	assert(cpu.stack[1] == 0x234);
	assert(cpu.pc == 0x567);

	ret(&cpu);
	assert(cpu.sp == 1);
	assert(cpu.pc == 0x236);

	ret(&cpu);
	assert(cpu.sp == 0);
	assert(cpu.pc == 2);
}

static void test_clc(void){
	struct chip8 cpu = {0};
	cpu.should_draw = 0;
	cpu.gfx[0][0] = 1;
	cls(&cpu);
	assert(cpu.gfx[0][0] == 0);
}

static void test_ret(void){
	struct chip8 cpu = {0};
	cpu.sp = 1;
	cpu.stack[0] = 11;  // ret pops stack[sp-1]
	ret(&cpu);
	assert(cpu.sp == 0);
	assert(cpu.pc == 13);  // 11 + 2 (pc_step)
}

static void test_jp_addr(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0x0123;
	jp_addr(&cpu);
	assert(cpu.opcode == 0x0123);
}

static void test_call_addr(void){
	struct chip8 cpu = {0};
	cpu.pc = 1;
	cpu.opcode = 88;
	call_addr(&cpu);
	assert(cpu.pc == 88);
	assert(cpu.sp == 1);
	assert(cpu.stack[0] == 1);
	ret(&cpu);
	assert(cpu.sp == 0);
	assert(cpu.pc == 1 + 2);
}

static void test_se_vx_byte(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[1] = 0x10;
	cpu.opcode = 0x0111;
	se_vx_byte(&cpu);
	assert(cpu.pc == 2);
	cpu.opcode = 0x0110;
	se_vx_byte(&cpu);
	assert(cpu.pc == 6);
}

static void test_sne_vx_byte(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[1] = 0x10;
	cpu.opcode = 0x0111;
	sne_vx_byte(&cpu);
	assert(cpu.pc == 4);
	cpu.opcode = 0x0110;
	sne_vx_byte(&cpu);
	assert(cpu.pc == 6);
}

static void test_se_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.V[0] = 1;
	cpu.V[1] = 1;
	cpu.pc = 0;
	cpu.opcode = 0x001F;
	se_vx_vy(&cpu);
	assert(cpu.pc == 4);
	cpu.V[1] = 2;
	se_vx_vy(&cpu);
	assert(cpu.pc == 6);
}

static void test_ld_vx_byte(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[0] = 0;
	cpu.opcode = 0x0099;
	ld_vx_byte(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.V[0] == 0x99);
}

static void test_add_vx_byte(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[1] = 1;
	cpu.opcode = 0x0102;
	add_vx_byte(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.V[1] == 1 + 0x02);
}

static void test_ld_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[0] = 2;
	cpu.V[1] = 3;
	cpu.opcode = 0x0010;
	ld_vx_vy(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.V[0] == cpu.V[1]);
}

static void test_or_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[0] = 0x0001;
	cpu.V[1] = 0x0010;
	cpu.opcode = 0x0010;
	or_vx_vy(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.V[0] == 0x0011);
}

static void test_and_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[0] = 0x0011;
	cpu.V[1] = 0x0001;
	cpu.opcode = 0x0010;
	and_vx_vy(&cpu);
	assert(cpu.V[0] == 0x0001);
	assert(cpu.pc == 2);
}

static void test_xor_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[0] = 0x00F0;
	cpu.V[1] = 0x00FF;
	cpu.opcode = 0x0010;
	xor_vx_vy(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.V[0] == 0x000F);
}

static void test_add_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[0] = 0;
	cpu.V[1] = 1;
	cpu.V[0xF] = 0;
	cpu.opcode = 0x0010;
	add_vx_vy(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.V[0] == 1);
	assert(cpu.V[0xF] == 0);
	cpu.V[0] = 0xFF;
	cpu.V[1] = 3;
	add_vx_vy(&cpu);
	assert(cpu.V[0xF] == 1);
	assert(cpu.V[0] == 2);
}

static void test_sub_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0x0010;
	cpu.V[0] = 0;
	cpu.V[1] = 1;
	cpu.V[0xF] = 0;
	sub_vx_vy(&cpu);
	assert(cpu.V[0xF] == 0);
	assert(cpu.V[0] == 255);
	assert(cpu.V[1] == 1);
	assert(cpu.pc == 2);

	cpu.V[0] = 2;
	sub_vx_vy(&cpu);
	assert(cpu.V[0] == 1);
	assert(cpu.V[0xF] == 1);
	assert(cpu.pc == 4);

	// VF as destination: 8F10 means VF -= V1
	cpu.pc = 0;
	cpu.opcode = 0x8F10;
	cpu.V[0xF] = 5;
	cpu.V[1] = 3;
	sub_vx_vy(&cpu);
	assert(cpu.V[0xF] == 1);  // flag wins over result
	assert(cpu.pc == 2);
}

static void test_shr_vx(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0x0000;
	cpu.V[0] = 4;
	shr_vx(&cpu);
	assert(cpu.V[0xF] == 0);
	assert(cpu.V[0] == 2);
	assert(cpu.pc == 2);
	cpu.V[0] = 3;
	shr_vx(&cpu);
	assert(cpu.V[0xF] == 1);
	assert(cpu.V[0] == 1);
	assert(cpu.pc == 4);
}

static void test_subn_vx_vy(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0x0010;
	cpu.V[0] = 0;
	cpu.V[1] = 1;
	cpu.V[0xF] = 0;

	subn_vx_vy(&cpu);
	assert(cpu.V[0] == 1);
	assert(cpu.V[0xF] == 1);  // no borrow: Vy(1) >= Vx(0)
	assert(cpu.pc == 2);

	cpu.V[0] = 1;
	cpu.V[1] = 0;
	subn_vx_vy(&cpu);
	assert(cpu.V[0] == 255);
	assert(cpu.V[0xF] == 0);  // borrow: Vy(0) < Vx(1)
	assert(cpu.pc == 4);
}

static void test_shl_vx(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0x0000;
	cpu.V[0] = 0x81;
	cpu.V[0xF] = 0;
	shl_vx(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.V[0xF] == 1);
	assert(cpu.V[0] == 0x2);
	shl_vx(&cpu);
	assert(cpu.pc == 4);
	assert(cpu.V[0xF] == 0);
	assert(cpu.V[0] == 0x4);
}

static void test_sne_vx(void){
	struct chip8 cpu = {0};
	cpu.opcode = 0x0010;
	cpu.V[0] = 0;
	cpu.V[1] = 0;
	cpu.pc = 0;
	sne_vx(&cpu);
	assert(cpu.pc == 2);
	cpu.V[0] = 1;
	sne_vx(&cpu);
	assert(cpu.pc == 6);
}

static void test_ld_i_addr(void){
	struct chip8 cpu = {0};
	cpu.opcode = 0x0123;
	cpu.pc = 0;
	cpu.I = 0;
	ld_i_addr(&cpu);
	assert(cpu.pc == 2);
	assert(cpu.I == 0x0123);
}

static void test_jp_v0_addr(void){
	struct chip8 cpu = {0};
	cpu.opcode = 0x0123;
	cpu.V[0] = 0x1;
	cpu.pc = 0;
	jp_v0_addr(&cpu);
	assert(cpu.pc == 0x0124);
}

static void test_skp_vx(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0xE09E;
	cpu.V[0] = 5;
	cpu.keys[5] = 1;
	skp_vx(&cpu);
	assert(cpu.pc == 4);  // skipped

	cpu.keys[5] = 0;
	skp_vx(&cpu);
	assert(cpu.pc == 6);  // not skipped
}

static void test_sknp_vx(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0xE0A1;
	cpu.V[0] = 5;
	cpu.keys[5] = 0;
	sknp_vx(&cpu);
	assert(cpu.pc == 4);  // skipped (not pressed)

	cpu.keys[5] = 1;
	sknp_vx(&cpu);
	assert(cpu.pc == 6);  // not skipped (pressed)
}

static void test_ld_vx_k(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0xF00A;

	// no key pressed — pc should not advance
	ld_vx_k(&cpu);
	assert(cpu.pc == 0);

	// one key pressed
	cpu.keys[7] = 1;
	ld_vx_k(&cpu);
	assert(cpu.V[0] == 7);
	assert(cpu.pc == 2);

	// multiple keys pressed — should only step pc once
	cpu.pc = 0;
	cpu.keys[3] = 1;
	cpu.keys[9] = 1;
	ld_vx_k(&cpu);
	assert(cpu.pc == 2);
}

static void test_rnd_vx_byte(void){
	srand(time(NULL));
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.V[0] = 0;
	cpu.opcode = 0x0023;
	rnd_vx_byte(&cpu);
	assert(cpu.pc == 2);
}

static void test_ld_i_vx(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0xF355;
	cpu.I = 0x300;
	cpu.V[0] = 10;
	cpu.V[1] = 20;
	cpu.V[2] = 30;
	cpu.V[3] = 40;
	cpu.V[4] = 99;
	ld_i_vx(&cpu);
	assert(cpu.memory[0x300] == 10);
	assert(cpu.memory[0x301] == 20);
	assert(cpu.memory[0x302] == 30);
	assert(cpu.memory[0x303] == 40);
	assert(cpu.memory[0x304] != 99);
	assert(cpu.I == 0x304);
	assert(cpu.pc == 2);
}

static void test_ld_vx_i(void){
	struct chip8 cpu = {0};
	cpu.pc = 0;
	cpu.opcode = 0xF265;
	cpu.I = 0x300;
	cpu.memory[0x300] = 11;
	cpu.memory[0x301] = 22;
	cpu.memory[0x302] = 33;
	cpu.memory[0x303] = 44;
	ld_vx_i(&cpu);
	assert(cpu.V[0] == 11);
	assert(cpu.V[1] == 22);
	assert(cpu.V[2] == 33);
	assert(cpu.V[3] != 44);
	assert(cpu.I == 0x303);
	assert(cpu.pc == 2);
}

int main(void){
	test_stack();
	test_rnd_vx_byte();
	test_ld_i_vx();
	test_ld_vx_i();
	test_skp_vx();
	test_sknp_vx();
	test_ld_vx_k();
	test_jp_v0_addr();
	test_ld_i_addr();
	test_sne_vx();
	test_shl_vx();
	test_subn_vx_vy();
	test_shr_vx();
	test_sub_vx_vy();
	test_add_vx_vy();
	test_xor_vx_vy();
	test_and_vx_vy();
	test_or_vx_vy();
	test_ld_vx_vy();
	test_add_vx_byte();
	test_ld_vx_byte();
	test_se_vx_vy();
	test_sne_vx_byte();
	test_se_vx_byte();
	test_call_addr();
	test_jp_addr();
	test_ret();
	test_clc();
	printf("done\n");
	return 0;
}
