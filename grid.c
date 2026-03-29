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
    
    grid_array[y1][x1].id = id;
    grid_array[y1][x1].type = type;
    grid_array[y1][x1].material = 0;
    grid_array[y1][x1].energy = 0;
    grid_array[y1][x1].rec_str = 0;
    grid_array[y1][x1].links = 0;
}

void Grid_Move(int16_t x, int16_t y, int16_t dx, int16_t dy)
{
    if(debug) fprintf(stderr, "\nGrid_Set"), fflush(stderr);
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    uint16_t x2 = mod(x + dx, grid_width);
    uint16_t y2 = mod(y + dy, grid_height);
    
    if(grid_array[y2][x2].id != 0) return;
    
    uint32_t id = grid_array[y1][x1].id;
    
    cells[id].x = x2;
    cells[id].y = y2;
    
    grid_array[y2][x2].id = grid_array[y1][x1].id;
    grid_array[y2][x2].type = grid_array[y1][x1].type;
    grid_array[y2][x2].material = grid_array[y1][x1].material;
    grid_array[y2][x2].energy = grid_array[y1][x1].energy;
    grid_array[y2][x2].rec_str = 0;
    grid_array[y2][x2].links = grid_array[y1][x1].links;
    
    grid_array[y1][x1].id = 0;
    grid_array[y1][x1].type = 0;
    grid_array[y1][x1].material = 0;
    grid_array[y1][x1].energy = 0;
    grid_array[y1][x1].rec_str = 0;
    grid_array[y1][x1].links = 0;
}

void Grid_Update()
{
    if(debug) 
    {
        freopen("debug.log", "w", stderr);
        fprintf(stderr, "\nGrid_Update"), fflush(stderr);
    }
    
}

int8_t Rec_Can_Move(int16_t x, int16_t y, int8_t dx, int8_t dy, int16_t strength, uint8_t rigid)
{
    uint8_t local_debug = 0;
    
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    int8_t problems = 0;
    int8_t ret;
    
    if(local_debug) printf("start str %d c %d\n", strength, center->links);
    
    if(strength == 0 || center->type == 0) 
    {
        if(local_debug) printf("too weak to move str %d\n", strength);
        if(center->rec_str == 0 && center->type != 0)
        {
            center->rec_str = -1; // this tile was not reached
            return 1; // too weak to move, add solvable problem
        }
        else return 0; // can't reach now, but it was reached by some other branch
    }
    
    if(center->rec_str == -1) // reached previously unreachable, str != 0
    {
        if(local_debug) printf("solution\n");
        problems = -1;
        center->rec_str = strength;
    }
    
    center->rec_str = strength;
    
    neighbor = Grid_Get(x + dx, y + dy);
    if(neighbor->type != 0)
    {
        if(neighbor->rec_str < strength - 1
        || neighbor->rec_str == 0)
        {
            ret = Rec_Can_Move(x + dx, y + dy, dx, dy, strength - 1, rigid);
            problems += ret;
            if(ret < 0)
            {
                if(local_debug) printf("more solutions than problems in front str %d\n", strength);
            }
            if(ret > 0)
            {
                if(local_debug) printf("My neighbor in the direction can't move str%d\n", strength);
            }
            
        }
    }
    else if(neighbor->type != 0)
        problems += 1; // add unsolvable problem
    
    int16_t Dx, Dy;
    int16_t nx, ny;
    uint8_t mask, opposite;
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        mask = (uint8_t)1 << dir;
        opposite = (uint8_t)1 << mod(dir + 4, 8);
        Dx = dir_to_coords[dir][0];
        Dy = dir_to_coords[dir][1];
        nx = x + Dx;
        ny = y + Dy;
        neighbor = Grid_Get(nx, ny);
        
        if(neighbor->type != 0
        && (neighbor->rec_str < strength - 1
        || neighbor->rec_str == 0)
        )
        {
            if(center->links & mask)
            {
                ret = Rec_Can_Move(nx, ny, dx, dy, strength - 1, rigid);
                problems += ret;
                if(local_debug) printf("str %d problems %d\n", strength, problems);
                
                if(rigid == 0)
                {
                    if(ret > 0 && strength == 1)
                    {
                        if(max(abs(nx - x - dx), abs(ny - y - dy)) < 2)
                        {
                            neighbor->rec_str = 0;
                            if(local_debug) printf("no tearing dir %d\n", dir);
                            problems -= ret;
                            
                            if(local_debug) printf("c %d n %d\n", center->links, neighbor->links);
                            
                            center->links &= (uint8_t)~mask;
                            neighbor->links &= (uint8_t)~opposite;
                            
                            if(local_debug) printf("c %d n %d\n", center->links, neighbor->links);                         
                            
                            uint8_t new_dir = coords_to_dir[Dy - dy + 1][Dx - dx + 1];
                            if(local_debug) printf("dx %d dy %d\n", Dx - dx + 1, Dy - dy + 1);
                            if(local_debug) printf("new dir %d\n", new_dir);
                            
                            mask = (uint8_t)1 << new_dir;
                            opposite = (uint8_t)1 << mod(new_dir + 4, 8);
                            
                            if(local_debug) printf("m %d o %d\n", mask, opposite);
                            
                            center->links |= mask;
                            neighbor->links |= opposite;
                            
                            if(local_debug) printf("c %d n %d\n", center->links, neighbor->links);
                        }
                        else
                        {
                            // problems += 1;
                            if(local_debug) printf("tearing\n");
                        }
                    }
                }
                if(local_debug) printf("str %d problems %d\n", strength, problems);
                if(ret < 0)
                {
                    if(local_debug) printf("more solutions than problems from side str %d\n", strength);
                }
                if(ret > 0) 
                {
                    if(local_debug) printf("My neighbor to the side can't move str %d\n", strength);
                }
            }
        }
    }
    
    if(local_debug) printf("finish str %d problems %d c %d\n", strength, problems, center->links);
    
    return problems;
}

