#include "init.h"

uint64_t prev_tick, cur_tick;
uint16_t cycles = 0, cps = 0, threshold = 1000;
uint32_t total_cycles = 0;
uint16_t ms_per_fast_frame = 32, ms_per_slow_frame = 320;
uint16_t ms_per_frame;

uint8_t leak_detector = 0;

int main(int argc, char* args[])
{   
    freopen("debug.log", "w", stderr);
    srand(clock());
    state = (uint32_t)time(NULL);
    
    All_Init();
    
    prev_tick = SDL_GetTicks64();

    while (!quit) {
        cycles++;
        
        // printf("cps %4d\n", cps);
        
        Global_Time_Update();
        
        Events_Handle();
        
        if(pause) continue;

        Events_Process();
        
        Cells_Update();
        
        cur_tick = SDL_GetTicks64();
        
        if(slow_mode == 0)
        {
            ms_per_frame = ms_per_fast_frame;
            // printf("fast\n");
        }
        else
        {
            ms_per_frame = ms_per_slow_frame;
            // printf("slow\n");
        }
        if(cur_tick - last_frame > ms_per_frame || leak_detector)
        {
            // printf("dt %d\n", cur_tick - last_frame);
            
            last_frame = cur_tick;
            Screen_Clear();
            Screen_Draw();
            freopen("debug.log", "w", stderr);
            // SDL_Delay(100);
        }
        
        if(cur_tick - prev_tick > threshold || leak_detector)
        {
            
            cps = cycles * 1000 / threshold;
            total_cycles += cycles;
            cycles = 0;
            prev_tick = cur_tick;
            printf("cps %4d cycle no. %d population %5d matter %d energy %7d sunlight %3d\n", cps, total_cycles, population_size, total_matter, total_energy_acc, sun_light);
            // if(leak_detector && total_energy != total_energy_acc) printf("\t\t\t\tenergy leak\n%d\n", 1 / 0);
        }
    }
    
    All_Quit();

    return 0;
}
