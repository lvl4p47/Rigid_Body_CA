#include "life.h"

Cell cells[MAX_CELLS];
Genome genomes[MAX_GENOMES];

uint32_t free_id;
uint16_t free_g_id;

uint8_t mutation_rate = 1;
uint8_t starting_material = 1;
uint8_t starting_energy = 127;

uint8_t debug_life = 0;

void Cells_Init()
{
    for(uint32_t id = 0; id < MAX_CELLS; id++)
    {
        cells[id].x = 0;
        cells[id].y = 0;
        cells[id].dir = 8;
        cells[id].prev = 0;
        cells[id].next = 0;
        cells[id].used = 0;
        cells[id].g_id = 0;
        cells[id].pc = 0;
        cells[id].acc = 0;
        Stack_Reset(&cells[id].call_stack);
        Stack_Reset(&cells[id].data_stack);
    }
    
    free_id = 1;
}

void Cells_Update()
{
    if(debug_life) printf("Cells_Update\n");
    
    uint32_t id = 0;
    
    do
    {
        if(cells[id].used)
        {
            Cell_Exec(id);
        }
    
        id = cells[id].next;
    }
    while(id != 0);
}

uint32_t Find_Free_Id()
{
    if(debug_life) printf("Find_Free_Id\n");
    
    uint32_t counter = 0;
    while((cells[free_id].used != 0 || free_id == 0)
    && counter < MAX_CELLS)
    {
        free_id = mod(free_id + 1, MAX_CELLS);
        counter++;
    }
    
    if(cells[free_id].used == 0) return free_id;
    else return 0;
}

void Cell_Create(int16_t x, int16_t y, uint32_t parent)
{
    if(debug_life) printf("Cell_Create\n");
    
    uint32_t id = Find_Free_Id();

    if(id == 0) return;
    
    Tile *tile = Grid_Get(x, y);
    if(tile->type != 0) return;
    
    cells[id].x = mod(x, grid_width);
    cells[id].y = mod(y, grid_height);
    cells[id].dir = cells[parent].dir;
    cells[id].prev = cells[parent].prev;
    cells[id].next = parent;
    cells[id].pc = 0;
    cells[id].used = 1;
    cells[id].acc = 0;
    Stack_Reset(&cells[id].call_stack);
    Stack_Reset(&cells[id].data_stack);
    
    cells[cells[id].prev].next = id;
    cells[parent].prev = id;
    
    Grid_Set(x, y, id, 1);
    
    tile->matter = starting_material;
    tile->energy = starting_energy;
    
    if(parent == 0)
    {
        cells[id].g_id = Genome_Create(parent);
        Genome_Copy(cells[id].g_id, 0, 100 * mutation_rate);
    }
    else if(rand() % 100 < mutation_rate)
    {
        cells[id].g_id = Genome_Create(parent);
    }
    else
    {
        cells[id].g_id = cells[parent].g_id;
        genomes[cells[id].g_id].used++;
    }
    
    if(parent == 0) return;
    
    uint8_t dir = cells[id].dir;
    uint8_t opp = mod(dir + 4, 8);
    int16_t dx = dir_to_coords[opp][0];
    int16_t dy = dir_to_coords[opp][1];
    Tile *new = Grid_Get(x, y);
    Tile *par = Grid_Get(cells[parent].x, cells[parent].y);
    uint8_t mask_dir = (uint8_t)1 << dir;
    uint8_t mask_opp = (uint8_t)1 << opp;
    
    par->links |= mask_dir;
    new->links |= mask_opp;
}

void Cell_Destroy(uint32_t id)
{
    if(debug_life) printf("Cell_Destroy\n");
    
    if(id == 0) return;
    
    cells[cells[id].prev].next = cells[id].next;
    cells[cells[id].next].prev = cells[id].prev;
    
    Tile *tile = Grid_Get(cells[id].x, cells[id].y);
    tile->id = 0;
    tile->links = 0;
    
    genomes[cells[id].g_id].used--;
    
    if(genomes[cells[id].g_id].used == 0) Genome_Destroy(cells[id].g_id);
    
    cells[id].x = 0;
    cells[id].y = 0;
    cells[id].dir = 8;
    cells[id].prev = 0;
    cells[id].next = 0;
    cells[id].used = 0;
    cells[id].g_id = 0;
    cells[id].pc = 0;
    cells[id].acc = 0;
    Stack_Reset(&cells[id].call_stack);
    Stack_Reset(&cells[id].data_stack);
    
    free_id = id;
}

// GENOME

