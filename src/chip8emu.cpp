#include "chip8emu.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <random>
#include <limits>

chip8emu::Chip8Emu::Chip8Emu()
   : mMem(4096, 0), mReg(16, 0), mStk(16, 0), mKey(16, 0)
{
   rnd = std::bind(
            std::uniform_int_distribution<std::uint16_t> {0, std::numeric_limits<std::uint8_t>::max()},
            std::mt19937(std::random_device {}()));
}

void chip8emu::Chip8Emu::init()
{
   // Initialize the registers and memory.
   mPc = 0x200;
   mOp = 0;
   mI = 0;
   mSp = 0;

   // Clear display.
   // std::fill(mGfx.begin(), mGfx.end(), 0);
   mGfx.clear();

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
      { 0x0000, [this]() { } },
      // Clear screen
      { 0x00E0, [this]() { mGfx.clear(); mPc += 2; } },
      // Return from subroutine
      {
         0x00EE, [this]() {
            mSp--;
            mPc = mStk[mSp];
            mPc += 2;
         }
      },
      // Jump to addr NNN
      {
         0x1000, [this]() {
            mPc = (mOp & 0x0FFF);
         }
      },
      // Call subroutine at nn
      {
         0x2000, [this]() {
            mStk[mSp] = mPc;
            mSp++;
            mPc = (mOp & 0x0FFF);
         }
      },
      // Skip next instruction if VX equals NN
      {
         0x3000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] == (mOp & 0x00FF) ? mPc += 4 : mPc += 2;
         }
      },
      // Skip next instruction if VX doesn't equal NN
      {
         0x4000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] != (mOp & 0x00FF) ? mPc += 4 : mPc += 2;
         }
      },
      // Skip the next instruction of VX equals VY
      {
         0x5000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] == mReg[(mOp & 0x00F0) >> 4] ? mPc += 4 : mPc += 2;
         }
      },
      // Set VX to NN
      {
         0x6000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] = (mOp & 0x00FF);
            mPc += 2;
         }
      },
      // Add NN to VX
      {
         0x7000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] += (mOp & 0x00FF);
            mPc += 2;
         }
      },
      // Set VX to value of VY
      {
         0x8000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] = mReg[(mOp & 0x00F0) >> 4];
            mPc += 2;
         }
      },
      // Set VX to VX or VY
      {
         0x8001, [this]() {
            mReg[(mOp & 0x0F00) >> 8] |= mReg[(mOp & 0x00F0) >> 4];
            mPc += 2;
         }
      },
      // Set VX to VX and VY
      {
         0x8002, [this]() {
            mReg[(mOp & 0x0F00) >> 8] &= mReg[(mOp & 0x00F0) >> 4];
            mPc += 2;
         }
      },
      // Set VX to VX xor VY
      {
         0x8003, [this]() {
            mReg[(mOp & 0x0F00) >> 8] ^= mReg[(mOp & 0x00F0) >> 4];
            mPc += 2;
         }
      },
      // Add VY to VX and set VF to 1 if there is a carry, 0 otherwise
      {
         0x8004, [this]() {
            mReg[0xF] = mReg[(mOp & 0x00F0) >> 4] > (0xFF - mReg[(mOp & 0x0F00) >> 8]) ? 1 : 0;
            mReg[(mOp & 0x0F00) >> 8] += mReg[(mOp & 0x00F0) >> 4];
            mPc += 2;
         }
      },
      // Substract VY from VX and set VF to 1 if there is a borrow, 0 otherwise
      {
         0x8005, [this]() {
            mReg[0xF] = mReg[(mOp & 0x00F0) >> 4] > (0xFF - mReg[(mOp & 0x0F00) >> 8]) ? 1 : 0;
            mReg[(mOp & 0x0F00) >> 8] -= mReg[(mOp & 0x00F0) >> 4];
            mPc += 2;
         }
      },
      // Shift VX right by one, set VF to the least significant bit of VX before
      {
         0x8006, [this]() {
            mReg[0xF] = mReg[(mOp & 0x0F00) >> 8] & 1;
            mReg[(mOp & 0x0F00) >> 8] >>= 1;
            mPc += 2;
         }
      },
      // Set VX to VY minus VX, set VF to 1 if there is a barrow, 0 otherwise
      {
         0x8007, [this]() {
            mReg[0xF] = mReg[(mOp & 0x00F0) >> 4] > (0xFF - mReg[(mOp & 0x0F00) >> 8]) ? 1 : 0;
            mReg[(mOp & 0x0F00) >> 8] = mReg[(mOp & 0x00F0) >> 4] - mReg[(mOp & 0x0F00) >> 8];
            mPc += 2;
         }
      },
      // Shift VX left by one, set VF to the most significant bit.
      {
         0x800E, [this]() {
            mReg[0xF] = mReg[(mOp & 0x0F00) >> 8] >> 7;
            mReg[(mOp & 0x0F00) >> 8] <<= 1;
            mPc += 2;
         }
      },
      // Skip the next instruction if VX doesn't equal VY
      {
         0x9000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] != mReg[(mOp & 0x00F0) >> 4] ? mPc += 4 : mPc += 2;
         }
      },
      // Set the index register to address NNN
      {
         0xA000, [this]() {
            mI = mOp & 0x0FFF;
            mPc += 2;
         }
      },
      // Jump to the address NNN plus V0
      {
         0xB000, [this]() {
            mPc = (mOp & 0x0FFF) + mReg[0];
         }
      },
      // Set VX to a bitweis and operation of a randam number and NN
      {
         0xC000, [this]() {
            mReg[(mOp & 0x0F00) >> 8] = rnd() & (mOp & 0x00FF);
            mPc += 2;
         }
      },
      // Draw a sprite at (VX, VY) that has a width of 8 pixels and a height of N pixels.
      {
         0xD000, [this]() {
            uint8_t x = mReg[(mOp & 0x0F00) >> 8];
            uint8_t y = mReg[(mOp & 0x00F0) >> 4];
            uint8_t h = (mOp & 0x00F0);
            mReg[0xF] = 0;

            for (std::uint8_t i = 0; i < h; i++) {
               std::uint8_t p = mMem[mI + i];
               for (std::uint8_t j = 0; j < 8; j++) {
                  if ((p & (0x80 >> j)) != 0) {
                     if (mGfx[(x + j + ((y + i) * 64))] == 1)
                        mReg[0xF] = 1;
                     mGfx[(x + j + ((y + i) * 64))] ^= 1;
                  }
               }
            }
         }
      },
      // Skip next instruction if key in VX is pressed
      {
         0xE09E, [this]() {
            mKey[mReg[(mOp & 0x0F00) >> 8]] ? mPc += 4 : mPc += 2;
         }
      },
      // Skip next instruction if key in VX is not pressed
      {
         0xE0A1, [this]() {
            !mKey[mReg[(mOp & 0x0F00) >> 8]] ? mPc += 4 : mPc += 2;
         }
      },
      // Set VX to value of delay timer
      {
         0xF007, [this]() {
            mReg[(mOp & 0x0F00) >> 8] = mDelayTimer;
            mPc += 2;
         }
      },
      // TODO: Store the next keypress in VX
      { 
         0xF00A, [this]() {
            mPc += 2;
         } 
      },
      // Set delay timer to VX
      {
         0xF015, [this]() {
            mDelayTimer = mReg[(mOp & 0x0F00) >> 8];
            mPc += 2;
         }
      },
      // Set sound timer to VX
      {
         0xF018, [this]() {
            mSoundTimer = mReg[(mOp & 0x0F00) >> 8];
            mPc += 2;
         }
      },
      // Add VX to I
      {
         0xF01E, [this]() {
            mI += mReg[(mOp & 0x0F00) >> 8];
            mPc += 2;
         }
      },
      // Set I to the location of the sprite for the character in VX.
      // Characters 0-F (in hexadecimal) are represented by a 4x5 font.
      {
         0xF029, [this]() {
            mI = mReg[(mOp & 0x0F00) >> 8] * 0x5;
            mPc += 2;
         }
      },
      // Store the binary-coded decimal representation of VX in memory at I.
      {
         0xF033, [this]() {
            mMem[mI] = mReg[(mOp & 0x0F00) >> 8] / 100;
            mMem[mI + 1] = (mReg[(mOp & 0x0F00) >> 8] / 10) % 10;
            mMem[mI + 2] = (mReg[(mOp & 0x0F00) >> 8] % 100) % 10;
            mPc += 2;
         }
      },
      // Store V0 to VX in memory starting at I
      {
         0xF055, [this]() {
            std::copy(mReg.begin(), mReg.begin() + ((mOp & 0x0F00) >> 8), &mMem[mI]);
            mPc += 2;
         }
      },
      // Fill V0 to VX with values from memory starting at I
      {
         0xF065, [this]() {
            std::copy(&mMem[mI], &mMem[mI + ((mOp & 0x0F00) >> 8)], mReg.begin());
            mPc += 2;
         }
      }
   };

   // Reset the timers.
}

void chip8emu::Chip8Emu::cycle()
{
   // Fetch the opcode, ...
   mOp = (mMem[mPc] << 8) | mMem[mPc +1];

   // ... decode ...
   std::map<std::uint16_t, std::function<void()>>::iterator it = mOpcodes.end();
   if(mOpcodes.count(mOp & 0xF0FF)) {
      it = mOpcodes.find(mOp & 0xF0FF);
   } else if(mOpcodes.count(mOp & 0xF00F)) {
      it = mOpcodes.find(mOp & 0xF00F);
   } else if(mOpcodes.count(mOp & 0xF000)) {
      it = mOpcodes.find(mOp & 0xF000);
   } else {
      std::cerr << "Error: Invalid opcode " << std::hex << mOp << std::endl;
   }

   // ... and execute the opcode.   
   if(it != mOpcodes.end()) {
      (it->second)();
   }

   // Update the timers.
   if (mDelayTimer > 0) {
      mDelayTimer--;
   }
   
   if (mSoundTimer > 0) {
      if (mSoundTimer == 1) {
         // TODO: Play sound with SDL lib.
         printf("BEEP!\n");
      }
      
      mSoundTimer--;
   }
}

void chip8emu::Chip8Emu::loadRom(const std::string &filename)
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
