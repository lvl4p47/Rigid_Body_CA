#include "graphics.h"
#include <SDL2/SDL_image.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

uint8_t draw_links = 1;


void Graphics_Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return ;
    }

    window = SDL_CreateWindow("templateSDL", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WINDOW_WIDTH, WINDOW_HEIGHT,
    SDL_WINDOW_BORDERLESS);
    if (window == NULL) {
        fprintf(stderr, "Window could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_Quit();
        return ;
    }
    
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) {
        fprintf(stderr, "Renderer could not be created! SDL Error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return ;
    }
}

void Graphics_Quit()
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void Screen_Clear()
{
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void Screen_Draw()
{
    Grid_Draw();
    SDL_RenderPresent(renderer);
}

void Grid_Draw()
{
    SDL_Rect rect;
    rect.x = 0;
    rect.y = 0;
    rect.w = CELL_SIZE;
    rect.h = CELL_SIZE;
    
    Tile *tile;
    
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            tile = Grid_Get(j, i);
            int type = tile->type;
            int id = tile->id;
            int str = tile->rec_str;
            uint8_t links = tile->links;
            int r = 0, g = 0, b = 0;
                
            switch (type)
            {
            case 0:
                
                break;
            
            default:
                r = 127, g = 127, b = 127;
                if(id != 0) b = 255;
                break;
            }
            
            rect.x = j * CELL_SIZE;
            rect.y = i * CELL_SIZE;
            
            SDL_SetRenderDrawColor(renderer, r, g, b, 255);
            SDL_RenderFillRect(renderer, &rect);
            
            if(draw_links == 0) continue;
            
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            uint8_t mask;
            int16_t dx, dy, cx, cy;
            cx = rect.x + CELL_SIZE / 2;
            cy = rect.y + CELL_SIZE / 2;
            for(uint8_t dir = 0; dir < 8; dir++)
            {
                dx = dir_to_coords[dir][0] * CELL_SIZE / 2;
                dy = dir_to_coords[dir][1] * CELL_SIZE / 2;
                mask = (uint8_t)1 << dir;
                
                if(links & mask)
                {
                    SDL_RenderDrawLineF(renderer, cx, cy, cx + dx, cy + dy);
                }
            }
        }
    }
}

