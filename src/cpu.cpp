#include "cpu.h"

#include <iostream>
#include <iterator>
#include <fstream>

chip8emu::CPU::CPU(std::shared_ptr<PPU> ppu, std::shared_ptr<Keyboard> keyboard)
   : mGfx(ppu), mKeyboard(keyboard), mMem(4096, 0), mReg(16, 0)
{
   rnd = std::bind(
            std::uniform_int_distribution<std::uint16_t> {0, std::numeric_limits<std::uint8_t>::max()},
            std::mt19937(std::random_device {}()));

   // Initialize the registers and memory.
   mPc = 0x200;
   mOp = 0;
   mI = 0;

   // Clear registers V0-VF.
   std::fill(mReg.begin(), mReg.end(), 0);

   // Clear memory.
   std::fill(mMem.begin(), mMem.end(), 0);

   // Load fontset into memory.
   std::copy(mFontset.begin(), mFontset.end(), mMem.begin());

   // Initialize the opcode matrix.
   mOpcodes = {
      // Call RCA 1802 programm at NNN
      { 0x0000, [this]() { } },
      // Clear screen
      { 0x00E0, [this]() {
            mGfx->clear();
            mPc += 2;
         }
      },
      // Return from subroutine
      {
         0x00EE, [this]() {
            mPc = mStk.back();
            mStk.pop_back();
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
            mStk.push_back(mPc);
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
            mReg[0xF] = mReg[(mOp & 0x00F0) >> 4] > mReg[(mOp & 0x0F00) >> 8] ? 0 : 1;
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
            mReg[0xF] = mReg[(mOp & 0x0F00) >> 8] > (mReg[(mOp & 0x00F0) >> 4]) ? 0 : 1;
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
            uint8_t h = (mOp & 0x000F);
            std::uint8_t p;

            mReg[0xF] = 0;

            for (std::uint8_t i = 0; i < h; i++) {
               p = mMem[mI + i];
               for (std::uint8_t j = 0; j < 8; j++) {
                  if ((p & (0x80 >> j)) != 0) {
                     if ((*mGfx)[(x + j + ((y + i) * 64))] != 0) {
                        mReg[0xF] = 1;
                     }

                     (*mGfx)[(x + j + ((y + i) * 64))] ^= 1;
                  }
               }
            }

            mPc += 2;
         }
      },
      // Skip next instruction if key in VX is pressed
      {
         0xE09E, [this]() {
            mKeyboard->isPadKeyDown(mReg[(mOp & 0x0F00) >> 8]) ? mPc += 4 : mPc += 2;
         }
      },
      // Skip next instruction if key in VX is not pressed
      {
         0xE0A1, [this]() {
            !mKeyboard->isPadKeyDown(mReg[(mOp & 0x0F00) >> 8]) ? mPc += 4 : mPc += 2;
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
            for(std::uint8_t i = 0; i < 16; i++) {
               if(mKeyboard->isPadKeyDown(i)) {
                  mReg[(mOp & 0x0F00) >> 8] = i;
                  mPc += 2;
                  break;
               }
            }
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
            // VF is set to 1 when range overflow (I+VX>0xFFF), and 0 when there isn't.
            mReg[0xF] = mI + mReg[(mOp & 0x0F00) >> 8] > 0xFFF ? 1 : 0;
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
            // TODO: Fix copy here.
            std::copy(mReg.begin(), mReg.begin() + ((mOp & 0x0F00) >> 8) + 1, mMem.begin() + mI);
            mPc += 2;
         }
      },
      // Fill V0 to VX with values from memory starting at I
      {
         0xF065, [this]() {
            std::copy(mMem.begin() + mI, mMem.begin() + mI + ((mOp & 0x0F00) >> 8) + 1, mReg.begin());
            mPc += 2;
         }
      }
   };

   // Initialize the timers.
   mDelayTimer = 0;
   mSoundTimer = 0;
}

chip8emu::CPU::~CPU()
{
   
}

void chip8emu::CPU::cycle()
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
      std::cerr << "Error: Invalid opcode 0x" << std::hex << mOp << std::endl;
      mPc += 2;
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

void chip8emu::CPU::loadRom(const std::string &filename)
{
   std::ifstream rom(filename, std::ios::in | std::ios::binary | std::ios::ate);

   if(rom.is_open()) {
      rom.unsetf(std::ios::skipws);
      std::ifstream::pos_type size = rom.tellg();
      rom.seekg(0, std::ios::beg);
      rom.read((char *)&mMem[mI+0x200], static_cast<std::size_t>(size));
      rom.close();
   }
}

void chip8emu::CPU::saveState(const std::string &filename) const
{
   std::ofstream rom(filename, std::ios::out | std::ios::binary);

   if(rom.is_open()) {
      rom.write((char*)&mI, sizeof(mI));
      rom.write((char*)&mPc, sizeof(mPc));
      rom.write((char*)&mOp, sizeof(mOp));
      rom.write((char*)&mDelayTimer, sizeof(mDelayTimer));
      rom.write((char*)&mSoundTimer, sizeof(mSoundTimer));
      std::copy(mReg.begin(), mReg.end(), std::ostream_iterator<std::uint8_t>(rom, ""));
      std::copy(mMem.begin(), mMem.end(), std::ostream_iterator<std::uint8_t>(rom, ""));
      std::copy(mStk.begin(), mStk.end(), std::ostream_iterator<std::uint16_t>(rom, ""));
      
      std::uint16_t numPixels = mGfx->width() * mGfx->height();
      for(std::uint16_t i = 0; i < numPixels; i++) {
         rom.put((*mGfx)[i]);
      }
      
      rom.close();
   }
}

void chip8emu::CPU::debugRegisters()
{
   std::uint16_t counter = 0;
   for(std::vector<std::uint8_t>::iterator it = mReg.begin(); it != mReg.end(); ++it) {
      std::cout << "0x" << std::hex << static_cast<int>(*it) << " ";

      if(counter % 4 == 4 - 1) {
         std::cout << std::endl;
      }

      counter++;
   }
}

void chip8emu::CPU::debugMemory()
{
   std::uint16_t counter = 0;
   for(std::vector<std::uint8_t>::iterator it = mMem.begin(); it != mMem.end(); ++it) {
      std::cout << "0x" << std::hex << static_cast<int>(*it) << " ";

      if(counter % 5 == 5 - 1) {
         std::cout << std::endl;
      }

      counter++;
   }
}
