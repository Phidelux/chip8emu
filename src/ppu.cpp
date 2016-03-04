#include "ppu.h"

#include <iostream>

chip8emu::PPU::PPU(const std::uint8_t width, const std::uint8_t height)
      : mWidth(width), mHeight(height), mGfx(width*height, 0)
{
}

chip8emu::PPU::~PPU()
{
}

std::uint8_t& chip8emu::PPU::operator[](std::size_t idx)
{ 
   mDrawFlag = true;
   return mGfx[idx]; 
}

const std::uint8_t& chip8emu::PPU::operator[](std::size_t idx) const 
{
   return mGfx[idx]; 
}

void chip8emu::PPU::clear()
{
   // Clear all pixel.
   std::fill(mGfx.begin(), mGfx.end(), 0);
   mDrawFlag = true;
}
   
bool chip8emu::PPU::isDrawFlagSet()
{
   return mDrawFlag;
}

void chip8emu::PPU::resetDrawFlag()
{
   mDrawFlag = false;
}

const std::uint8_t chip8emu::PPU::width()
{
   return mWidth;
}

const std::uint8_t chip8emu::PPU::height()
{
   return mHeight;
}

void chip8emu::PPU::debugGfx()
{
   std::cout << "\033[2J\033[1;1H";
   for(std::uint8_t y = 0; y < mHeight; ++y) {
      for(std::uint8_t x = 0; x < mWidth; ++x) {
         if (mGfx[y * mWidth + x]) {
            std::cout << " ";
         } else {
            std::cout << "#";
         }
      }

      std::cout << std::endl;
   }
}