#ifndef GRID_H
#define GRID_H

#include "utility.h"

typedef struct {
    uint32_t id;
    uint8_t type;
    uint8_t material;
    uint8_t energy;
    int16_t rec_str;
    uint8_t links;
} Tile;

extern uint16_t grid_width;
extern uint16_t grid_height;

extern uint8_t timer;
extern Tile **grid_array;

typedef enum
{
    X_DIRECTION,
    Y_DIRECTION,
    STRENGTH,
    DISTANCE
} indeces;

void Grid_Init(uint16_t w, uint16_t h);
void Grid_Quit();
void Grid_Reset();

static inline Tile* Grid_Get(int16_t x, int16_t y)
{
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    return &grid_array[y1][x1];
}

void Grid_Set(int16_t x, int16_t y, uint32_t id, uint8_t type);
void Grid_Move(int16_t x, int16_t y, int16_t dx, int16_t dy);
void Grid_Update();

int8_t Rec_Can_Move(int16_t x, int16_t y, int8_t dx, int8_t dy, int16_t strength, uint8_t rigid);
void Rec_Move(int16_t x, int16_t y, int8_t dx, int8_t dy);
void Rec_Clean(int16_t x, int16_t y, int8_t dx, int8_t dy);
void Rec_Push(int16_t x, int16_t y, int8_t dx, int8_t dy, int16_t strength, uint8_t rigid);
void Rec_Link_All(int16_t x, int16_t y, int16_t strength);
void Rec_Connect(int16_t x, int16_t y, int16_t strength);

// CELLS

#define MAX_CELLS 100000

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t dir;
    
    uint32_t prev;
    uint32_t next;
    
    uint8_t used;
    uint16_t g_id;
    uint8_t pc;
    
    Stack call_stack;
    Stack data_stack;
    
    uint8_t acc;
} Cell;

extern Cell cells[MAX_CELLS];

#endif