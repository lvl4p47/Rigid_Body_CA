#ifndef GRAPHICS_H
#define GRAPHICS_H

#include "life.h"

// #define WINDOW_WIDTH 1920
// #define WINDOW_HEIGHT 1080
// #define CELL_SIZE 1

// #define WINDOW_WIDTH 1366 // for small resolution screen
// #define WINDOW_HEIGHT 768
// #define CELL_SIZE 1

#define WINDOW_WIDTH 1920 // for debugging
#define WINDOW_HEIGHT 720
#define CELL_SIZE 1

typedef enum
{
    ENERGY,
    TROPHS,
    GENOMES,
    ACC,
    PHERO,
    DISP_MODE_COUNT
} disp_modes;

extern SDL_Window* window;
extern SDL_Renderer* renderer;
extern uint8_t display_mode;

void Graphics_Init();
void Graphics_Quit();
void Screen_Clear();
void Screen_Draw();
void Grid_Draw();

#endif