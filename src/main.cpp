#include <iostream>
#include <memory>

#include "chip8emu.h"

const int FPS = 60;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char **argv)
{
   std::cout << "Starting chip8 emulator ..." << std::endl;

   if(argc > 1) {
      chip8emu::Chip8Emu chip8(std::make_unique<chip8emu::PPU>());
      
      std::cout << "Initializing memory ..." << std::endl;
      chip8.init();

      std::cout << "Loading rom '" << argv[1] << "' ..." << std::endl;
      chip8.loadRom(argv[1]);

      while(chip8.running()) {
         chip8.cycle();
         chip8.render();
         //chip8.debugGfx();
         
         SDL_Delay(DELAY_TIME);
      }
   }

   return 0;
}