void Genomes_Init()
{
    for(uint16_t g_id = 0; g_id < MAX_GENOMES; g_id++)
    {
        genomes[g_id].used = 0;
        
        for(uint8_t g = 0; g < GENOME_SIZE; g++)
        {
            genomes[g_id].genes[g].cmd = 0;
            genomes[g_id].genes[g].arg = 0;
        }
    }
    
    
    free_g_id = 1;
}

void Genomes_Print()
{
    printf("Genomes_Print\n");
    for(uint16_t g_id = 1; g_id < MAX_GENOMES; g_id++)
    {
        if(genomes[g_id].used == 0 && g_id != 0) continue;
        
        printf("g_id %3d\n", g_id);
        for(uint8_t g = 0; g < GENOME_SIZE; g++)
        {
            printf("g %2d cmd %3d arg %3d\n", g, 
            genomes[g_id].genes[g].cmd, genomes[g_id].genes[g].arg);
        }
    }
}

uint16_t Find_Free_Genome_Id()
{
    if(debug_life) printf("Find_Free_Genome_Id\n");
    
    uint16_t counter = 0;
    while((genomes[free_g_id].used != 0 || free_g_id == 0)
    && counter < MAX_GENOMES)
    {
        free_g_id = mod(free_g_id + 1, MAX_GENOMES);
        counter++;
    }
    
    // printf("free_g_id %d\n", free_g_id);
    
    if(genomes[free_g_id].used == 0) return free_g_id;
    else return 0;
}

uint16_t Genome_Create(uint16_t par_id)
{
    if(debug_life) printf("Genome_Create\n");
    
    uint16_t g_id = Find_Free_Genome_Id();
    uint16_t par_g_id = cells[par_id].g_id;
    
    if(g_id == 0) return 0;
    
    genomes[g_id].used = 1;
    
    Genome_Copy(g_id, par_g_id, mutation_rate);
    
    return g_id;
}

void Genome_Destroy(uint16_t g_id)
{
    if(debug_life) printf("Genome_Destroy\n");
    
    if(g_id == 0) return;
    
    genomes[g_id].used = 0;
    
    free_g_id = g_id;
}

void Genome_Copy(uint16_t g_id_to, uint16_t g_id_from, uint8_t mut_rate)
{
    if(debug_life) printf("Genome_Copy to %d from %d\n", g_id_to, g_id_from);
    
    for(uint8_t g = 0; g < GENOME_SIZE; g++)
    {
        genomes[g_id_to].genes[g].cmd = genomes[g_id_from].genes[g].cmd;
        genomes[g_id_to].genes[g].arg = genomes[g_id_from].genes[g].arg;
        
        if(rand() % 100 < mut_rate) genomes[g_id_to].genes[g].cmd = rand() % GENOME_SIZE;
        if(rand() % 100 < mut_rate) genomes[g_id_to].genes[g].arg = rand() % GENOME_SIZE;
    }
}

