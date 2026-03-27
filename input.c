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
            }
            if (e.button.button == SDL_BUTTON_LEFT) 
            {
                lmb_held = 1;
                Rec_Push(x, y, 1, 0, 10);
            }
            if (e.button.button == SDL_BUTTON_MIDDLE) 
            {
                
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
                Grid_Set(x, y, 1, 1);
            }
            if (lmb_held == 1)
            {
                
            }
        }
    }
}
