#include "init.h"

uint64_t prev_tick, cur_tick;
uint16_t cycles = 0, cps = 0, threshold = 1000;

int main(int argc, char* args[])
{   
    freopen("debug.log", "w", stderr);
    srand(clock());
    state = (uint32_t)time(NULL);
    
    All_Init();
    
    prev_tick = SDL_GetTicks64();

    while (!quit) {
        freopen("debug.log", "w", stderr);
        
        cycles++;
        
        // printf("cps %4d\n", cps);
        
        Global_Time_Update();
        
        Cells_Update();
        
        Events_Handle();
        
        if(pause) continue;

        Events_Process();

        if(display_mode == TROPHS
        || display_mode == GENOMES
        || display_mode == ACC
        || display_mode == DEBUG
        // || display_mode == ENERGY
        || timer == 0)
        {
            Screen_Clear();
            Screen_Draw();
            // SDL_Delay(1);
        }
        
        cur_tick = SDL_GetTicks64();
        
        if(cur_tick - prev_tick > threshold)
        {
            
            cps = cycles * 1000 / threshold;
            cycles = 0;
            prev_tick = cur_tick;
        }
    }
    
    All_Quit();

    return 0;
}
