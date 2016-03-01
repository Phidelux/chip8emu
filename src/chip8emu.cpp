#include "chip8emu.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <random>
#include <limits>

chip8emu::Chip8Emu::Chip8Emu(std::shared_ptr<chip8emu::PPU> ppu)
   : mMem(4096, 0), mReg(16, 0), mGfx(std::move(ppu)), mStk(16, 0), mKey(16, 0)
{
   rnd = std::bind(
            std::uniform_int_distribution<std::uint16_t> {0, std::numeric_limits<std::uint8_t>::max()},
            std::mt19937(std::random_device {}()));
}

bool chip8emu::Chip8Emu::init()
{
   // Define the pixel tile size.
   mScale = 10;
   
   // Initialize SDL.
   if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
      // Create the main window on success.
      mWindow = std::shared_ptr<SDL_Window>(
         SDL_CreateWindow("Chip8 Emulator by Phidelux", 
         SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCR_W * mScale, SCR_H * mScale, 0),
         [](SDL_Window* w){ SDL_DestroyWindow(w); });

      // Create the renderer if the window creation succeeded.
      if (mWindow != nullptr) {
         mRenderer = std::shared_ptr<SDL_Renderer>(
            SDL_CreateRenderer(mWindow.get(), -1, 0),
            [](SDL_Renderer* r){ SDL_DestroyRenderer(r); });
         
         if (mRenderer != nullptr) {
            // Setup the pixel rectangle dimensions.
            for (std::uint16_t i = 0; i < SCR_SIZE; i++) {
               mPixelRects[i].w = mPixelRects[i].h = mScale;
            }
            
            SDL_ShowCursor(0);
         } else {
            std::cout << "Failed to initialize renderer!" << std::endl;
            return false;
         }
      } else {
         std::cout << "Failed to initialize window!" << std::endl;
         return false;
      }
   } else {
      std::cout << "Failed to initialize SDL!" << std::endl;
      return false;
   }
   
   mRunning = true;
   
   // Initialize the registers and memory.
   mPc = 0x200;
   mOp = 0;
   mI = 0;
   mSp = 0;

   // Clear display.
   // std::fill(mGfx.begin(), mGfx.end(), 0);
   mGfx->clear();

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
      { 0x00E0, [this]() { mGfx->clear(); mPc += 2; } },
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
                     if ((*mGfx)[(x + j + ((y + i) * 64))] == 1) {
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
            std::copy(mReg.begin(), mReg.begin() + ((mOp & 0x0F00) >> 8), &mMem[mI]);

            // On the original interpreter, when the operation is done, I = I + X + 1.
				mI += ((mOp & 0x0F00) >> 8) + 1;
            mPc += 2;
         }
      },
      // Fill V0 to VX with values from memory starting at I
      {
         0xF065, [this]() {
            std::copy(&mMem[mI], &mMem[mI + ((mOp & 0x0F00) >> 8)], mReg.begin());

            // On the original interpreter, when the operation is done, I = I + X + 1.
				mI += ((mOp & 0x0F00) >> 8) + 1;
            mPc += 2;
         }
      }
   };

   // Reset the timers.
   mDelayTimer = 0;
   mSoundTimer = 0;
   
   return true;
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

void chip8emu::Chip8Emu::render()
{
   std::uint16_t pixelsOn = 0;
   for (int y = 0; y < SCR_H; y++) {
      for (int x = 0; x < SCR_W; x++) {
         if ((*mGfx)[y * SCR_W + x]) {
            mPixelRects[pixelsOn].x = mScale * x, mPixelRects[pixelsOn].y = mScale * y;
            pixelsOn++;
         }
      }
   }
  
	// Clear the screen then draw the pixels
   SDL_SetRenderDrawColor(mRenderer.get(), 0x00, 0x00, 0x00, 0xFF);
   SDL_RenderClear(mRenderer.get());
   SDL_SetRenderDrawColor(mRenderer.get(), 0xE0, 0xEE, 0xEE, 0xFF);
   SDL_RenderFillRects(mRenderer.get(), mPixelRects, pixelsOn);

   // Flip the screen and hold
   SDL_RenderPresent(mRenderer.get());
}

void chip8emu::Chip8Emu::loadRom(const std::string &filename)
{
   std::ifstream rom(filename, std::ios::in | std::ios::binary | std::ios::ate);

   if(rom.is_open()) {
      std::cout << "File is open for reading!" << std::endl;
      rom.unsetf(std::ios::skipws);
      std::ifstream::pos_type size = rom.tellg();
      std::cout << "File is " << static_cast<std::size_t>(size) << " bytes long!" << std::endl;
      rom.seekg(0, std::ios::beg);

      rom.read((char *)&mMem[mI+0x200], static_cast<std::size_t>(size));

      rom.close();
   }
   
   debugMemory();
}

void chip8emu::Chip8Emu::debugMemory()
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

void chip8emu::Chip8Emu::debugGfx()
{
   std::cout << "\033[2J\033[1;1H";
	for(std::uint8_t y = 0; y < SCR_H; ++y) {
		for(std::uint8_t x = 0; x < SCR_W; ++x) {
         if ((*mGfx)[y * SCR_W + x]) {
            std::cout << " ";
         } else {
            std::cout << "#";
         }
      }
      
      std::cout << std::endl;
   }
}

bool chip8emu::Chip8Emu::running()
{
   return mRunning;
}

void chip8emu::Chip8Emu::quit()
{
   mRunning = false;
}