void Cell_Exec(uint32_t id)
{
    if(debug_life) printf("Cell_Exec\n");
    
    uint8_t print = 0;
    
    if(id == 0) return;
    if(cells[id].g_id == 0) return;
    
    if(print) printf("Cell_Exec id %d\n", id);
    
    Cell *cell = &cells[id];
    Genome *genome = &genomes[cell->g_id];
    uint8_t *pc = &cell->pc;
    Gene *gene;
    
    int16_t pos;
    uint8_t read;
    int16_t temp;
    Tile *neighbor;
    Tile *itself = Grid_Get(cell->x, cell->y);
    uint8_t mask;
    
    int16_t x = cell->x;
    int16_t y = cell->y;
    int16_t dx = dir_to_coords[cell->dir][0];
    int16_t dy = dir_to_coords[cell->dir][1];
    
    uint8_t move = 0;
    
    itself->energy = max(itself->energy - 1, 0);
    
    for(int steps = 0; steps < MAX_STEPS; steps++)
    {
        gene = &genome->genes[*pc];
        
        if(print) printf("pc %2d cmd %3d arg %3d ", *pc, gene->cmd, gene->arg);
        
        switch (gene->cmd)
        {
        case CMD_NO_OP:
            *pc = mod(*pc + 1, GENOME_SIZE);
            break;
        case CMD_LABEL:
            *pc = mod(*pc + 1, GENOME_SIZE);
            break;
        case CMD_EXEC:
            pos = Find_Tag(genome, cell->acc);
            *pc = mod(*pc + 1, GENOME_SIZE);
            
            if(pos >= 0)
            {
                if(Stack_Push(&cell->call_stack, *pc))
                    *pc = pos;
            }
            break;
        case CMD_RET:
            if(Stack_Pop(&cell->call_stack, &read))
            {
                *pc = read % GENOME_SIZE;
            }
            else
            {
                *pc = mod(*pc + 1, GENOME_SIZE);
            }
            break;
        case CMD_PUSH_IMM:
            *pc = mod(*pc + 1, GENOME_SIZE);
            Stack_Push(&cell->data_stack, gene->arg);
            break;
        case CMD_PUSH_ACC:
            *pc = mod(*pc + 1, GENOME_SIZE);
            Stack_Push(&cell->data_stack, cell->acc);
            break;
        case CMD_POP:
            *pc = mod(*pc + 1, GENOME_SIZE);
            Stack_Pop(&cell->data_stack, &cell->acc);
            break;
        case CMD_ADD_POP:
            *pc = mod(*pc + 1, GENOME_SIZE);
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = cell->acc + read;
                cell->acc = min(temp, 255);
            }
            break;
        case CMD_SUB_POP:
            *pc = mod(*pc + 1, GENOME_SIZE);
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = cell->acc - read;
                cell->acc = max(temp, 0);
            }
            break;
        case CMD_MULTIPLY:
            *pc = mod(*pc + 1, GENOME_SIZE);
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(cell->dir == 8)
                cell->dir = rand() % 8;
            cell->acc = 0;
            if(itself->matter >= 2 && itself->energy > 0
            && neighbor->type == 0)
            {
                Cell_Create(x + dx, y + dy, id);
                itself->matter -= 2;
                itself->energy--;
                cell->acc = 1;
            }
            break;
        case CMD_ROT:
            *pc = mod(*pc + 1, GENOME_SIZE);
            
            if(cell->dir == 8)
                cell->dir = rand() % 8;
            temp = cell->dir + gene->arg;
            cell->dir = mod(temp, 8);
            break;
        case CMD_MOVE:
            *pc = mod(*pc + 1, GENOME_SIZE);
            
            if(cell->dir == 8)
                cell->dir = rand() % 8;
            cell->acc = 0;
            if(Rec_Push(x, y, dx, dy, 10, 0))
                cell->acc = 1;
            break;
        case CMD_EAT:
            *pc = mod(*pc + 1, GENOME_SIZE);
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(cell->dir == 8)
                cell->dir = rand() % 8;
            
            cell->acc = 0;
            if(neighbor->type != 0 && neighbor != itself)
            {
                temp = itself->matter + neighbor->matter;
                if(temp < 255)
                {
                    itself->matter = temp;
                    neighbor->matter = 0;
                    cell->acc += 1;
                }
                temp = itself->energy + neighbor->energy;
                if(temp < 255)
                {
                    itself->energy = temp;
                    neighbor->energy = 0;
                    cell->acc += 1;
                }
                temp = itself->matter + 1;
                if(neighbor->matter == 0 && neighbor->energy == 0
                && temp < 255)
                {
                    if(neighbor->id != 0)
                        Cell_Destroy(neighbor->id);
                    Grid_Set(x + dx, y + dy, 0, 0);
                    itself->matter = temp;
                    cell->acc += 1;
                }
            }
            break;
        case CMD_LOOK_TYPE:
            *pc = mod(*pc + 1, GENOME_SIZE);
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->type;
            break;
        case CMD_LOOK_LINK:
            *pc = mod(*pc + 1, GENOME_SIZE);
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            mask = (uint8_t)1 << cell->dir;
            
            cell->acc = 0;
            if(itself->links &= mask)
                cell->acc = 1;
            break;
        case CMD_DETACH:
            *pc = mod(*pc + 1, GENOME_SIZE);
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            mask = (uint8_t)1 << cell->dir;
            itself->links &= (uint8_t)~mask;
            
            mask = (uint8_t)1 << mod(cell->dir + 4, 8);
            neighbor->links &= (uint8_t)~mask;
            break;
        
        default:
            *pc = mod(*pc + 1, GENOME_SIZE);
            break;
        }
        
        if(print) printf("acc %3d\n", cell->acc);
    }
    
    // if(move) Rec_Push(x, y, dx, dy, 10, 1);
    
    if(itself->energy == 0) Cell_Destroy(id);
}

int16_t Find_Tag(Genome *genome, uint8_t tag)
{
    if(debug_life) printf("Find_Tag\n");
    
    int16_t best_pos = -1;
    uint8_t best_match = 0;
    Gene *gene;
    uint8_t diff, same_bits;
    
    for (uint8_t i = 0; i < GENOME_SIZE; i++) {
        gene = &genome->genes[i];

        if (gene->cmd != CMD_LABEL) {
            continue;
        }
        
        diff = (uint8_t)(gene->arg ^ tag);
        same_bits = (uint8_t)(8u - Count_Bits_8(diff));

        if (same_bits < 6u) {
            continue;
        }

        if (best_pos < 0 || same_bits > best_match) {
            best_pos = (int16_t)i;
            best_match = same_bits;
        }
    }

    return best_pos;
}