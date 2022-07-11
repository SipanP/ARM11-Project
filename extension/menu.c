#include "menu.h"

void addBackground() {
  SDL_Surface *image = SDL_LoadBMP("Tetrisbackground.bmp");

  // Places image in memory close to the graphics card
  SDL_Texture *texture = SDL_CreateTextureFromSurface(render, image);
  // Display it in the window
  SDL_RenderCopy(render, texture, NULL, NULL);

  // Clearing variables 
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(image);
}

// draws menu
void draw_menu(int difficulty) {
  // Initialising colours
  SDL_Color color;
  SDL_Color White = {255, 255, 255};
  SDL_Color Grey = {169, 169, 169};

  SDL_Surface *surface;
  SDL_Texture *texture;
  
  // Adds background
  addBackground();

  // Setting and adding "Easy mode" option to menu
  if (difficulty == 5) {
    color = White;
  } else {
    color = Grey;
  }
  surface = TTF_RenderText_Blended(gFont, "Easy Mode", color);
  texture = SDL_CreateTextureFromSurface(render, surface);
  SDL_Rect easy_rect = {(WINDOW_WIDTH / 2) - 200, 250, 100, 40};
  SDL_RenderCopy(render, texture, NULL, &easy_rect);

  // Setting and adding "Hard mode" option to menu
  if (difficulty == 1) {
    color = White;
  } else {
    color = Grey;
  }
  surface = TTF_RenderText_Blended(gFont, "Hard Mode", color);
  texture = SDL_CreateTextureFromSurface(render, surface);
  SDL_Rect hard_rect = {(WINDOW_WIDTH / 2) - 200, 350, 100, 40};
  SDL_RenderCopy(render, texture, NULL, &hard_rect);

  // freeing
  SDL_DestroyTexture(texture);
  SDL_FreeSurface(surface);

  // Set rendering clear color
  // This sets the 'background color'
  // SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
  

  // Update the screen
  setRenderChanged();

  updateRender();
}



void getMenuInput(int *difficulty, int *selected) {
  SDL_Event event;
  while (SDL_PollEvent(&event)) switch (event.type) {
    // Terminate program if 'X' has been clicked
      case SDL_QUIT:
        exit(0);
        break;
      // If a key has been pressed 
      case SDL_KEYDOWN:
        switch (event.key.keysym.scancode) {
          // Terminate program if 'ESC' has been pressed
          case SDL_SCANCODE_ESCAPE:
            exit(0);
            break;
          // Cycle option up if W or Up pressed
          case SDL_SCANCODE_W:
          case SDL_SCANCODE_UP:
            *difficulty = 5;
            draw_menu(*difficulty);
            break;
          // Cycle option down if S or Down pressed
          case SDL_SCANCODE_S:
          case SDL_SCANCODE_DOWN:
            *difficulty = 1;
            draw_menu(*difficulty);
            break;
          // Select game mode with space / return
          case SDL_SCANCODE_SPACE:
          case SDL_SCANCODE_RETURN:
            *selected = 1;
            break;
          default:
            break;
        }
        break;
    }
}
