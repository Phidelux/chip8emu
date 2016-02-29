#ifndef PPU_H
#define PPU_H

#include <vector>
#include <cstdint>

namespace chip8emu
{

class PPU
{
public:
   PPU();
   ~PPU();
   
   void clear();
   
   std::uint8_t& operator[](std::size_t idx);
   const std::uint8_t& operator[](std::size_t idx) const;
   
private:
   bool mDrawFlag; // Drawing flag
   std::vector<std::uint8_t> mGfx; // Display of 64x32 px
};

}

#endif // PPU_H
