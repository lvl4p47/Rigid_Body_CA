#ifndef LIFE_H
#define LIFE_H

#include "grid.h"

typedef struct {
    uint16_t x;
    uint16_t y;
    
    uint32_t prev;
    uint32_t next;
} Cell;

#endif