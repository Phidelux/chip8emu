#include <iostream>
#include <memory>

#include "chip8emu.h"

const int FPS = 180;
const int DELAY_TIME = 1000.0f / FPS;

int main(int argc, char **argv)
{
   std::cout << "Starting chip8 emulator ..." << std::endl;

   if(argc > 1) {
      std::cout << "Initializing Picture Processing Unit (PPU) ..." << std::endl;
      std::shared_ptr<chip8emu::PPU> ppu = std::make_shared<chip8emu::PPU>(64, 32);
      
      std::cout << "Initializing Keypad ..." << std::endl;
      std::shared_ptr<chip8emu::Keyboard> keyboard = std::make_shared<chip8emu::Keyboard>();
      
      std::cout << "Initializing Central Processing Unit (CPU) ..." << std::endl;
      std::unique_ptr<chip8emu::CPU> cpu = std::make_unique<chip8emu::CPU>(ppu, keyboard);
      
      std::cout << "Initializing Emulator ..." << std::endl;
      chip8emu::Chip8Emu chip8(std::move(cpu), ppu, keyboard);
      
      std::cout << "Initializing memory ..." << std::endl;
      chip8.init();

      std::cout << "Loading rom '" << argv[1] << "' ..." << std::endl;
      chip8.loadRom(argv[1]);
      
      int frameStart, frameTime;

      while(chip8.running()) {
         frameStart = SDL_GetTicks();
         
         chip8.handleEvents();
         chip8.cycle();
         chip8.render();

         frameTime = SDL_GetTicks() - frameStart;

         while(frameTime < DELAY_TIME) {
            chip8.handleEvents();
            frameTime = SDL_GetTicks() - frameStart;
         }
      }
      
      chip8.clean();
   }

   return 0;
}