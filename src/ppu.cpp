#include "ppu.h"

chip8emu::PPU::PPU()
      : mGfx(2048, 0)
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
}