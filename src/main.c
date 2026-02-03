#include <string.h>
#include <stdio.h>
#include <SDL.h>
#include "../includes/cpu.h"
#include "../includes/display.h"
#include "../includes/controller.h"
#include "../includes/file_browser.h"

int debug = 0;
int nosync = 0;

int main(int argc, char **argv) {


    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--nosync") == 0)
            nosync = 1;
        else if (strcmp(argv[i], "--debug") == 0)
            debug = 1;
    }

    Chip8 chip;
    chip8_init(&chip);

    char *file_path = select_file();
    if (!file_path) {
        fprintf(stderr, "❌ No file select.\n");
        return 1;
    }

    if (chip8_load_program(&chip, file_path) != 0) {
        free(file_path);
        return 1;
    }
    free(file_path);

    createWindow();
    controller_init();
    printf("🚀 Emulation started.\n");

    const int cycles_per_frame = 10; // opcode per frame
    const int frame_delay = 1000 / 60; // 60 FPS

    while (!controller_quit()) {
        uint32_t start_ticks = SDL_GetTicks();

        controller_update(&chip);

        for (int i = 0; i < cycles_per_frame; i++) {
            chip8_emulation_cycle(&chip);
        }

        chip8_update_timers(&chip);
        draw_screen(&chip);

        if (!nosync) {
            uint32_t end_ticks = SDL_GetTicks();
            uint32_t duration = end_ticks - start_ticks;
            if (duration < frame_delay) {
                SDL_Delay(frame_delay - duration);
            }
        }
    }

    destroyWindow();
    return 0;
}
