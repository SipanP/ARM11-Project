#include "defs.h"
#include "graphics.h"

// Global variables (Used in graphics.h)
SDL_Window *window;
SDL_Renderer *render;
SDL_Texture *display;
TTF_Font *gFont;

void draw_menu(int difficulty);

void getMenuInput(int *difficulty, int *selected);
