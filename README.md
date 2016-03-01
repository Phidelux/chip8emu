     ______ _______ _______ ______ ______ _______ _______ _______
    |      |   |   |_     _|   __ \  __  |    ___|   |   |   |   |
    |   ---|       |_|   |_|    __/  __  |    ___|       |   |   |
    |______|___|___|_______|___|  |______|_______|__|_|__|_______|


  Chip8Emu is a simple chip8 emulator written in C++!

  For hardware documentation, visit:
  https://en.wikipedia.org/wiki/CHIP-8

  Latest version can be found in:
  https://github.com/phidelux/chip8emu/

# Keys & Information:

    Chip8            PC Keys
    ----------------------------
    NUM-PAD 1        Keyboard 1
    NUM-PAD 2        Keyboard 2
    NUM-PAD 3        Keyboard 3
    NUM-PAD 4        Keyboard q
    NUM-PAD 5        Keyboard w
    NUM-PAD 6        Keyboard e
    NUM-PAD 7        Keyboard a
    NUM-PAD 8        Keyboard s
    NUM-PAD 9        Keyboard d
    NUM-PAD 0        Keyboard x
    KEY A            Keyboard y
    KEY B            Keyboard c
    KEY C            Keyboard v
    KEY D            Keyboard f
    KEY E            Keyboard r
    KEY F            Keyboard 4
    ----------------------------

    ESC  Closes the window and exits
    F8   Saves the current gamestate as "chip8_<game>_<number>.bak"
    F9   Saves a screenshot as "snap_<game>_<number>.bmp"
    F10  Toggles between fullscreen and windowed mode.
    TAB  Disables speed throttle when hold.

# Command Line Interface

  Usage:
    chip8emu [Options] rom.min

  Options:
    -nostate               Discard state data (default)
    -state pokemini.bak    Load/Save state file
    -nosound               Disable sound
    -colorlight 0xFFFFFF   Light Color
    -colordark 0x000000    Dark Color
    -windowed              Display in window (default)
    -fullscreen            Display in fullscreen
    -zoom n                Zoom display: 1 to 20 (def 10)
    -logmem                Dumps the memory state
    -loglcd                Logs the pixel buffer state