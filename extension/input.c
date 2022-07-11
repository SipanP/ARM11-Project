#include "input.h"

void getInput(int *selected, int *firstLoop, int *difficulty) {
  SDL_Event event;

  // Goes through each buffered event
  while (SDL_PollEvent(&event)) {
    switch (event.type) {

      // Exits program when window closed
      case SDL_QUIT:
        exit(0);
        break;

      case SDL_KEYDOWN:
        switch (event.key.keysym.sym) {
          // Exits program when escape pressed
          case SDLK_ESCAPE:
            exit(0);
            break;
          
          // Moves down when S or DOWN pressed
          case SDLK_s:
          case SDLK_DOWN:
            TETROMINO_ACTION = DOWN;
            break;
            
          // Moves right when D or RIGHT pressed  
          case SDLK_d:
          case SDLK_RIGHT:
            TETROMINO_ACTION = RIGHT;
            break;

          // Moves left when A or LEFT pressed  
          case SDLK_a:
          case SDLK_LEFT:
            TETROMINO_ACTION = LEFT;
            break;

          // Moves up when W or UP pressed 
          case SDLK_w:
          case SDLK_UP:
            TETROMINO_ACTION = ROTATE;
            break;

          // Restarts the game when R pressed   
          case SDLK_r:
            TETROMINO_ACTION = RESTART;
            break;

          // Returns to menu when Q pressed    
          case SDLK_q:
            *selected = 0;
            cleanup();
            initialise();
            *firstLoop = 0;
            *difficulty = 5;
            break;

          // Fast-Drops Tetromino when SPACE pressed  
          case SDLK_SPACE:
            TETROMINO_ACTION = DROP;
            break;
        }
        break;
      
      // Proceeds with normal dropping otherwise
      case SDL_USEREVENT:
        TETROMINO_ACTION = AUTO_DROP;
        break;
    }
  }
}