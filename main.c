#include "init.h"


int main(int argc, char* args[])
{   
    freopen("debug.log", "w", stderr);
    srand(clock());
    
    All_Init();

    while (!quit) {
        Events_Handle();
        
        Cells_Update();
        Grid_Update();
        
        // Genomes_Print();

        Screen_Clear();

        Screen_Draw();
        
        SDL_Delay(0);
    }
    
    All_Quit();

    return 0;
}
