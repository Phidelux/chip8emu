#ifndef CHIP_EIGHT_EMU_H
#define CHIP_EIGHT_EMU_H

#include "cpu.h"
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

#define STK_SIZE 16

#define KEY_SIZE 16

class SDL_Window;
class SDL_Renderer;

namespace chip8emu
{

class Chip8Emu
{
public:
   Chip8Emu(std::unique_ptr<chip8emu::CPU>cpu, std::shared_ptr<chip8emu::PPU> ppu, std::shared_ptr<chip8emu::Keyboard> keyboard);

   bool init();
   void cycle();
   void render();
	void handleEvents();
   
   std::shared_ptr<SDL_Renderer> getRenderer() const;
   std::shared_ptr<SDL_Window> getWindow() const;

   void loadRom(const std::string &filename);
   
   bool speedTrottled();
   bool fullscreen();
   bool running();
   
   void clean();
   void quit();
   
private:
   bool mRunning;
   bool mFullscreen;
   bool mSpeedTrottled;
   std::uint8_t mScale;

   std::unique_ptr<CPU> mCpu;
   std::shared_ptr<PPU> mGfx; // Display of 64x32 px
   std::shared_ptr<Keyboard> mKeyboard; // Current keypad state
   
   std::shared_ptr<SDL_Window> mWindow;
   std::shared_ptr<SDL_Renderer> mRenderer;
   SDL_Rect *mPixelRects;
};

}

#endif // CHIP_EIGHT_EMU_H