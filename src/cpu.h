#ifndef CPU_H
#define CPU_H

#include "ppu.h"
#include "keyboard.h"

#include <map>
#include <stack>
#include <vector>
#include <memory>
#include <random>
#include <limits>
#include <functional>

namespace chip8emu
{

class CPU
{
public:
   CPU(std::shared_ptr<PPU> ppu, std::shared_ptr<Keyboard> keyboard);
   ~CPU();
   
   void cycle();

   void loadRom(const std::string &filename);
   void loadState(const std::string &filename);
   void saveState(const std::string &filename) const;

   void debugRegisters();
   void debugMemory();
   
private:
   std::shared_ptr<PPU> mGfx; // Display of 64x32 px
   std::shared_ptr<Keyboard> mKeyboard; // Current keypad state
   
   std::uint16_t mOp; // the current opcode
   std::vector<std::uint8_t> mMem; // 4k of memory
   std::vector<std::uint8_t> mReg; // 15 8-bit registers + carry flag

   std::uint16_t mI; // Index register
   std::uint16_t mPc; // Instruction pointer

   std::uint8_t mDelayTimer; // Delay timer at 60Hz
   std::uint8_t mSoundTimer; // Sound timer at 60Hz

   std::vector<std::uint16_t> mStk; // Jump stack

   std::function<std::uint16_t()> rnd;

   std::map<std::uint16_t, std::function<void()>> mOpcodes;
   
   std::vector<std::uint8_t> mFontset {
      0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
      0x20, 0x60, 0x20, 0x20, 0x70, // 1
      0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
      0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
      0x90, 0x90, 0xF0, 0x10, 0x10, // 4
      0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
      0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
      0xF0, 0x10, 0x20, 0x40, 0x40, // 7
      0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
      0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
      0xF0, 0x90, 0xF0, 0x90, 0x90, // A
      0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
      0xF0, 0x80, 0x80, 0x80, 0xF0, // C
      0xE0, 0x90, 0x90, 0x90, 0xE0, // D
      0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
      0xF0, 0x80, 0xF0, 0x80, 0x80  // F
   };
};

}

#endif // CPU_H
