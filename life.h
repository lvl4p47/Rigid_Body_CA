#ifndef LIFE_H
#define LIFE_H

#include "grid.h"

void Cells_Init();
void Cells_Update();
uint32_t Find_Free_Id();
void Cell_Create(int16_t x, int16_t y, uint32_t parent);
void Cell_Destroy(uint32_t id);

// GENOME

#define GENOME_SIZE 64
#define MAX_GENOMES 10000
#define MAX_STEPS 64

typedef enum {
    CMD_NO_OP,
    CMD_LABEL,
    CMD_EXEC,
    CMD_RET,
    CMD_PUSH_IMM,
    CMD_PUSH_ACC,
    CMD_POP,
    CMD_ADD_POP,
    CMD_SUB_POP,
    CMD_MULTIPLY,
    CMD_ROT,
    CMD_MOVE,
    CMD_EAT,
    CMD_LOOK
} GeneCmd;

typedef struct {
    uint8_t cmd;
    uint8_t arg;
} Gene;

typedef struct {
    Gene genes[GENOME_SIZE];
    uint32_t used;
} Genome;

extern Genome genomes[MAX_GENOMES];

void Genomes_Init();
void Genomes_Print();
uint16_t Find_Free_Genome_Id();
uint16_t Genome_Create(uint16_t par_id);
void Genome_Destroy(uint16_t g_id);
void Genome_Copy(uint16_t g_id_to, uint16_t g_id_from, uint8_t mut_rate);

void Cell_Exec(uint32_t id);
int16_t Find_Tag(Genome *genome, uint8_t tag);

#endif