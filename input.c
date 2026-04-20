#include "input.h"

int quit = 0;
int lmb_held = 0;
int rmb_held = 0;
SDL_Event e;

void Events_Handle()
{
    
    while (SDL_PollEvent(&e) != 0) 
    {
        
        if (e.type == SDL_QUIT) 
        {
            quit = 1;
        }
        
        if (e.type == SDL_KEYDOWN) 
        {
            if(e.key.keysym.sym == SDLK_q)
            {
                display_mode = mod(display_mode - 1, DISP_MODE_COUNT);
            }
            if(e.key.keysym.sym == SDLK_w)
            {
                display_mode = mod(display_mode + 1, DISP_MODE_COUNT);
            }
            if(e.key.keysym.sym == SDLK_r)
            {
                Grid_Reset(0, 1000);
            }
            if(e.key.keysym.sym == SDLK_p)
            {
                Populate(100);
            }
            if(e.key.keysym.sym == SDLK_ESCAPE)
            {
                SDL_MinimizeWindow(window);
            }
        }
        
        if (e.type == SDL_MOUSEBUTTONDOWN) 
        {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            
            int x = mouseX / CELL_SIZE;
            int y = mouseY / CELL_SIZE;
            
            if (e.button.button == SDL_BUTTON_RIGHT) 
            {
                rmb_held = 1;
                
                Cell_Create(x, y, 0, rand() % 2, 0);
                
            }
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                lmb_held = 1;
                
                // uint8_t dir = 1;
                // int dx = dir_to_coords[dir][0];
                // int dy = dir_to_coords[dir][1];
                // Tile *neighbor = Grid_Get(x + dx, y + dy);
                // Tile *itself = Grid_Get(x, y);
                // Cell *cell = &cells[itself->id];
                // uint8_t mask;
                
                // if(neighbor->id != 0)
                // {
                //     mask = (uint8_t)1 << dir;
                //     itself->links |= mask;
                    
                //     mask = (uint8_t)1 << mod(dir + 4, 8);
                //     neighbor->links |= mask;
                
                //     mask = (uint8_t)1 << dir;
                //     cell->outlet |= mask;
                // }
                
                Rec_Connect(x, y, 3);
            }
            if (e.button.button == SDL_BUTTON_MIDDLE) 
            {
                Rec_Push(x, y, 1, 0, 2, 0);
            }
        }
        if (e.type == SDL_MOUSEBUTTONUP) {
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                lmb_held = 0;
            }
            if (e.button.button == SDL_BUTTON_RIGHT) 
            {
                rmb_held = 0;
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            
            int x = mouseX / CELL_SIZE;
            int y = mouseY / CELL_SIZE;

            if (rmb_held == 1)
            {
                int rad = grid_height / 8;
                int amount = rad / 16;
                int perc = 2 * rad;
                int dx, dy;
                for(int n = 0; n < amount; n++)
                {
                    dx = rand() % (2 * rad) - rad;
                    dy = rand() % (2 * rad) - rad;
                    
                    if((rand() % perc + rand() % perc) / 2 > abs(dx) + abs(dy)
                    && Grid_Get(x + dx, y + dy)->id == 0)
                        Grid_Set(x + dx, y + dy, 0, 1);
                }
                
                
            }
            if (lmb_held == 1)
            {
                if(Find_Free_Genome_Id() != 0)
                {
                    int rad = grid_height / 8;
                    int amount = rad / 16;
                    int perc = 2 * rad;
                    
                    int dx, dy;
                    for(int n = 0; n < amount; n++)
                    {
                        dx = rand() % (2 * rad) - rad;
                        dy = rand() % (2 * rad) - rad;
                        
                        if((rand() % perc + rand() % perc) / 2 > abs(dx) + abs(dy))
                            Cell_Create(x + dx, y + dy, 0, rand() % 2, 0);
                    }
                }
                
                // Phero_Set(x, y, 0, 255);
                
                // Grid_Set(x, y, 0, 1);
            }
        }
    }
}
