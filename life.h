#ifndef LIFE_H
#define LIFE_H

#include "grid.h"

void Cells_Init();
void Cells_Update();
uint32_t Find_Free_Id();
void Cell_Create(int16_t x, int16_t y, uint32_t parent, uint8_t photo, uint8_t out_in);
void Cell_Destroy(uint32_t id);

// GENOME

#define GENOME_SIZE 255
#define MAX_GENOMES 60000
#define MAX_STEPS 1

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
    CMD_CMP_POP,
    CMD_MULTIPLY,
    CMD_ROT,
    CMD_CENTRE,
    CMD_MOVE,
    CMD_EAT,
    CMD_LOOK_TYPE,
    CMD_LOOK_LINK,
    CMD_LOOK_MEMB,
    CMD_LOOK_MAT,
    CMD_LOOK_NRG,
    CMD_LOOK_ACC,
    CMD_DETACH,
    CMD_ATTACH,
    CMD_OUTLET_OFF,
    CMD_OUTLET_ON,
    CMD_SET_PHERO,
    CMD_GET_PHERO,
    CMD_COUNT
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

extern int32_t energy_change;
extern uint32_t total_energy_acc;
extern uint8_t max_light;
extern uint8_t push_away;

void Genomes_Init();
void Genomes_Print();
uint16_t Find_Free_Genome_Id();
uint16_t Genome_Create(uint16_t par_id);
void Genome_Destroy(uint16_t g_id);
void Genome_Copy(uint16_t g_id_to, uint16_t g_id_from, uint16_t mut_rate);

void Cell_Exec(uint32_t id);
void Cell_Buf_Upd(uint32_t id);
void Redist_Energy(uint32_t id);
int16_t Find_Tag(Genome *genome, uint8_t tag);
void Populate(int n);
void Force_Multiply();
void Life_Reset(uint16_t n);
void Gravity();
void Illuminate();

#endif