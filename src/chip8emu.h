#ifndef CHIP_EIGHT_EMU_H
#define CHIP_EIGHT_EMU_H

#include <string>
#include <vector>

#define MEM_SIZE 4096
#define REG_SIZE 16

#define SCR_H 32
#define SCR_W 64
#define SCR_SIZE 2048

#define STK_SIZE 16

#define KEY_SIZE 16

class Chip8Emu {
public:
  void init();
  void cycle();

  void loadRom(const std::string &filename);
private:
  std::uint16_t mOpcode; // the current opcode
  std::vector<std::uint8_t> mMemory; // 4k of memory
  std::vector<std::uint8_t> mRegisters; // 15 8-bit registers + carry flag

  std::uint16_t mI; // Index register
  std::uint16_t mPc; // Instruction pointer

  std::vector<std::uint8_t> mGfx; // Display of 64x32 px

  std::uint8_t mDelayTimer; // Delay timer at 60Hz
  std::uint8_t mSoundTimer; // Sound timer at 60Hz

  std::vector<std::uint16_t> mStack; // Jump stack
  std::uint16_t mSp; // The stack pointer

  std::vector<std::uint8_t> mKeys; // Current keypad state

  bool mDrawFlag; // Drawing flag
};

#endif // CHIP_EIGHT_EMU_H
