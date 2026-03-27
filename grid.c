#include "grid.h"

Tile **grid_array = NULL;
uint16_t grid_width = 0;
uint16_t grid_height = 0;

uint8_t timer = 0;
uint8_t debug = 0;

FILE *file_ptr;
uint16_t integer;

void Grid_Init(uint16_t w, uint16_t h)
{
    grid_width = w;
    grid_height = h;
    
    grid_array = (Tile**)malloc(h * sizeof(Tile*));
    
    for(int i = 0; i < h; i++)
    {
        grid_array[i] = (Tile*)malloc(w * sizeof(Tile));
    }
    
    Grid_Reset(0);
}

void Grid_Quit()
{
    for(int i = 0; i < grid_height; i++)
    {
        free(grid_array[i]);
    }
    
    free(grid_array);
}

void Grid_Reset()
{
    if(debug) fprintf(stderr, "\nGrid_Reset"), fflush(stderr);
    for(int i = 0; i < grid_height; i++)
    {
        for(int j = 0; j < grid_width; j++)
        {
            Grid_Set(j, i, 0, 0);
        }
    }
}

void Grid_Set(int16_t x, int16_t y, uint32_t id, uint8_t type)
{
    if(debug) fprintf(stderr, "\nGrid_Set"), fflush(stderr);
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t temp_id = grid_array[y1][x1].id;
    
    grid_array[y1][x1].id = id;
    grid_array[y1][x1].type = type;
    grid_array[y1][x1].rec_str = 0;
}

void Grid_Update()
{
    if(debug) 
    {
        freopen("debug.log", "w", stderr);
        fprintf(stderr, "\nGrid_Update"), fflush(stderr);
    }
    
}

uint8_t Rec_Can_Move(int16_t x, int16_t y, int8_t dx, int8_t dy, uint16_t strength)
{
    uint8_t local_debug = 0;
    
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    uint8_t can_move = 1;
    
    if(strength == 0 || center->type == 0) return 0;
    
    center->rec_str = strength;
    
    neighbor = Grid_Get(x + dx, y + dy);
    if(neighbor->id == center->id)
    {
        if(neighbor->rec_str < strength - 1
        || neighbor->rec_str == 0)
        {
            if(Rec_Can_Move(x + dx, y + dy, dx, dy, strength - 1) == 0)
            {
                can_move = 0;
                if(local_debug) printf("My neighbor in the direction can't move\n");
            }
        }
    }
    else if(neighbor->type != 0)
        can_move = 0;
    
    if(can_move == 0)
        return 0;
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            if(neighbor->id == center->id
            && (neighbor->rec_str < strength - 1
            || neighbor->rec_str == 0)
            )
            {
                if(Rec_Can_Move(nx, ny, dx, dy, strength - 1) == 0)
                {
                    can_move = 0;
                    if(local_debug) printf("My neighbor to the side can't move\n");
                }
            }
        }
    }
    
    return can_move;
}

void Rec_Move(int16_t x, int16_t y, int8_t dx, int8_t dy)
{
    uint8_t local_debug = 0;
    
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    uint8_t can_move = 1;
    uint16_t str = center->rec_str;
    
    if(str <= 0) return;
    
    if(local_debug) printf("x %d y %d str %d\n", x, y, str);
    
    neighbor = Grid_Get(x + dx, y + dy);
    if(neighbor->id == center->id)
    {
        if(neighbor->rec_str == str - 1)
        {
            if(local_debug) printf("x %d y %d str %d front reaching x %d y %d str %d\n", x, y, str, x + dx, y + dy, neighbor->rec_str);
            Rec_Move(x + dx, y + dy, dx, dy);
        }
    }
    else if(neighbor->type != 0)
        can_move = 0;
    
    if(can_move == 0)
        return;
        
    if(neighbor->type == 0)
    {
        Grid_Set(x + dx, y + dy, center->id, center->type);
        Grid_Set(x, y, 0, 0); 
        center->rec_str = 0;
        if(local_debug) printf("x %d y %d str %d moved\n", x, y, str);
    }
    else
    {
        return;
    }
    
    center = Grid_Get(x + dx, y + dy);
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            if(neighbor->id == center->id
            && neighbor->rec_str >= str - 1)
            {
                if(local_debug) if(neighbor->rec_str > str) printf("%d > %d ", neighbor->rec_str, str - 1);
                if(local_debug) printf("x %d y %d str %d side reaching x %d y %d str %d\n", x, y, str, nx, ny, neighbor->rec_str);
                Rec_Move(nx, ny, dx, dy);
            }
        }
    }
    
    return;
}

void Rec_Clean(int16_t x, int16_t y, int8_t dx, int8_t dy)
{
    uint8_t local_debug = 0;
    
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    uint8_t can_move = 1;
    uint16_t str = center->rec_str;
    
    if(local_debug) printf("x %d y %d str %d\n", x, y, str);
    
    neighbor = Grid_Get(x + dx, y + dy);
    if(neighbor->id == center->id)
    {
        if(neighbor->rec_str != 0)
        {
            if(local_debug) printf("x %d y %d str %d front reaching x %d y %d str %d\n", x, y, str, x + dx, y + dy, neighbor->rec_str);
            Rec_Clean(x + dx, y + dy, dx, dy);
        }
    }
    else if(neighbor->type != 0)
        can_move = 0;
    
    if(can_move == 0)
        return;
        
    center->rec_str = 0;
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            if(neighbor->id == center->id
            && neighbor->rec_str != 0)
            {
                if(local_debug) if(neighbor->rec_str > str) printf("%d > %d ", neighbor->rec_str, str - 1);
                if(local_debug) printf("x %d y %d str %d side reaching x %d y %d str %d\n", x, y, str, nx, ny, neighbor->rec_str);
                Rec_Clean(nx, ny, dx, dy);
            }
        }
    }
    
    return;
}

void Rec_Push(int16_t x, int16_t y, int8_t dx, int8_t dy, uint16_t strength)
{
    if(Rec_Can_Move(x, y, dx, dy, strength)) 
        Rec_Move(x, y, dx, dy);
    else
        Rec_Clean(x, y, dx, dy);
}