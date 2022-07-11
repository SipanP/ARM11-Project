#include "main.h"

void initialise() {
  // Start up SDL
  if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
    fprintf(stderr, "\nUnable to initialize SDL:  %s\n", SDL_GetError());
  }

  // Cleans the SDL up on exit
  atexit(cleanup);

  if (TTF_Init() == -1) {
    fprintf(stderr, "\nTTF_Init Error:  %s\n", SDL_GetError());
    exit(1);
  }

  init_graphics();

  // Draws intial menu page.
  draw_menu(5);
}

void cleanup() {
  cleanup_graphics();

  // Shuts down font-use
  TTF_Quit();

  // Shuts down SDL
  SDL_Quit();
}

int main(int argc, const char *argv[]) {
  initialise();

  // Menu variable
  int selected = 0;

  // Gameplay variables
  int difficulty = 5;
  float speedMultiply = 1.0;
  int firstLoop = 0;

  while (true) {
    preRender();

    if (selected == 0) {
      getMenuInput(&difficulty, &selected);
    } else {
      if (firstLoop == 0) {
        firstLoop = 1;
        initTetris(&selected, &firstLoop, &speedMultiply, &difficulty);
      }
      getInput(&selected, &firstLoop, &difficulty);

      if (selected == 1) {
        updateTetris(&difficulty, &speedMultiply, &selected, &firstLoop);
        updateRender();
        // Delay achieves 60 FPS: 1000 ms/ 60 fps = 1/16 s^2/frame
        SDL_Delay(16);
      }
    }
  }

  return 0;
}