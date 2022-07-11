#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SDL2/SDL.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2_gfxPrimitives.h"
#include "sds.h"

#ifndef _GLOBAL_CONSTANTS
#define _GLOBAL_CONSTANTS

#define WINDOW_TITLE "Tetris"

// Sets size of each individual block (In terms of pixels)
#define BLOCK_SIZE 30

// Sets width and height of the Tetris grid (In terms of blocks)
#define PLAYFIELD_HEIGHT 22
#define PLAYFIELD_WIDTH 10

// Defines window size based on block size and number of blocks
#define WINDOW_HEIGHT PLAYFIELD_HEIGHT*(BLOCK_SIZE + 1) + 1
#define WINDOW_WIDTH PLAYFIELD_WIDTH*(BLOCK_SIZE + 1) + 1

#endif