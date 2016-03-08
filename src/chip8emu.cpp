#include "chip8emu.h"

#include <algorithm>
#include <iostream>
#include <fstream>
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
                   SDL_CreateWindow("Chip8 Emulator by Phidelux", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                        mGfx->width() * mScale, mGfx->height() * mScale, 0), SDL_DestroyWindow);

      // Create the renderer if the window creation succeeded.
      if (mWindow != nullptr) {
         mRenderer = std::shared_ptr<SDL_Renderer>(
                        SDL_CreateRenderer(mWindow.get(), -1, 0), SDL_DestroyRenderer);

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
   mFullscreen = false;
   mSpeedTrottled = true;

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
   
   if(mKeyboard->isKeyDown(SDL_SCANCODE_F10)) {
      if(mFullscreen) {
         SDL_SetWindowFullscreen(mWindow.get(), 0);
      } else {
         SDL_SetWindowFullscreen(mWindow.get(), SDL_WINDOW_FULLSCREEN);
      }
      
      mFullscreen = !mFullscreen;
   }
   
   if(mKeyboard->isKeyPressed(SDLK_F8)) {
      saveState();
   }
   
   if(mKeyboard->isKeyPressed(SDLK_F9)) {
      takeSnapshot();
   }

   mSpeedTrottled = !mKeyboard->isKeyDown(SDL_SCANCODE_SPACE);
    
   mKeyboard->update();
}

void chip8emu::Chip8Emu::clean()
{
   SDL_Quit();
}

void chip8emu::Chip8Emu::loadRom(const std::string &filename)
{
   mRomName = filename;

   // Remove directory if present.
   const size_t lastSlashIdx = mRomName.find_last_of("\\/");
   if (std::string::npos != lastSlashIdx) {
      mRomName.erase(0, lastSlashIdx + 1);
   }

   // Remove extension if present.
   const size_t periodIdx = mRomName.rfind('.');
   if (std::string::npos != periodIdx) {
      mRomName.erase(periodIdx);
   }
   
   mCpu->loadRom(filename);
      
   // Fetch the name of the last save state, ...
   std::string stateFile = generateFilename("chip8_", ".bak", true);
   
   // ... check if the file exists.
   if(std::ifstream(stateFile)) {
      std::cout << "Loading safe state from " << stateFile << " ..." << std::endl;
      loadState(stateFile);
   }
}

void chip8emu::Chip8Emu::loadState(const std::string &filename)
{
   mCpu->loadState(filename);
}

void chip8emu::Chip8Emu::saveState()
{
   // Generate the save state filename ...
   std::string filename = generateFilename("chip8_", ".bak");

   // ... and store the current state as binary file.
   mCpu->saveState(filename);
   
   std::cout << "Saved machine state as " << filename << " ..." << std::endl;
}

void chip8emu::Chip8Emu::takeSnapshot()
{
   // Generate the snapshot filename ...
   std::string filename = generateFilename("snap_", ".bmp");

   // ... and store the current screen as bitmap.
   std::shared_ptr<SDL_Surface> sshot = std::shared_ptr<SDL_Surface>(SDL_GetWindowSurface(mWindow.get()), SDL_FreeSurface);
   Uint32 format = SDL_GetWindowPixelFormat(mWindow.get());
   SDL_RenderReadPixels(mRenderer.get(), nullptr, format, sshot->pixels, sshot->pitch);
   SDL_SaveBMP(sshot.get(), filename.c_str());
   
   std::cout << "Saved snapshot as " << filename << " ..." << std::endl;
}

std::string chip8emu::Chip8Emu::generateFilename(const std::string &prefix, const std::string &ext, const bool exists) const
{
   // Fetch the current rom name, ...
   std::string romName = mRomName;
   
   // ... translate to lower case, ...
   std::transform(romName.begin(), romName.end(), romName.begin(), ::tolower);
   
   // ... replace whitespaces with underscores ...
   std::replace( romName.begin(), romName.end(), ' ', '_');
   
   // ... and erase all special chars.
   romName.erase(std::remove_if(romName.begin(), romName.end(), 
         [](char c){ return !std::isalnum(c) && c != '_'; }), romName.end());
   
   std::uint8_t i;
   for(i = 0; std::ifstream(prefix + romName + "_" + std::to_string(i) + ext); i++);
   
   if(exists && i-1 >= 0) {
      return prefix + romName + "_" + std::to_string(i-1) + ext;
   }
   
   return prefix + romName + "_" + std::to_string(i) + ext;
}

bool chip8emu::Chip8Emu::running()
{
   return mRunning;
}

bool chip8emu::Chip8Emu::speedTrottled()
{
   return mSpeedTrottled;
}

void chip8emu::Chip8Emu::quit()
{
   mRunning = false;
}