#include "graphics.h"
#include <SDL2/SDL_image.h>

SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;

uint8_t draw_links = 1;
uint8_t display_mode = TROPHS;
uint32_t prev_matter = 0;


void Graphics_Init()
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return ;
    }

    window = SDL_CreateWindow("templateSDL", 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
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
    
    int16_t x, y;
    
    switch (display_mode)
    {
    case ENERGY:
        uint32_t total_matter = 0;
        for(int i = 0; i < grid_height; i++)
        {
            for(int j = 0; j < grid_width; j++)
            {
                tile = Grid_Get(j, i);
                int type = tile->type;
                int matter = tile->matter;
                int energy = tile->energy;
                int id = tile->id;
                int photo = cells[id].photo;
                int str = tile->rec_str;
                uint8_t links = tile->links;
                int r = 0, g = 0, b = 0;
                
                if(type != 0) total_matter++;
                total_matter += matter;
                    
                r = matter;
                b = energy;
                
                rect.x = j * CELL_SIZE;
                rect.y = i * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        
        // printf("total_matter %d\n", total_matter);
        // if(total_matter != prev_matter && prev_matter != 0) printf("%d\n", 1 / 0);
        prev_matter = total_matter;
        break;
    case TROPHS:
        for(uint32_t id = 1; id < MAX_CELLS; id++)
        {
            if(cells[id].used)
            {
                x = cells[id].x;
                y = cells[id].y;
                tile = Grid_Get(x, y);
                int type = tile->type;
                int matter = tile->matter;
                int energy = tile->energy;
                int photo = cells[id].photo;
                int str = tile->rec_str;
                uint8_t links = tile->links;
                // links = cells[tile->id].outlet;
                int r = 0, g = 0, b = 0;
                    
                switch (type)
                {
                case 0:
                    
                    break;
                
                default:
                    r = 0, g = 0, b = energy;
                    if(id != 0)
                    {
                        if(photo)
                            g = 255;
                        else r = 255;
                    }
                    break;
                }
                
                rect.x = x * CELL_SIZE;
                rect.y = y * CELL_SIZE;
                
                if(draw_links == 1 && 0)
                {
                    r = 0, g = 0, b = 0;
                }
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
                
                if(draw_links == 0) continue;
                
                switch (type)
                {
                case 0:
                    
                    break;
                
                default:
                    r = 0, g = 0, b = energy;
                    if(id != 0)
                    {
                        if(photo)
                            g = 255;
                        else r = 255;
                    }
                    break;
                }
                
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
        break;
    case GENOMES:
        for(int id = 1; id < MAX_CELLS; id++)
        {
            if(cells[id].used)
            {
                x = cells[id].x;
                y = cells[id].y;
                tile = Grid_Get(x, y);
                int type = tile->type;
                int energy = tile->energy;
                int gen_id = cells[id].g_id;
                int r = 0, g = 0, b = 0;
                    
                switch (type)
                {
                case 0:
                    
                    break;
                
                default:
                    r = gen_id % 255;
                    g = (uint8_t)((gen_id >> 8) ^ (gen_id & 0xFF));;
                    b = (uint8_t)((gen_id * 2654435761u) >> 24);
                    
                    // r = r * energy / 255;
                    // g = g * energy / 255;
                    // b = b * energy / 255;
                }
                
                rect.x = x * CELL_SIZE;
                rect.y = y * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        break;
    case ACC:
        for(int id = 1; id < MAX_CELLS; id++)
        {
            if(cells[id].used)
            {
                x = cells[id].x;
                y = cells[id].y;
                tile = Grid_Get(x, y);
                int type = tile->type;
                int acc = cells[tile->id].acc;
                
                int r = 0, g = 0, b = 0;
                    
                switch (type)
                {
                case 0:
                    
                    break;
                
                default:
                    if(acc == 0) 1;
                    else if(acc < 85) r = 255;
                    else if(acc < 170) g = 255;
                    else b = 255;
                    break;
                }
                
                rect.x = x * CELL_SIZE;
                rect.y = y * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        break;
    case PHERO:
        for(int i = 0; i < grid_height; i++)
        {
            for(int j = 0; j < grid_width; j++)
            {
                tile = Grid_Get(j, i);
                
                int c_0 = 0, c_1 = 0, c_2 = 0;
                int r = 0, g = 0, b = 0;
                c_0 = Phero_Get(j, i, 0, 0);
                // c_0 = tile->pheromone[0][0];
                c_1 = Phero_Get(j, i, 1, 0);
                int count = 0;
                for(int p = 2; p < MAX_PHEROMONES; p++)
                {
                    count++;
                    c_2 += Phero_Get(j, i, p, 0);
                }
                
                if(c_2 != 0) 
                {
                    c_2 = c_2 / count;
                }
                
                r = c_0;
                g = c_1;
                b = c_2;
                
                rect.x = j * CELL_SIZE;
                rect.y = i * CELL_SIZE;
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
            }
        }
        break;
    case DEBUG:
        for(uint32_t id = 1; id < MAX_CELLS; id++)
        {
            if(cells[id].used)
            {
                x = cells[id].x;
                y = cells[id].y;
                tile = Grid_Get(x, y);
                int type = tile->type;
                int value = tile->on_edge;
                int str = tile->rec_str;
                uint8_t links = tile->links;
                // links = cells[tile->id].outlet;
                int r = 0, g = 0, b = 0;
                    
                switch (value)
                {
                case 0:
                    r = 83;
                    break;
                case 1:
                    r = 166;
                    break;
                case 2:
                    r = 255;
                    break;
                }
                
                g = (str > 0) * 255;
                b = (str < 0) * 255;
                
                // if(value != 0) pause = 1;
                
                rect.x = x * CELL_SIZE;
                rect.y = y * CELL_SIZE;
                
                if(draw_links == 1 && 0)
                {
                    r = 0, g = 0, b = 0;
                }
                
                SDL_SetRenderDrawColor(renderer, r, g, b, 255);
                SDL_RenderFillRect(renderer, &rect);
                
                if(draw_links == 0) continue;
                
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
                        // if(Grid_Get(x + dir_to_coords[dir][0], y + dir_to_coords[dir][1])->type == 0)
                        // {
                        //     pause = 1;
                        //     SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                        // }
                        SDL_RenderDrawLineF(renderer, cx, cy, cx + dx, cy + dy);
                    }
                }
            }
        }
        break;
    
    default:
        break;
    }
    
}

