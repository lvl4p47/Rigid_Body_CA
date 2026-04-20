#include "input.h"

int quit = 0;
int lmb_held = 0;
int rmb_held = 0;
int mmb_held = 0;
SDL_Event e;

int grab_x, grab_y, dest_x, dest_y, str;

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
                
                Rec_Connect(x, y, 100);
            }
            if (e.button.button == SDL_BUTTON_MIDDLE) 
            {
                mmb_held = 1;
                
                grab_x = x;
                grab_y = y;
                dest_x = x;
                dest_y = y;
                
                // Rec_Push(x, y, 0, -1, 1, 0);
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
            if (e.button.button == SDL_BUTTON_MIDDLE) 
            {
                mmb_held = 0;
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            int mouseX = e.button.x;
            int mouseY = e.button.y;
            
            int x = mouseX / CELL_SIZE;
            int y = mouseY / CELL_SIZE;

            if (rmb_held == 1)
            {
                // int rad = grid_height / 8;
                // int amount = rad / 16;
                // int perc = 2 * rad;
                // int dx, dy;
                // for(int n = 0; n < amount; n++)
                // {
                //     dx = rand() % (2 * rad) - rad;
                //     dy = rand() % (2 * rad) - rad;
                    
                //     if((rand() % perc + rand() % perc) / 2 > abs(dx) + abs(dy)
                //     && Grid_Get(x + dx, y + dy)->id == 0)
                //         Grid_Set(x + dx, y + dy, 0, 1);
                // }
                Cell_Create(x, y, 0, rand() % 2, 0);
                
                
            }
            if (lmb_held == 1)
            {
                // if(Find_Free_Genome_Id() != 0)
                // {
                //     int rad = grid_height / 8;
                //     int amount = rad / 16;
                //     int perc = 2 * rad;
                    
                //     int dx, dy;
                //     for(int n = 0; n < amount; n++)
                //     {
                //         dx = rand() % (2 * rad) - rad;
                //         dy = rand() % (2 * rad) - rad;
                        
                //         if((rand() % perc + rand() % perc) / 2 > abs(dx) + abs(dy))
                //             Cell_Create(x + dx, y + dy, 0, rand() % 2, 0);
                //     }
                // }
                
                // Phero_Set(x, y, 0, 255);
                
                // Grid_Set(x, y, 0, 1);
                
                
            }
            if (mmb_held == 1)
            {
                dest_x = x;
                dest_y = y;
            }
        }
    }
}

void Events_Process()
{
    int dx = (dest_x - grab_x);
    int dy = (dest_y - grab_y);
    int str = abs(dx) + abs(dy);
    
    dx = sign(dx);
    dy = sign(dy);
    
    if(Grid_Get(grab_x, grab_y)->type != 0 && str > 0)
    {
        if(Rec_Push(grab_x, grab_y, dx, dy, str, 0))
        {
            // printf("\ntimer %3d dest_x %d dest_y %d grab_x %d grab_y %d dx %d dy %d str %d\n", timer, dest_x, dest_y, grab_x, grab_y, dx, dy, str);
            
            grab_x += dx;
            grab_y += dy;
        }
    }
}
