#include "life.h"

Cell cells[MAX_CELLS];
Genome genomes[MAX_GENOMES];

uint32_t free_id;
uint16_t free_g_id;

uint16_t mutation_rate = 200;
uint16_t mutation_max = 1000;
uint8_t starting_matter = 32;
uint8_t starting_energy = 1;
uint8_t req_matter = 1;
uint8_t req_energy = 127;

uint8_t debug_life = 0;
uint8_t dynamic_rules = 1;
uint32_t max_lifetime = 20000;
uint32_t max_population = 3000;
uint16_t pop_perc = 50;
uint16_t pop_threshold = 50;
uint32_t A;
uint32_t B;
uint8_t force_mult = 0;
uint8_t repopulate = 1;

uint32_t next_id;
uint32_t population_size;
uint32_t lifetime = 128;
uint8_t eat_div = 1;
uint8_t life = 1;

uint8_t force_mult_mode = 0;

void Cells_Init()
{
    for(uint32_t id = 0; id < MAX_CELLS; id++)
    {
        cells[id].x = 0;
        cells[id].y = 0;
        cells[id].buf_energy = 0;
        cells[id].buf_matter = 0;
        cells[id].outlet = 0;
        cells[id].dir = 8;
        cells[id].photo = 0;
        cells[id].prev = 0;
        cells[id].next = 0;
        cells[id].parent = 0;
        cells[id].used = 0;
        cells[id].g_id = 0;
        cells[id].pc = 0;
        cells[id].acc = 0;
        Stack_Reset(&cells[id].call_stack);
        Stack_Reset(&cells[id].data_stack);
    }
    
    B = MAX_CELLS / max_population;
    A = max_population / (1 + max_lifetime / B);
    
    free_id = 1;
    population_size = 0;
}

