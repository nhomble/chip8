#define DEFAULT_ROM "./roms/TETRIS"
#define DRAW_DELAY 10
#define KEY_TIMEOUT 30

WINDOW* create_win(int height, int width);
void draw(WINDOW *win, struct chip8 *cpu);
void init_screen();
void  destroy_window(WINDOW *win);
