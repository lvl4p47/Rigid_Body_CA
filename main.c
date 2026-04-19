#include "init.h"


int main(int argc, char* args[])
{   
    freopen("debug.log", "w", stderr);
    srand(clock());
    state = (uint32_t)time(NULL);
    
    All_Init();

    while (!quit) {
        freopen("debug.log", "w", stderr);
        
        Global_Time_Update();
        
        Events_Handle();
        
        Cells_Update();
        
        // Genomes_Print();

        if(display_mode == TROPHS
        || display_mode == GENOMES
        || display_mode == ACC
        // || display_mode == PHERO
        || global_time % 100 == 0)
        {
            Screen_Clear();
            Screen_Draw();
            SDL_Delay(1);
        }
    }
    
    All_Quit();

    return 0;
}
