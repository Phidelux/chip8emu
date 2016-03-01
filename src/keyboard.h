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
   const std::vector<SDL_Keycode> mKeyMap {
      SDLK_x, SDLK_1, SDLK_2, SDLK_3, 
      SDLK_q, SDLK_w, SDLK_e, SDLK_a, 
      SDLK_s, SDLK_d, SDLK_z, SDLK_c, 
      SDLK_4, SDLK_r, SDLK_f, SDLK_v
   };
};

}

#endif // KEYBOARD_H
