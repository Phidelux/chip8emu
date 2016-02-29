#include <iostream>

#include "chip8emu.h"

int main(int argc, char **argv)
{
   std::cout << "Starting chip8 emulator ..." << std::endl;

   if(argc > 1) {
      std::cout << "Rom: " << argv[1] << std::endl;
   }
   
   chip8emu::Chip8Emu chip8;
   chip8.init();
   chip8.loadRom("example.c8");

   return 0;
}
