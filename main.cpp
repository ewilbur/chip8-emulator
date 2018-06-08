#include <SDL_video.h>
#include <SDL_render.h>
#include <SDL.h>
#include <chrono>
#include <thread>
#include "chip8.h"
#include <iostream>

#define windowWidth 1024
#define windowHeight 512

using namespace std;

uint8_t keymap[16] = {
  SDLK_x,
  SDLK_1,
  SDLK_2,
  SDLK_3,
  SDLK_q,
  SDLK_w,
  SDLK_e,
  SDLK_a,
  SDLK_s,
  SDLK_d,
  SDLK_z,
  SDLK_c,
  SDLK_4,
  SDLK_r,
  SDLK_f,
  SDLK_v,
};

int main(int argc, char **argv) {

  Chip8 chip8 = Chip8();

  if (argc == 1) {
    cerr << "Usage: chip8 <ROM file> [flags]" << endl;
    exit(1);
  } else {
    for (int i = 1; i < argc; ++i) {
      switch (argv[i][0]) {
        case '-':
          switch (argv[i][1]) {
            case 'd':
              chip8.setDebugFlag(true);
              break;

            default:
              cerr << "Unrecognized flag: " << argv[i] << endl;
              exit(1);
              break;
          }
          break;
      }
    }
  }
  int w = windowWidth;
  int h = windowHeight;

  // The window we'll be rendering to
  SDL_Window* window = NULL;

  // Initialize SDL
  if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) {
    cerr << "SDL could not initialize: " << SDL_GetError() << endl;
    exit(1);
  }
  // Create window
  window = SDL_CreateWindow(
      "CHIP-8 Emulator",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      w, h, SDL_WINDOW_SHOWN
      );
  if (window == NULL){
    cerr << "Unable to create window: " << SDL_GetError() << endl;
    exit(2);
  }

  // Create renderer
  SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
  SDL_RenderSetLogicalSize(renderer, w, h);

  // Create texture that stores frame buffer
  SDL_Texture* sdlTexture = SDL_CreateTexture(renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_STREAMING,
      64, 32);

  // Temporary pixel buffer
  uint32_t pixels[2048];


load:
  // Attempt to load ROM
  if (!chip8.loadRom(argv[1]))
    return 2;

  // Emulation loop
  while (true) {
    chip8.executeCycle();

    // Process SDL events
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
      if (e.type == SDL_QUIT) exit(0);

      // Process keydown events
      if (e.type == SDL_KEYDOWN) {
        if (e.key.keysym.sym == SDLK_ESCAPE)
          exit(0);

        if (e.key.keysym.sym == SDLK_F1)
          goto load;

        for (int i = 0; i < 16; ++i) {
          if (e.key.keysym.sym == keymap[i]) {
            chip8.setKeypadValue(i,true);
          }
        }
      }
      // Process keyup events
      if (e.type == SDL_KEYUP) {
        for (int i = 0; i < 16; ++i) {
          if (e.key.keysym.sym == keymap[i]) {
            chip8.setKeypadValue(i,false);
          }
        }
      }
    }

    // If draw occurred, redraw SDL screen
    if (chip8.drawFlag) {
      chip8.drawFlag = false;

      // Store pixels in temporary buffer
      for (int i = 0; i < 2048; ++i) {
        uint8_t pixel = chip8.getDisplayValue(i);
        pixels[i] = (0x00FFFFFF * pixel) | 0xFF000000;
      }
      // Update SDL texture
      SDL_UpdateTexture(sdlTexture, NULL, pixels, 64 * sizeof(Uint32));
      // Clear screen and render
      SDL_RenderClear(renderer);
      SDL_RenderCopy(renderer, sdlTexture, NULL, NULL);
      SDL_RenderPresent(renderer);
    }

    // Sleep to slow down emulation speed
    std::this_thread::sleep_for(std::chrono::microseconds(1200));

  }
}
