#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "chip8.h"
#include "ui.h"

static void usage(char *prog){
	printf("usage: %s [path to rom]\n", prog);
}

void end(int status){
	if(status){
		fprintf(stdout,	"emulator ended with code %d\n", status);
	}
	exit(status);
}
int main(int argc, char **argv){
	int times;
	struct chip8 cpu;
	WINDOW *win;

	init_chip(&cpu);

	init_screen();
	win = create_win(SCREEN_WIDTH, SCREEN_LENGTH);

	if(argc != 2){
		usage(argv[0]);
		load_rom(&cpu, DEFAULT_ROM);		
	}
	else {
		load_rom(&cpu, argv[1]);
	}

	for(;;){
		get_keys(&cpu);
		for(times = 0; times < DRAW_DELAY; ++times){
			if(cpu.error){
				destroy_window(win);
				fprintf(stdout, "last opcode %04X\n", cpu.opcode);
				end(cpu.error);
			}
			do_cycle(&cpu);
		}

		if(cpu.should_draw){
			draw(win, &cpu);
			cpu.should_draw = false;
		}
	}

	return 0;
}
