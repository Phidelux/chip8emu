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
    NUM-PAD 4        Keyboard Q
    NUM-PAD 5        Keyboard W
    NUM-PAD 6        Keyboard E
    NUM-PAD 7        Keyboard A
    NUM-PAD 8        Keyboard S
    NUM-PAD 9        Keyboard D
    NUM-PAD 0        Keyboard X
    KEY A            Keyboard Y
    KEY B            Keyboard C
    KEY C            Keyboard V
    KEY D            Keyboard F
    KEY E            Keyboard R
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
    -state tetris.bak      Load/Save state file
    -nosound               Disable sound
    -colorlight 0xFFFFFF   Light Color
    -colordark 0x000000    Dark Color
    -windowed              Display in window (default)
    -fullscreen            Display in fullscreen
    -zoom n                Zoom display: 1 to 20 (def 10)
    -logmem                Dumps the memory state
    -loglcd                Logs the pixel buffer state

# Dependencies

 - SDL2
 - SDL2_image
 - SDL2_mixer

# Apache License v2.0

    Copyright 2016 Phidelux <info@bornageek.com>

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.