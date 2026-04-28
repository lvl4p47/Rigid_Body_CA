#include "init.h"

void All_Init()
{
    Grid_Init(WINDOW_WIDTH / CELL_SIZE, WINDOW_HEIGHT / CELL_SIZE);
    Genomes_Init();
    Cells_Init();
    Graphics_Init();
}

void All_Quit()
{
    Graphics_Quit();
    Life_Reset(1000);
    Grid_Quit();
}
