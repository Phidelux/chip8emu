#include "keyboard.h"

#include <algorithm>

chip8emu::Keyboard::Keyboard()
   : mWindowClosed(false), mKeyPad(16, false)
{
}

chip8emu::Keyboard::~Keyboard()
{
}

void chip8emu::Keyboard::update()
{
   SDL_Event event;

   while (SDL_PollEvent(&event)) {
      switch (event.type) {
      case SDL_QUIT:
         // TODO: Handle quit here.
         mWindowClosed = true;
         break;

      case SDL_KEYDOWN:
         {         
            std::vector<SDL_Keycode>::const_iterator it = std::find(mPadMap.begin(), mPadMap.end(), event.key.keysym.sym);
            if(it != mPadMap.end()) {
               mKeyPad[std::distance(mPadMap.begin(), it)] = true;
            }
            
            this->onKeyDown();
         }
         
         break;

      case SDL_KEYUP:
         this->onKeyUp();
         break;

      default:
         break;
      }
   }
}

void chip8emu::Keyboard::onKeyDown()
{
   mKeystates = SDL_GetKeyboardState(0);
}

void chip8emu::Keyboard::onKeyUp()
{
   mKeystates = SDL_GetKeyboardState(0);
}

bool chip8emu::Keyboard::isPadKeyDown(std::uint8_t key)
{
   if(key < mKeyPad.size() && mKeyPad[key]) {
      mKeyPad[key] = false;
      return true;
   }

   return false;
}

bool chip8emu::Keyboard::isKeyDown(SDL_Scancode key) const
{
   if(mWindowClosed && key == SDL_SCANCODE_ESCAPE) {
      return true;
   }
   
   if (mKeystates != 0) {
      if (mKeystates[key] == 1) {
         return true;
      }
   }

   return false;
}