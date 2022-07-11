#include "defs.h"
#include "graphics.h"
#include "input.h"
#include "menu.h"
#include "tetris.h"

// Global variables (Used in graphics.h)
SDL_Window *window;
SDL_Renderer *render;
SDL_Texture *display;
TTF_Font *gFont;

bool render_changed;

void initialise();

void cleanup();