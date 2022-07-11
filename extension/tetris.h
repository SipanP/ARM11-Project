#include "defs.h"
#include "graphics.h"
#include "shuffle.h"

#ifndef MAIN_H
#define MAIN_H
#include "main.h"
#endif

#ifndef _TETRIS_CONSTANTS
#define _TETRIS_CONSTANTS

typedef struct {
  // an array of rotation schemes of a tetromino.
  // each rotation scheme is represented as 16 bits which form 4x4 matrix.
  // row-major order convention is used to interpret this matrix.
  uint16_t rotation[4];

  // RGBA convention: 0xAABBGGRR
  uint32_t color;

} Tetromino;

typedef struct {
  Tetromino type;

  // expected values from 0 to 3 which are the indices of Tetromino.rotation
  uint8_t rotation;

  // its x and y coordinates (which should be between (0,0) <= (x,y) <= (22, 10))
  uint8_t x;
  uint8_t y;

} Tetromino_Movement;

// define movements that a piece can make
typedef enum {
  NONE,
  DOWN,
  LEFT,
  RIGHT,
  DROP,
  ROTATE,
  // normal movement down
  AUTO_DROP,
  // when you press 'r'
  RESTART
} Tetris_Action;

// define colours that a block can take
typedef enum {
  EMPTY = 0xFF000000, // black
  WHITE = 0xFFFFFFFF

} Color_Block;

// default tetris action
// defines the action to apply to current tetromino
extern Tetris_Action TETROMINO_ACTION;

// initialising tetromino data with its rotations (upright, 90 CW rotation, 180 rotation, 90 ACW rotation)
const static Tetromino TETRA_I = {{0x0F00, 0x2222, 0x00F0, 0x4444}, WHITE};

const static Tetromino TETRA_J = {{0x8E00, 0x6440, 0x0E20, 0x44C0}, WHITE};

const static Tetromino TETRA_L = {{0x2E00, 0x4460, 0x0E80, 0xC440}, WHITE};

const static Tetromino TETRA_O = {{0x6600, 0x6600, 0x6600, 0x6600}, WHITE};

const static Tetromino TETRA_S = {{0x6C00, 0x4620, 0x06C0, 0x8c40}, WHITE};

const static Tetromino TETRA_T = {{0x4E00, 0x4640, 0x0E40, 0x4C40}, WHITE};

const static Tetromino TETRA_Z = {{0xC600, 0x2640, 0x0C60, 0x4C80}, WHITE};

// simple array to store coords of blocks rendered on playing field.
// Each tetromino has 4 blocks with total of 4 coordinates.

// {x1, y1, x2, y2, x3, y3, x4, y4}

// initialise all points to 0
static uint8_t CURRENT_TETROMINO_COORDS[8] = {0};
// the position of the piece where it will fall
static uint8_t GHOST_TETROMINO_COORDS[8] = {0};

// the tetromino currently falling that will have be identified by the:
// piece, rotation and its current coordinates
static Tetromino_Movement CURRENT_TETROMINO;

// bool array of the playfield.
// Use row-major order convention to access (x,y) coord.
static Color_Block playfield[PLAYFIELD_HEIGHT * PLAYFIELD_WIDTH];

// The current tetromino will drop by one block every time the AUTO DROP event is triggered. 
// The tetromino stops in place if the drop is unsuccessful an equivalent number of times 
// as the lock delay threshold.
const static uint8_t lock_delay_threshold = 3;
static uint8_t lock_delay_count = 0;

// Queue to determine the next tetromino.
// Fisher-Yates shuffle algorithm is applied.
static uint8_t tetromino_queue[7 * 4]; // 7 block types * 4 rotations
static uint8_t tetromino_queue_size = 7 * 4;
static uint8_t current_queue_index = 0;

static SDL_TimerID cb_timer = 0;

static int score = 0;

#endif

void draw_playing_field();
Color_Block get_playfield(uint8_t x, uint8_t y);
void set_playfield(uint8_t x, uint8_t y, Color_Block color);

void initTetris(int *selected, int *firstLoop, float *speedMultiply, int *difficulty);
void updateTetris(int *difficulty, float *speedMultiply, int *selected,
                  int *firstLoop);
void lockTetromino(int *difficulty, float *speedMultiply, int *selected,
                   int *firstLoop);

void spawn_tetromino(int *selected, int *firstLoop, int *difficulty);
bool render_tetromino(Tetromino_Movement tetra_request,
                      uint8_t current_coords[]);
bool render_current_tetromino(Tetromino_Movement tetra_request);