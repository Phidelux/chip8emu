#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>

#include <functional>
#include <iostream>
#include <vector>

namespace chip8emu
{

class Keyboard
{
public:
   Keyboard();
   ~Keyboard();

   void update();
   void reset();

   bool isKeyDown(std::uint8_t key);

   void setQuitHandler(std::function<void()> quitHandler);

private:
   std::function<void()> mQuitHandler;

   std::vector<bool> mKeyPad;
   const std::vector<SDL_Keycode> mPadMap {
      SDLK_x, SDLK_1, SDLK_2, SDLK_3,
      SDLK_q, SDLK_w, SDLK_e, SDLK_a,
      SDLK_s, SDLK_d, SDLK_y, SDLK_c,
      SDLK_4, SDLK_r, SDLK_f, SDLK_v
   };

   const std::vector<SDL_Keycode> mEmuMap {
      SDLK_ESCAPE, SDLK_F8, SDLK_F9, SDLK_F10, SDLK_SPACE
   };
};

}

#endif // KEYBOARD_H