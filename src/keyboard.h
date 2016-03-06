#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <SDL2/SDL.h>

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

   bool isPadKeyDown(std::uint8_t key);
   bool isKeyDown(SDL_Scancode key) const;

   void onKeyDown();
   void onKeyUp();

private:
   bool mWindowClosed;
   const Uint8* mKeystates;
    
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