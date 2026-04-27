#ifndef GRID_H
#define GRID_H

#include "utility.h"

#define MAX_PHEROMONES 8

extern uint8_t global_time;

typedef struct {
    uint32_t id;
    uint8_t type;
    uint8_t matter;
    uint8_t energy;
    int32_t rec_str;
    uint8_t on_edge;
    uint8_t links;
    uint8_t buf_links;
    uint8_t buf_energy_out;
    uint8_t buf_matter_out;
    uint8_t will_move;
    uint8_t light;
    
    uint8_t pheromone[MAX_PHEROMONES][3];
} Tile;

extern uint16_t grid_width;
extern uint16_t grid_height;

extern uint8_t timer;
extern uint32_t long_timer;
extern Tile **grid_array;
extern uint8_t border;
extern uint8_t max_matter;
extern uint8_t starting_matter;
extern int grab_x, grab_y;
extern int lmb_held, rmb_held, mmb_held;
extern uint16_t soil;
extern int32_t energy_gain, energy_loss;
extern uint16_t cycles;
extern uint32_t total_cycles;

typedef enum
{
    X_DIRECTION,
    Y_DIRECTION,
    STRENGTH,
    DISTANCE
} indeces;

void Grid_Init(uint16_t w, uint16_t h);
void Grid_Quit();
void Grid_Reset(uint8_t type, uint16_t chance);
void Border();
void Grid_Reset_Half(uint8_t type, uint16_t chance);

static inline Tile* Grid_Get(int16_t x, int16_t y)
{
    uint16_t x1 = mod(x, grid_width);
    uint16_t y1 = mod(y, grid_height);
    return &grid_array[y1][x1];
}

void Grid_Set(int16_t x, int16_t y, uint32_t id, uint8_t type);
void Grid_Move(int16_t x, int16_t y, int16_t dx, int16_t dy);
void Grid_Update();

int32_t Rec_Can_Move(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid, uint8_t linked);
void Rec_Move(int16_t x, int16_t y, int8_t dx, int8_t dy, uint32_t *moved);
uint8_t Active_Neighbors(int16_t x, int16_t y);
void Rec_Clean(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t depth);
void Rec_Immovable(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength);
uint32_t Rec_Push(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid);
uint32_t Rec_Push_Away(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid);
uint32_t Rec_Push_Fake(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid);
uint32_t Rec_Push_Attempt(int16_t x, int16_t y, int8_t dx, int8_t dy, int32_t strength, uint8_t rigid);
void Rec_Link_All(int16_t x, int16_t y, int32_t strength);
void Rec_Connect(int16_t x, int16_t y, int32_t strength);
uint8_t Is_Membrane(int16_t x, int16_t y);
uint8_t Neighbor_Energy(int16_t x, int16_t y);

void Global_Time_Update();
void Phero_Set(int16_t x, int16_t y, uint8_t type, uint8_t range);
uint8_t Phero_Get(int16_t x, int16_t y, uint8_t type, uint8_t update);

// CELLS

#define MAX_CELLS 4000000

typedef struct {
    uint16_t x;
    uint16_t y;
    uint8_t buf_matter;
    uint8_t buf_energy;
    uint8_t energy_out;
    uint8_t matter_out;
    uint8_t dir;
    uint8_t photo;
    
    uint32_t prev;
    uint32_t next;
    uint32_t parent;
    
    uint8_t used;
    uint8_t active;
    uint16_t g_id;
    uint8_t pc;
    
    Stack call_stack;
    Stack data_stack;
    
    uint8_t acc;
} Cell;

extern Cell cells[MAX_CELLS];

#endif