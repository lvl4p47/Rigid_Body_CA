#include "input.h"

int quit = 0;
uint8_t slow_mode = 0;
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
                last_frame = 0;
            }
            if(e.key.keysym.sym == SDLK_w)
            {
                display_mode = mod(display_mode + 1, DISP_MODE_COUNT);
                last_frame = 0;
            }
            if(e.key.keysym.sym == SDLK_r)
            {
                Life_Reset(1000);
                Grid_Reset(0, 1000);
                Grid_Reset_Half(1, soil);
                Reanimate(1000);
            }
            if(e.key.keysym.sym == SDLK_t)
            {
                sudden_death = 1 - sudden_death;
                // save_them = 1 - save_them;
            }
            if(e.key.keysym.sym == SDLK_p)
            {
                Populate(1);
            }
            if(e.key.keysym.sym == SDLK_s)
            {
                slow_mode = 1 - slow_mode;
            }
            if(e.key.keysym.sym == SDLK_d)
            {
                draw_dots = 1 - draw_dots;
            }
            if(e.key.keysym.sym == SDLK_l)
            {
                draw_links = 1 - draw_links;
            }
            if(e.key.keysym.sym == SDLK_ESCAPE)
            {
                // SDL_MinimizeWindow(window);
                quit = 1;
            }
            if(e.key.keysym.sym == SDLK_SPACE)
            {
                pause = 1 - pause;
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
                
                Cell_Create(x, y, 0, 0);
                
            }
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                lmb_held = 1;
                
                grab_x = x;
                grab_y = y;
                dest_x = x;
                dest_y = y;
                
            }
            if (e.button.button == SDL_BUTTON_MIDDLE) 
            {
                mmb_held = 1;
                
                Rec_Connect(x, y, 10);
                
                // Rec_Push_Away(x, y, 0, -1, 1, 0);
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
                
                Cell_Create(x, y, 0, 0);
                
                // if(Find_Free_Genome_Id() != 0)
                // {
                //     int rad = grid_height / 8;
                //     int amount = max(rad / 4, 1);
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
                
                
            }
            if (mmb_held == 1)
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
            if (lmb_held == 1)
            {
                dest_x = x;
                dest_y = y;
            }
        }
    }
}

void Events_Process()
{
    if(lmb_held == 0) return;
    
    int dx = (dest_x - grab_x);
    int dy = (dest_y - grab_y);
    
    uint8_t ax = abs(dx);
    uint8_t ay = abs(dy);
    int8_t sx = sign(dx);
    int8_t sy = sign(dy);
    uint8_t w_diag = min(ax, ay);
    uint8_t w_total = max(ax, ay);
    uint8_t w_axis = w_total - w_diag;
    
    if(w_total != 0)
    {
        uint8_t r = rand() % w_total;
        if (r >= w_diag)
        {
            if(ax > ay) 
            {
                sy = 0;
            }
            else 
            {
                sx = 0;
            }
        }
    }
    
    int str = ax + ay;
    int32_t moved = 0;
    
    if(Grid_Get(grab_x, grab_y)->type != 0 && str > 0)
    {
        if(push_away && 0)
        {
            moved = Rec_Push_CoM(grab_x, grab_y, sx, sy, str);
            // grab_x += sx * moved;
            // grab_y += sy * moved;
            printf("grab_x %d grab_y %d moved %d\n", grab_x, grab_y, moved);
        }
        else
        {
            moved = Rec_Push(grab_x, grab_y, sx, sy, str, 0);
        }
    }
}
