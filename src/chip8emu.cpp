#include "chip8emu.h"

#include <algorithm>
#include <iostream>
#include <fstream>

Chip8Emu::Chip8Emu()
        : mMemory(4096, 0), mRegisters(16, 0), mGfx(2048, 0),
          mStack(16, 0), mKeys(16, 0)
{}

void Chip8Emu::init()
{
  // Initialize the registers and memory.
  mPc = 0x200;
  mOpcode = 0;
  mI = 0;
  mSp = 0;

  // Clear display.
  std::fill(mGfx.begin(), mGfx.end(), 0);

  // Clear stack.
  std::fill(mStack.begin(), mStack.end(), 0);

  // Clear registers V0-VF.
  std::fill(mRegisters.begin(), mRegisters.end(), 0);

  // Clear memory.
  std::fill(mMemory.begin(), mMemory.end(), 0);

  // Clear keypad state.
  std::fill(mKeys.begin(), mKeys.end(), 0);

  // Load fontset into memory.
  std::copy(mFontset.begin(), mFontset.end(), mMemory.begin());

  // Initialize the opcode matrix.
  mOpcodes.emplace(0xA000, [this](){
    this->mI = this->mOpcode & 0x0FFF;
    this->mPc += 2;
  });

  // Reset the timers.
}

void Chip8Emu::cycle()
{
  // Fetch the opcode, ...
  mOpcode = (mMemory[mPc] << 8) | mMemory[mPc +1];

  // ... decode ...

  // ... and execute the opcode.

  // Update the timers.
} 

void Chip8Emu::loadRom(const std::string &filename)
{
  std::ifstream rom;
  rom.open(filename, std::ios::in | std::ios::binary);

  if(rom.is_open()) {
    rom.seekg(SEEK_END);
    std::streampos size = rom.tellg();
    rom.seekg(SEEK_SET);

    rom.read((char *)&mMemory[mI+0x200], static_cast<std::size_t>(size));

    rom.close();
  }
}
