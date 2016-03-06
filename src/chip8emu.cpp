#include "chip8emu.h"

#include <algorithm>
#include <iostream>
#include <memory>

chip8emu::Chip8Emu::Chip8Emu(std::unique_ptr<chip8emu::CPU> cpu, std::shared_ptr<chip8emu::PPU> ppu, std::shared_ptr<chip8emu::Keyboard> keyboard)
   : mCpu(std::move(cpu)), mGfx(ppu), mKeyboard(keyboard), mPixelRects(new SDL_Rect[ppu->width()* ppu->height()])
{
   
}

bool chip8emu::Chip8Emu::init()
{
   //keyboard->setQuitHandler([this](){ this->quit(); });
   
   // Define the pixel tile size.
   mScale = 10;

   // Initialize SDL.
   if (SDL_Init(SDL_INIT_EVERYTHING) == 0) {
      // Create the main window on success.
      mWindow = std::shared_ptr<SDL_Window>(
                   SDL_CreateWindow("Chip8 Emulator by Phidelux",
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, mGfx->width() * mScale, mGfx->height() * mScale, 0),
      [](SDL_Window* w) {
         SDL_DestroyWindow(w);
      });

      // Create the renderer if the window creation succeeded.
      if (mWindow != nullptr) {
         mRenderer = std::shared_ptr<SDL_Renderer>(
                        SDL_CreateRenderer(mWindow.get(), -1, 0),
         [](SDL_Renderer* r) {
            SDL_DestroyRenderer(r);
         });

         if (mRenderer != nullptr) {
            // Setup the pixel rectangle dimensions.
            const std::uint16_t screenSize = mGfx->width() * mGfx->height(); 
            for (std::uint16_t i = 0; i < screenSize; i++) {
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

   std::cout << "Clear screen ..." << std::endl;
   mGfx->clear();

   return true;
}

void chip8emu::Chip8Emu::cycle()
{
   mCpu->cycle();
}

void chip8emu::Chip8Emu::render()
{
   if(mGfx->isDrawFlagSet()) {
      std::uint16_t pixelsOn = 0;
      for (int y = 0; y < mGfx->height(); y++) {
         for (int x = 0; x < mGfx->width(); x++) {
            if ((*mGfx)[y * mGfx->width() + x]) {
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

      mGfx->resetDrawFlag();
   }
}

void chip8emu::Chip8Emu::handleEvents()
{
   if(mKeyboard->isKeyDown(SDL_SCANCODE_ESCAPE)) {
      mRunning = false;
   }
   
   mKeyboard->update();
}

void chip8emu::Chip8Emu::clean()
{
   //SDL_DestroyWindow(mWindow.get());
   //SDL_DestroyRenderer(mRenderer.get());
   SDL_Quit();
}

void chip8emu::Chip8Emu::loadRom(const std::string &filename)
{
   mCpu->loadRom(filename);
}

bool chip8emu::Chip8Emu::running()
{
   return mRunning;
}

void chip8emu::Chip8Emu::quit()
{
   mRunning = false;
}