void Cells_Update()
{
    if(debug_life) fprintf(stderr, "Cells_Update\n"), fflush(stderr);
    
    uint32_t id = 0;
    next_id = 0;
    do
    {
        next_id = cells[id].next;
        if(cells[id].used)
        {
            Cell_Exec(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    
    id = 0;
    next_id = 0;
    do
    {
        next_id = cells[id].next;
        if(cells[id].used)
        {
            Redist_Energy(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    id = 0;
    next_id = 0;
    do
    {
        next_id = cells[id].next;
        if(cells[id].used)
        {
            Cell_Buf_Upd(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    if(population_size < pop_threshold)
    {
        if((population_size == 0 || force_mult == 0)
        && repopulate) 
        {
            Life_Reset();
            Populate(pop_perc);
        }
        else if(force_mult == 1)
        {
            Grid_Reset(0, 100);
            force_mult_mode = 1;
        }
    }
    
    if(force_mult)
    {
        if(force_mult_mode == 1)
        {
            Force_Multiply();
            if(population_size > 50000) force_mult_mode = 0;
        }
        printf("population_size %6d force_mult_mode %d\n", population_size, force_mult_mode);
    }
    if(dynamic_rules)
    {
        lifetime = max(MAX_CELLS / (population_size + A) - B, 1);
        // lifetime = max(max_lifetime - population_size * max_lifetime / max_population, 1);
        mutation_rate = min(fast_root(max(mutation_max * mutation_max * lifetime / population_size, 1)), 300);
        printf("population_size %6d mutation_rate %3d lifetime %4d\n", population_size, mutation_rate, lifetime);
    }
}

uint32_t Find_Free_Id()
{
    if(debug_life) fprintf(stderr, "Find_Free_Id\n"), fflush(stderr);
    
    uint32_t counter = 0;
    while((cells[free_id].used != 0 || free_id == 0)
    && counter < MAX_CELLS)
    {
        free_id = mod(free_id + 1, MAX_CELLS);
        counter++;
    }
    
    if(cells[free_id].used == 0) return free_id;
    else 
    {
        printf("no free cells\n");
        return 0;
    }
}

void Cell_Create(int16_t x, int16_t y, uint32_t parent, uint8_t photo, uint8_t out_in)
{
    if(debug_life) fprintf(stderr, "Cell_Create\n"), fflush(stderr);
    
    uint32_t id = Find_Free_Id();

    if(id == 0) return;
    
    if(parent == 0)
    {
        cells[id].g_id = Genome_Create(parent);
        if(cells[id].g_id == 0)
        {
            return;
        }
        Genome_Copy(cells[id].g_id, 0, mutation_max);
    }
    else if(rnd() % mutation_max < mutation_rate)
    {
        cells[id].g_id = Genome_Create(parent);
        if(cells[id].g_id == 0)
        {
            return;
        }
    }
    else
    {
        cells[id].g_id = cells[parent].g_id;
        genomes[cells[id].g_id].used++;
    }
    
    Tile *tile = Grid_Get(x, y);
    if(tile->type != 0) return;
    
    Grid_Set(x, y, id, 1);
    
    Tile *par_tile = Grid_Get(cells[parent].x, cells[parent].y);
    Cell *par_cell = &cells[parent];
    Cell *new_cell = &cells[id];
    
    if(parent == 0)
    {
        tile->matter = starting_matter;
        tile->energy = starting_energy;
    }
    else
    {
        tile->matter = req_matter;
        tile->energy = par_tile->energy / 2;
        par_tile->matter -= 1 + req_matter;
        par_tile->energy -= par_tile->energy / 2;
    }
    
    cells[id].x = mod(x, grid_width);
    cells[id].y = mod(y, grid_height);
    cells[id].buf_energy = 0;
    cells[id].buf_matter = 0;
    cells[id].outlet = 0;
    cells[id].dir = cells[parent].dir;
    cells[id].photo = photo;
    cells[id].prev = cells[parent].prev;
    cells[id].next = parent;
    cells[id].parent = parent;
    cells[id].pc = 0;
    cells[id].used = 1;
    cells[id].acc = 0;
    Stack_Reset(&cells[id].call_stack);
    Stack_Reset(&cells[id].data_stack);
    
    cells[cells[id].prev].next = id;
    cells[parent].prev = id;
    
    population_size++;
    
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
    if(out_in == 0)
    {
        par_cell->outlet |= mask_dir;
    }
    else
    {
        new_cell->outlet |= mask_opp;
    }
}

void Cell_Destroy(uint32_t id)
{
    if(debug_life) fprintf(stderr, "Cell_Destroy\n"), fflush(stderr);
    
    if(id == 0) return;
    
    if(cells[id].used) population_size--;
    
    cells[cells[id].prev].next = cells[id].next;
    cells[cells[id].next].prev = cells[id].prev;
    
    Tile *tile = Grid_Get(cells[id].x, cells[id].y);
    Tile *neighbor;
    tile->id = 0;
    tile->links = 0;
    
    int16_t x, y;
    uint8_t mask;
    for(int dir = 0; dir < 8; dir++)
    {
        x = cells[id].x + dir_to_coords[dir][0];
        y = cells[id].y + dir_to_coords[dir][1];
        neighbor = Grid_Get(x, y);
        mask = (uint8_t)1 << mod(dir + 4, 8);
        
        neighbor->links &= (uint8_t)~mask;
        cells[neighbor->id].outlet &= (uint8_t)~mask;
    }
    
    genomes[cells[id].g_id].used--;
    
    if(genomes[cells[id].g_id].used == 0) Genome_Destroy(cells[id].g_id);
    
    tile->energy += cells[id].buf_energy;
    tile->matter += cells[id].buf_matter;
    
    cells[id].x = 0;
    cells[id].y = 0;
    cells[id].buf_energy = 0;
    cells[id].buf_matter = 0;
    cells[id].outlet = 0;
    cells[id].dir = 8;
    cells[id].photo = 0;
    cells[id].prev = 0;
    cells[id].next = 0;
    cells[id].parent = 0;
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
    if(debug_life) fprintf(stderr, "Find_Free_Genome_Id\n"), fflush(stderr);
    
    uint16_t counter = 0;
    while((genomes[free_g_id].used != 0 || free_g_id == 0)
    && counter < MAX_GENOMES)
    {
        free_g_id = mod(free_g_id + 1, MAX_GENOMES);
        counter++;
    }
    
    // printf("free_g_id %d\n", free_g_id);
    
    if(genomes[free_g_id].used == 0) return free_g_id;
    else 
    {
        // printf("no free genomes\n");
        return 0;
    }
}

uint16_t Genome_Create(uint16_t par_id)
{
    if(debug_life) fprintf(stderr, "Genome_Create\n"), fflush(stderr);
    
    uint16_t g_id = Find_Free_Genome_Id();
    uint16_t par_g_id = cells[par_id].g_id;
    
    if(g_id == 0) return 0;
    
    genomes[g_id].used = 1;
    
    Genome_Copy(g_id, par_g_id, mutation_rate);
    
    return g_id;
}

void Genome_Destroy(uint16_t g_id)
{
    if(debug_life) fprintf(stderr, "Genome_Destroy\n"), fflush(stderr);
    
    if(g_id == 0) return;
    
    genomes[g_id].used = 0;
    
    free_g_id = g_id;
}

void Genome_Copy(uint16_t g_id_to, uint16_t g_id_from, uint16_t mut_rate)
{
    if(debug_life) fprintf(stderr, "Genome_Copy to %d from %d\n", g_id_to, g_id_from), fflush(stderr);
    
    for(uint8_t g = 0; g < GENOME_SIZE; g++)
    {
        genomes[g_id_to].genes[g].cmd = genomes[g_id_from].genes[g].cmd;
        genomes[g_id_to].genes[g].arg = genomes[g_id_from].genes[g].arg;
        
        if(rnd() % mutation_max < mut_rate) genomes[g_id_to].genes[g].cmd = rnd() % CMD_COUNT;
        if(rnd() % mutation_max < mut_rate) genomes[g_id_to].genes[g].arg = rnd() % GENOME_SIZE;
    }
}

void Cell_Exec(uint32_t id)
{
    if(debug_life) fprintf(stderr, "Cell_Exec\n"), fflush(stderr);
    
    uint8_t print = 0;
    
    if(id == 0) return;
    if(cells[id].g_id == 0) return;
    
    if(print) fprintf(stderr, "Cell_Exec id %d\n", id);
    
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
    uint8_t eat_amount = 255;
    uint8_t move_strength = 2;
    
    int16_t x = cell->x;
    int16_t y = cell->y;
    int16_t dx = dir_to_coords[cell->dir][0];
    int16_t dy = dir_to_coords[cell->dir][1];
    
    int8_t photo_threshold = -1;
    uint8_t how_open = Is_Membrane(x, y);
    int16_t new_energy;
    
    if(life)
    {
        if(cell->photo == 0)
        {
            new_energy = itself->energy - 1 - how_open;
            itself->energy = max(new_energy, 0);
        }
        if(cell->photo == 1)
        {
            new_energy = itself->energy + 1 + how_open;
            itself->energy = min(new_energy, 255);
        }
    }
    
    for(int steps = MAX_STEPS * (1 - life); steps < MAX_STEPS; steps++)
    {
        gene = &genome->genes[*pc];
        
        x = cell->x;
        y = cell->y;
        dx = dir_to_coords[cell->dir][0];
        dy = dir_to_coords[cell->dir][1];
        neighbor = Grid_Get(cell->x + dx, cell->y + dy);
        itself = Grid_Get(cell->x, cell->y);
        
        if(print) fprintf(stderr, "pc %2d cmd %3d arg %3d\n", *pc, gene->cmd, gene->arg), fflush(stderr);
        
        *pc = mod(*pc + 1, GENOME_SIZE);
        switch (gene->cmd)
        {
        case CMD_NO_OP:
            
            break;
        case CMD_LABEL:
            
            break;
        case CMD_EXEC:
            pos = Find_Tag(genome, cell->acc);
            
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
            break;
        case CMD_PUSH_IMM:
            Stack_Push(&cell->data_stack, gene->arg);
            break;
        case CMD_PUSH_ACC:
            Stack_Push(&cell->data_stack, cell->acc);
            break;
        case CMD_POP:
            Stack_Pop(&cell->data_stack, &cell->acc);
            break;
        case CMD_ADD_POP:
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = cell->acc + read;
                cell->acc = min(temp, 255);
            }
            break;
        case CMD_SUB_POP:
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = cell->acc - read;
                cell->acc = max(temp, 0);
            }
            break;
        case CMD_CMP_POP:
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = (cell->acc > read);
                cell->acc = temp;
            }
            break;
        case CMD_MULTIPLY:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(dx != 0 && dy != 0
            && rand() % 1000 > 707) break; 
            
            if(cell->dir != 8)
            {
                cell->acc = 0;
                
                if(itself->matter >= req_matter + 1
                && itself->energy >= 2)
                {
                    if(neighbor->matter == 0
                    && neighbor->energy == 0
                    && neighbor->type == 0)
                    {
                        Cell_Create(x + dx, y + dy, id, (gene->arg >> 0) & 1, (gene->arg >> 1) & 1);
                        cell->acc += 1;
                    }
                }
            }
            break;
        case CMD_ROT:
            if(cell->dir == 8)
                cell->dir = rand() % 8;
            temp = cell->dir + gene->arg;
            cell->dir = mod(temp, 8);
            break;
        case CMD_CENTRE:
            cell->dir = 8;
            break;
        case CMD_MOVE:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
        
            temp = cell->photo ? 1 : 1 + gene->arg / move_strength;
            
            if(dx != 0 && dy != 0
            && rand() % 1000 > 707) break; 
            
            if(cell->dir != 8// && itself->energy > temp
            )
            {
                // itself->energy -= temp;
                cell->acc = 0;
                if(Rec_Push(cell->x, cell->y, dx, dy, temp, 0))
                {
                    cell->acc = 1;
                }
            }
            break;
        case CMD_EAT:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            read = cell->photo ? eat_amount : eat_amount;
            
            if(cells[neighbor->id].photo) read /= 16;
            
            if(dx != 0 && dy != 0
            && rand() % 1000 > 707) break; 
            
            cell->acc = 0;
            if(neighbor->type != 0 && neighbor != itself
            && (Count_Bits_8(neighbor->links) <= Count_Bits_8(itself->links)
            || neighbor->id == 0)
            // && (neighbor->matter < itself->matter
            // || neighbor->id == 0)
            // || neighbor->energy < itself->energy)
            )
            {
                if(neighbor->id != 0
                )
                {
                    Cell_Destroy(neighbor->id);
                    next_id = cell->next;
                }
                // break;
                
                if(neighbor->matter >= read && itself->matter < 256 - read)
                {
                    itself->matter += read;
                    neighbor->matter -= read;
                    cell->acc += 1;
                }
                else if(itself->matter + neighbor->matter > 255)
                {
                    neighbor->matter -= 255 - itself->matter;
                    itself->matter = 255;
                }
                else
                {
                    itself->matter += neighbor->matter;
                    neighbor->matter = 0;
                }
                
                if(cell->photo == 0)
                {
                    if(neighbor->energy >= read && itself->energy < 255 - read / eat_div)
                    {
                        itself->energy += read / eat_div;
                        neighbor->energy -= read;
                        cell->acc += 1;
                    }
                    else if(itself->energy + neighbor->energy / eat_div > 254)
                    {
                        neighbor->energy -= (254 - itself->energy) * eat_div;
                        itself->energy = 254;
                    }
                    else 
                    {
                        itself->energy += neighbor->energy / eat_div;
                        neighbor->energy = 0;
                    }
                }
                
                temp = itself->matter + 1;
                if(neighbor->matter == 0 && neighbor->energy == 0
                && neighbor->type != 0
                && temp < 256)
                {
                    if(neighbor->id != 0 && cells[neighbor->id].used)
                    {
                        Cell_Destroy(neighbor->id);
                        next_id = cell->next;
                    }
                    Grid_Set(x + dx, y + dy, 0, 0);
                    itself->matter = temp;
                    cell->acc += 1;
                }
            }
            break;
        case CMD_LOOK_TYPE:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->type;
            break;
        case CMD_LOOK_LINK:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = 0;
            if(cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                
                if(itself->links & mask)
                    cell->acc = 1;
            }
            break;
        case CMD_LOOK_MEMB:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = Is_Membrane(cell->x + dx, cell->y + dy);
            break;
        case CMD_LOOK_MAT:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->matter;
            break;
        case CMD_LOOK_NRG:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->energy;
            break;
        case CMD_LOOK_ACC:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = cells[neighbor->id].acc;
            break;
        case CMD_DETACH:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                if(itself->links & mask)
                    itself->links &= (uint8_t)~mask;
                mask = (uint8_t)1 << mod(cell->dir + 4, 8);
                if(neighbor->links & mask)
                {
                    neighbor->links &= (uint8_t)~mask;
                }
                if(cell->outlet & mask)
                    cell->outlet &= (uint8_t)~mask;
            }
            break;
        case CMD_ATTACH:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(neighbor->type != 0 && cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                itself->links |= mask;
                
                mask = (uint8_t)1 << mod(cell->dir + 4, 8);
                neighbor->links |= mask;
            }
            break;
        case CMD_OUTLET_OFF:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                if(cell->outlet & mask)
                    cell->outlet &= (uint8_t)~mask;
            }
            break;
        case CMD_OUTLET_ON:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            temp = gene->arg % 2;
            
            if(neighbor->id != 0 && cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                itself->links |= mask;
                
                mask = (uint8_t)1 << mod(cell->dir + 4, 8);
                neighbor->links |= mask;
            
                mask = (uint8_t)1 << cell->dir;
                cell->outlet |= mask;
            }
            break;
        case CMD_SET_PHERO:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            Phero_Set(cell->x, cell->y, gene->arg * MAX_PHEROMONES / 255, cell->acc);
            break;
        case CMD_GET_PHERO:
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = Phero_Get(cell->x + dx, cell->y + dy, gene->arg * MAX_PHEROMONES / 255, 0);
            break;
        
        default:
            break;
        }
        
        x = cell->x;
        y = cell->y;
        dx = dir_to_coords[cell->dir][0];
        dy = dir_to_coords[cell->dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        itself = Grid_Get(x, y);
        
        // if(itself->energy == 0 
        // || itself->energy == 255
        // ) Cell_Destroy(id), steps = MAX_STEPS;
        
        if(print) fprintf(stderr, "acc %3d\n", cell->acc);
    }
}

void Cell_Buf_Upd(uint32_t id)
{
    if(debug_life) fprintf(stderr, "Cell_Buf_Upd\n"), fflush(stderr);
    
    uint8_t print = 0;
    
    if(id == 0) return;
    if(cells[id].g_id == 0) return;
    
    if(print) fprintf(stderr, "Cell_Buf_Upd id %d\n", id);
    
    Cell *cell = &cells[id];
    Tile *itself = Grid_Get(cell->x, cell->y);
    
    itself->matter += cell->buf_matter;
    itself->energy += cell->buf_energy;
    cell->buf_matter = 0;
    cell->buf_energy = 0;
        
    if(rand() % lifetime == 0)
    {
        if(itself->energy == 0 
        || itself->energy == 255
        ) Cell_Destroy(id);
    }
}

void Redist_Energy(uint32_t id)
{
    Cell *cell = &cells[id];
    Cell *cell_n;
    Tile *itself = Grid_Get(cell->x, cell->y);
    Tile *neighbor;
    uint8_t mask;
    int16_t x, y, dx, dy;
    int16_t ediff, s_ediff;
    int16_t mdiff, s_mdiff;
    uint8_t neighbor_amount = 0;
    int16_t desired_energy = 0;
    int16_t desired_matter = 0;
    int16_t spread_energy = 0;
    int16_t spread_matter = 0;
    
    int16_t res_energy = 0;
    int16_t res_matter = 0;
    x = cell->x, y = cell->y;
    
    if(Count_Bits_8(itself->links) == 0) return;
    
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        mask = (uint8_t)1 << dir;
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        cell_n = &cells[neighbor->id];
        
        if(cell->outlet & mask
        && neighbor->id != 0)
        {
            neighbor_amount++;
        }
    }
    
    {
        desired_energy = -cell->buf_energy;
        desired_matter = -cell->buf_matter;
    }
    
    if(neighbor_amount == 0) return;
    
    spread_energy = desired_energy / neighbor_amount;
    spread_matter = desired_matter / neighbor_amount;
    
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        mask = (uint8_t)1 << dir;
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        cell_n = &cells[neighbor->id];
        
        if(cell->outlet & mask
        && neighbor->id != 0)
        {
            res_energy = cell_n->buf_energy - spread_energy;
            res_matter = cell_n->buf_matter - spread_matter;
            
            ediff = spread_energy;
            mdiff = spread_matter;
            
            if(res_energy < 1)
            {
                ediff = cell_n->buf_energy - 1;
                res_energy = 1;
            }
            if(res_energy > 254)
            {
                ediff = cell_n->buf_energy - 254;
                res_energy = 254;
            }
            if(res_matter < 0)
            {
                mdiff = cell_n->buf_matter;
                res_matter = 0;
            }
            if(res_matter > 255)
            {
                mdiff = cell_n->buf_matter - 255;
                res_matter = 255;
            }
            
            s_ediff = (ediff);
            s_mdiff = (mdiff);
            
            if(life)
            {
                cell->buf_energy += s_ediff;
                neighbor->energy -= s_ediff;
                cell->buf_matter += s_mdiff;
                neighbor->matter -= s_mdiff;
            }
        }
    }
}

int16_t Find_Tag(Genome *genome, uint8_t tag)
{
    if(debug_life) fprintf(stderr, "Find_Tag\n"), fflush(stderr);
    
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

void Populate(int n)
{
    Tile *tile;
    for(int y = 0; y < grid_height; y++)
    {
        for(int x = 0; x < grid_width; x++)
        {
            if(rand() % 10000 < n// && rand() % grid_height < y
            )
            {
                if(Find_Free_Genome_Id() != 0)
                {
                    tile = Grid_Get(x, y);
                    if(cells[tile->id].used && tile->id)
                    {
                        Cell_Destroy(tile->id);
                    }
                    Grid_Set(x, y, 0, 0);
                    Cell_Create(x, y, 0, rand() % 2, 0);
                }
            }
        }
    }
}

void Force_Multiply()
{
    Tile *neighbor, *itself;
    uint32_t id = 0;
    next_id = 0;
    int dx;
    int dy;
    do
    {
        next_id = cells[id].next;
        if(cells[id].used)
        {
            dx = dir_to_coords[cells[id].dir][0];
            dy = dir_to_coords[cells[id].dir][1];
            itself = Grid_Get(cells[id].x, cells[id].y);
            neighbor = Grid_Get(cells[id].x + dx, cells[id].y + dy);
            if(itself->matter > req_matter + 1
            && itself->energy >= 2
            && neighbor->matter == 0
            && neighbor->energy == 0
            && neighbor->type == 0
            )
                Cell_Create(cells[id].x + dx, cells[id].y + dy, id, cells[id].photo, rand() % 2);
            else
            {
                Grid_Get(cells[id].x, cells[id].y)->matter = req_matter + 10;
                Grid_Get(cells[id].x, cells[id].y)->energy = req_energy + 10;
            }
        }
    
        id = next_id;
    }
    while(id != 0);
}

void Life_Reset()
{
    Tile *tile;
    for(int y = 0; y < grid_height; y++)
    {
        for(int x = 0; x < grid_width; x++)
        {
            tile = Grid_Get(x, y);
            if(cells[tile->id].used && tile->id != 0)
            {
                Cell_Destroy(tile->id);
            }
            Grid_Set(x, y, 0, 0);
        }
    }
}