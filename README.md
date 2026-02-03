# CunCHIP8

*CunCHIP8* is an lightweight CHIP-8 emulator written in **C**.

*(Yes the name means "It's a CHIP-8" but in french 🇫🇷)*

### Example

| OPCODES Test    |
| ------------- |
| <img src="assets/test_opcode.gif" width="500"> |

---

### Features
- Full CHIP‑8 instruction support
- Keyboard input handling
- SDL2 graphics for 64x32 monochrome display
- Timers
- Debug mode
- Non Sync mode (for more speed)

---

### Installation
1. Clone the repository:
```
git clone https://github.com/squach90/CunCHIP8.git
cd CunCHIP8
```
2. Make sure you have **SDL2** installed:
- Ubuntu / Debian:
    ```sh
    sudo apt install libsdl2-dev
    ```

- macOS (Homebrew):
    ```sh
    brew install sdl2
    ```

3. Build the emulator:
    ```sh
    make
    ```

### Usage

Run a rom :
```sh
./chip8
```
Optional flags:
- `--nosync` → disable 60Hz frame sync
- `--debug` → print opcode info for debugging and key pressed

---

### Controls

```
     Keypad               Keyboard
+ - + - + - + - +    + - + - + - + - +
| 1 | 2 | 3 | C |    | 1 | 2 | 3 | 4 |
+ - + - + - + - +    + - + - + - + - +
| 4 | 5 | 6 | D |    | Q | W | E | R |
+ - + - + - + - + => + - + - + - + - +
| 7 | 8 | 9 | E |    | A | S | D | F |
+ - + - + - + - +    + - + - + - + - +
| A | 0 | B | F |    | Z | X | C | V |
+ - + - + - + - +    + - + - + - + - +
```

## Dependencies

- **[FileBrowser.h](https://github.com/wirenux/choiceMaker.h)**
- **SDL**
- **Standard C** – `stdio.h`, `stdint.h`, `stdbool.h` ,`stdlib.h`, `string.h`, `stddef.h`, `time.h`
- **POSIX / Terminal** – `unistd.h`, `dirent.h`, `termios.h`, `sys/stat.h`, `limits.h`

- **[CHIP-8 Wikipedia](https://en.wikipedia.org/wiki/CHIP-8)**
> Anyone is free to copy, modify, publish, use, compile, sell, or distribute this software, either in source code form or as a compiled binary, for any purpose, commercial or non-commercial, and by any means.
