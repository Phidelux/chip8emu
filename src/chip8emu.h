#ifndef CHIP_EIGHT_EMU_H
#define CHIP_EIGHT_EMU_H

#include "ppu.h"
#include "keyboard.h"

#include "SDL2/SDL.h"

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <functional>

#define MEM_SIZE 4096
#define REG_SIZE 16

#define SCR_H 32
#define SCR_W 64
#define SCR_SIZE 2048

#define STK_SIZE 16

#define KEY_SIZE 16

class SDL_Window;
class SDL_Renderer;

namespace chip8emu
{

class Chip8Emu
{
public:
   Chip8Emu(std::unique_ptr<PPU> ppu, std::unique_ptr<chip8emu::Keyboard> keyboard);

   bool init();
   void cycle();
   void render();
	void handleEvents();
   
   std::shared_ptr<SDL_Renderer> getRenderer() const;
   std::shared_ptr<SDL_Window> getWindow() const;

   void loadRom(const std::string &filename);

   void debugRegisters();
   void debugMemory();
   void debugGfx();
   
   bool running();
   void clean();
   void quit();
   
private:
   std::shared_ptr<SDL_Window> mWindow;
   std::shared_ptr<SDL_Renderer> mRenderer;
   SDL_Rect mPixelRects[SCR_SIZE];

   bool mRunning;
   bool mDrawFlag;
   std::uint8_t mScale; 
   
   std::uint16_t mOp; // the current opcode
   std::vector<std::uint8_t> mMem; // 4k of memory
   std::vector<std::uint8_t> mReg; // 15 8-bit registers + carry flag

   std::uint16_t mI; // Index register
   std::uint16_t mPc; // Instruction pointer

   std::unique_ptr<PPU> mGfx; // Display of 64x32 px
   std::unique_ptr<Keyboard> mKeyboard; // Current keypad state

   std::uint8_t mDelayTimer; // Delay timer at 60Hz
   std::uint8_t mSoundTimer; // Sound timer at 60Hz

   std::vector<std::uint16_t> mStk; // Jump stack
   std::uint16_t mSp; // The stack pointer

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

#endif // CHIP_EIGHT_EMU_H
