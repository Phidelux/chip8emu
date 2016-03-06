#include "keyboard.h"

chip8emu::Keyboard::Keyboard()
      : mKeyPad(16, false)
{
}

chip8emu::Keyboard::~Keyboard()
{
}

void chip8emu::Keyboard::setQuitHandler(std::function<void()> quitHandler)
{
   mQuitHandler = quitHandler;
}

void chip8emu::Keyboard::update()
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
         // TODO: Handle quit here.
			mQuitHandler();
         break;

		case SDL_KEYDOWN:
         for (std::uint8_t i = 0; i < 16; i++) {
            if (mPadMap[i] == event.key.keysym.sym) {
               mKeyPad[i] = true;
               break;
            }
         }
         
         break;
         
		default:
			break;
		}
	}
}

bool chip8emu::Keyboard::isKeyDown(std::uint8_t key)
{
   if(key < mKeyPad.size() && mKeyPad[key]) {
      mKeyPad[key] = false;
      return true;
   }
   
   return false;
}