void Rec_Move(int16_t x, int16_t y, int8_t dx, int8_t dy)
{
    uint8_t local_debug = 0;
    
    Tile *center = Grid_Get(x, y);
    Tile *neighbor, *linked;
    int8_t problems = 0;
    int16_t str = center->rec_str;
    if(str < -1) str = -1 - str;
    
    if(str <= 0) 
    {
        if(center->rec_str == -1)
            center->rec_str = 0;
        return;
    }
    
    if(local_debug) printf("x %d y %d str %d\n", x, y, str);
    
    neighbor = Grid_Get(x + dx, y + dy);
    if(neighbor->type != 0)
    {
        if(neighbor->rec_str == str - 1)
        {
            if(local_debug) printf("x %d y %d str %d front reaching x %d y %d str %d\n", x, y, str, x + dx, y + dy, neighbor->rec_str);
            Rec_Move(x + dx, y + dy, dx, dy);
        }
    }
    else if(neighbor->type != 0)
        problems = 1;
        
    if(neighbor->type == 0)
    {
        Grid_Move(x, y, dx, dy);
        if(local_debug) printf("x %d y %d str %d moved\n", x, y, str);
        center = Grid_Get(x + dx, y + dy);
    }
    else
    {
        return;
    }
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            if(neighbor->type != 0
            && (neighbor->rec_str >= str - 1
            || neighbor->rec_str <= 0)
            )
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
    int8_t problems = 0;
    int16_t str = center->rec_str;
    
    if(local_debug) printf("x %d y %d str %d\n", x, y, str);
    
    neighbor = Grid_Get(x + dx, y + dy);
    if(neighbor->type != 0)
    {
        if(neighbor->rec_str != 0)
        {
            if(local_debug) printf("x %d y %d str %d front reaching x %d y %d str %d\n", x, y, str, x + dx, y + dy, neighbor->rec_str);
            Rec_Clean(x + dx, y + dy, dx, dy);
        }
    }
        
    center->rec_str = 0;
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            if(neighbor->type != 0
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

void Rec_Push(int16_t x, int16_t y, int8_t dx, int8_t dy, int16_t strength, uint8_t rigid)
{
    int16_t ret = Rec_Can_Move(x, y, dx, dy, strength, rigid);
    // printf("\nRCM %d\n", ret);
    if(ret <= 0) 
        Rec_Move(x, y, dx, dy); 
    else Rec_Clean(x, y, dx, dy);
}

void Rec_Link_All(int16_t x, int16_t y, int16_t strength)
{
    Tile *center = Grid_Get(x, y);
    Tile *neighbor;
    uint8_t just_remove = (center->type == 0);
    
    if(strength == 0) 
    {
        return;
    }
    
    center->rec_str = strength;
    
    for(int ny = y - 1; ny <= y + 1; ny++)
    {
        for(int nx = x - 1; nx <= x + 1; nx++)
        {
            neighbor = Grid_Get(nx, ny);
            if(neighbor->type != 0
            && neighbor->rec_str < strength - 1
            || just_remove)
            {
                Rec_Link_All(nx, ny, strength - 1);
            }
        }
    }
    
    uint8_t mask;
    int16_t dx, dy;
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        mask = (uint8_t)1 << dir;
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        
        if(just_remove)
        {
            continue;
        }
        
        if(neighbor->type != 0) center->links |= mask;
        else center->links &= (uint8_t)~mask;
        // printf("mask %d links %d n_id %d, dx %d, dy %d\n", mask, center->links, neighbor->id, dx, dy);
    }
}

void Rec_Connect(int16_t x, int16_t y, int16_t strength)
{
    Rec_Link_All(x, y, strength);
    Rec_Clean(x, y, 1, 0);
}