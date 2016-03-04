#ifndef PPU_H
#define PPU_H

#include <vector>
#include <cstdint>

namespace chip8emu
{

class PPU
{
public:
   PPU(const std::uint8_t width, const std::uint8_t height);
   ~PPU();
   
   void clear();
   
   bool isDrawFlagSet();
   void resetDrawFlag();
   
   const std::uint8_t width();
   const std::uint8_t height();
   
   std::uint8_t& operator[](std::size_t idx);
   const std::uint8_t& operator[](std::size_t idx) const;
   
   void debugGfx();
   
private:
   bool mDrawFlag; // Drawing flag
   
   const std::uint8_t mWidth;
   const std::uint8_t mHeight;
   
   std::vector<std::uint8_t> mGfx; // Display of 64x32 px
};

}

#endif // PPU_H
