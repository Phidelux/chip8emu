#include "chip8emu.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <random>
#include <limits>

Chip8Emu::Chip8Emu()
        : mMem(4096, 0), mReg(16, 0), mGfx(2048, 0),
          mStk(16, 0), mKey(16, 0)
{
  rnd = std::bind(
      std::uniform_int_distribution<std::uint16_t>{0, std::numeric_limits<std::uint8_t>::max()},
      std::mt19937(std::random_device{}()));
}

void Chip8Emu::init()
{
  // Initialize the registers and memory.
  mPc = 0x200;
  mOp = 0;
  mI = 0;
  mSp = 0;

  // Clear display.
  std::fill(mGfx.begin(), mGfx.end(), 0);

  // Clear stack.
  std::fill(mStk.begin(), mStk.end(), 0);

  // Clear registers V0-VF.
  std::fill(mReg.begin(), mReg.end(), 0);

  // Clear memory.
  std::fill(mMem.begin(), mMem.end(), 0);

  // Clear keypad state.
  std::fill(mKey.begin(), mKey.end(), 0);

  // Load fontset into memory.
  std::copy(mFontset.begin(), mFontset.end(), mMem.begin());

  // Initialize the opcode matrix.
  mOpcodes = {
    // Call RCA 1802 programm at NNN
    { 0x0000, [this](){ } },
    // Clear screen
    { 0x00E0, [this](){ } },
    // Return from subroutine
    { 0x00EE, [this](){ mSp--; mPc = mStk[mSp]; mPc += 2; } },
    // Jump to addr NNN
    { 0x1000, [this](){ mPc = (mOp & 0x0FFF); } },
    // Call subroutine at nn
    { 0x2000, [this](){ mStk[mSp] = mPc; mSp++; mPc = (mOp & 0x0FFF); } },
    // Skip next instruction if VX equals NN
    { 0x3000, [this](){ mReg[(mOp & 0x0F00) >> 8] == (mOp & 0x00FF) ? mPc += 4 : mPc += 2; } },
    // Skip next instruction if VX doesn't equal NN
    { 0x4000, [this](){ mReg[(mOp & 0x0F00) >> 8] != (mOp & 0x00FF) ? mPc += 4 : mPc += 2; } },
    // Skip the next instruction of VX equals VY
    { 0x5000, [this](){ mReg[(mOp & 0x0F00) >> 8] == mReg[(mOp & 0x00F0) >> 4] ? mPc += 4 : mPc += 2; } },
    // Set VX to NN
    { 0x6000, [this](){ mReg[(mOp & 0x0F00) >> 8] = (mOp & 0x00FF); mPc += 2; } },
    // Add NN to VX
    { 0x7000, [this](){ mReg[(mOp & 0x0F00) >> 8] += (mOp & 0x00FF); mPc += 2; } },
    // Set VX to value of VY
    { 0x8000, [this](){ mReg[(mOp & 0x0F00) >> 8] = mReg[(mOp & 0x00F0) >> 4]; mPc += 2; } },
    // Set VX to VX or VY
    { 0x8001, [this](){ mReg[(mOp & 0x0F00) >> 8] |= mReg[(mOp & 0x00F0) >> 4]; mPc += 2; } },
    // Set VX to VX and VY
    { 0x8002, [this](){ mReg[(mOp & 0x0F00) >> 8] &= mReg[(mOp & 0x00F0) >> 4]; mPc += 2; } },
    // Set VX to VX xor VY
    { 0x8003, [this](){ mReg[(mOp & 0x0F00) >> 8] ^= mReg[(mOp & 0x00F0) >> 4]; mPc += 2; } },
    // Add VY to VX and set VF to 1 if there is a carry, 0 otherwise
    { 0x8004, [this](){ mReg[0xF] = mReg[(mOp & 0x00F0) >> 4] > (0xFF - mReg[(mOp & 0x0F00) >> 8]) ? 1 : 0; mReg[(mOp & 0x0F00) >> 8] += mReg[(mOp & 0x00F0) >> 4]; mPc += 2; } },
    // Substract VY from VX and set VF to 1 if there is a borrow, 0 otherwise
    { 0x8005, [this](){ mReg[0xF] = mReg[(mOp & 0x00F0) >> 4] > (0xFF - mReg[(mOp & 0x0F00) >> 8]) ? 1 : 0; mReg[(mOp & 0x0F00) >> 8] -= mReg[(mOp & 0x00F0) >> 4]; mPc += 2; } },
    // Shift VX right by one, set VF to the least significant bit of VX before
    { 0x8006, [this](){ mReg[0xF] = mReg[(mOp & 0x0F00) >> 8] & 1; mReg[(mOp & 0x0F00) >> 8] >>= 1; mPc += 2; } },
    // Set VX to VY minus VX, set VF to 1 if there is a barrow, 0 otherwise
    { 0x8007, [this](){ mReg[0xF] = mReg[(mOp & 0x00F0) >> 4] > (0xFF - mReg[(mOp & 0x0F00) >> 8]) ? 1 : 0; mReg[(mOp & 0x0F00) >> 8] = mReg[(mOp & 0x00F0) >> 4] - mReg[(mOp & 0x0F00) >> 8]; mPc += 2; } },
    // Shift VX left by one, set VF to the most significant bit.
    { 0x800E, [this](){ mReg[0xF] = mReg[(mOp & 0x0F00) >> 8] >> 7; mReg[(mOp & 0x0F00) >> 8] <<= 1; mPc += 2; } },
    // Skip the next instruction if VX doesn't equal VY
    { 0x9000, [this](){ mReg[(mOp & 0x0F00) >> 8] != mReg[(mOp & 0x00F0) >> 4] ? mPc += 4 : mPc += 2; } },
    // Set the index register to address NNN
    { 0xA000, [this](){ mI = mOp & 0x0FFF; mPc += 2;} },
    // Jump to the address NNN plus V0
    { 0xB000, [this](){ mPc = (mOp & 0x0FFF) + mReg[0]; } },
    // Set VX to a bitweis and operation of a randam number and NN
    { 0xC000, [this](){ mReg[(mOp & 0x0F00) >> 8] = rnd() & (mOp & 0x00FF); mPc += 2; } },
    { 0xD000, [this](){ } },
    { 0xE09E, [this](){ } },
    { 0xE0A1, [this](){ } },
    { 0xF007, [this](){ } },
    { 0xF00A, [this](){ } },
    { 0xF015, [this](){ } },
    { 0xF018, [this](){ } },
    { 0xF01E, [this](){ } },
    { 0xF029, [this](){ } },
    { 0xF033, [this](){ } },
    { 0xF055, [this](){ } },
    { 0xF065, [this](){ } },
  };

  // Reset the timers.
}

void Chip8Emu::cycle()
{
  // Fetch the opcode, ...
  mOp = (mMem[mPc] << 8) | mMem[mPc +1];

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

    rom.read((char *)&mMem[mI+0x200], static_cast<std::size_t>(size));

    rom.close();
  }
}
