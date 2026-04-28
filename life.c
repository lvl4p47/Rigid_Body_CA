#include "life.h"

Cell cells[MAX_CELLS];
Genome genomes[MAX_GENOMES];

uint32_t free_id;
uint16_t free_g_id;
uint16_t last_plant = 0;
uint16_t last_animal = 0;
uint8_t save_them = 0;

uint32_t followed_id = 1;

uint16_t mutation_rate = 16;
uint16_t mutation_max = 1000;
uint8_t starting_matter = 3;
uint8_t starting_energy = 127;
uint8_t req_matter = 0;
uint8_t req_energy = 127;
uint8_t max_matter = 3;
uint16_t soil = 300;

uint8_t debug_life = 01;

uint8_t dynamic_rules = 0;
uint32_t max_lifetime = 100;
uint32_t max_population = 500;
uint32_t A;
uint32_t B;

uint8_t repopulate = 01;
uint16_t pop_perc = 1000;
uint16_t pop_threshold = 1;

uint8_t force_mult = 0;

uint32_t next_id;
uint32_t population_size;
uint32_t plant_pop;
uint32_t animal_pop;
uint32_t lifetime = GENOME_SIZE;
uint8_t eat_div = 1;
uint8_t life = 01;
uint8_t nat_death = 1;

uint8_t gravity = 01;
uint16_t grav_period = 1;
uint32_t grav_rate = 150;
uint32_t max_strength;

uint8_t lighting = 01;
uint16_t lighting_period = 1;
uint16_t max_light_strength;

uint8_t max_light = 255;
uint8_t sun_light = 0;
uint32_t day_length = GENOME_SIZE * 8;
uint8_t night_depth = 255;

uint8_t track_energy = 0;
int32_t energy_gain;
int32_t energy_loss;
int32_t energy_change;
uint32_t total_energy_acc = 0;

uint8_t birth_control = 0;
uint32_t born;
uint32_t birth_debt = 0;
uint32_t max_birth_debt = 10000;
uint16_t born_desired = 1;
uint32_t birth_delta;

uint8_t force_mult_mode = 0;

uint8_t push_away = 01;

uint8_t max_links = 3;

uint8_t sudden_death = 0;

FILE *file_ptr;
uint8_t integer;

void Cells_Init()
{
    for(uint32_t id = 0; id < MAX_CELLS; id++)
    {
        cells[id].x = 0;
        cells[id].y = 0;
        cells[id].buf_energy = 0;
        cells[id].buf_matter = 0;
        cells[id].energy_out = 0;
        cells[id].matter_out = 0;
        cells[id].dir = 8;
        cells[id].photo = 0;
        cells[id].prev = 0;
        cells[id].next = 0;
        cells[id].parent = 0;
        cells[id].used = 0;
        cells[id].active = 0;
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
    plant_pop = 0;
    animal_pop = 0;
    birth_debt = 0;
    grav_rate = grid_width * grid_height / 100;
    max_strength = min(grid_height * grid_width, 100);
}

void Cells_Update()
{
    born = 0;
    
    if(debug_life) fprintf(stderr, "Cells_Update\n"), fflush(stderr);
    
    uint32_t id = 0;
    next_id = 0;
    do
    {
        next_id = cells[id].next;
        if(cells[id].used && cells[id].active
        )
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
        if(cells[id].used
        )
        {
            Cell_Buf_Upd(id);
        }
    
        id = next_id;
    }
    while(id != 0);
    
    if(birth_control)
    {
        birth_delta = born_desired - born;
        birth_debt = max(birth_debt + birth_delta, 0);
        // lifetime = min(max(lifetime + birth_delta, 1), 1000);
        // lifetime = max(max_lifetime - population_size * max_lifetime / max_population, 1);
        // printf("birth_debt %d\tlifetime %d\t population %d\n", birth_debt, lifetime, population_size);
    }
    if(birth_debt > max_birth_debt && birth_control
    || population_size < pop_threshold && repopulate)
    {
        if(population_size < pop_threshold && repopulate && !sudden_death)
        {
            save_them = 1;
            Life_Reset(1000);
            Grid_Reset(0, 1000);
            Grid_Reset_Half(1, soil);
            Reanimate(pop_perc);
            birth_debt = 0;
            save_them = 0;
            
            file_ptr = fopen("followed_exec.txt", "w");
            fclose(file_ptr);
        }
        // else if(birth_control)
        // {
        //     Life_Reset(1000);
        //     Grid_Reset(0, 1000);
        //     Grid_Reset_Half(1, soil);
        //     Reanimate(pop_perc);
        //     birth_debt = 0;
        // }
    }
    
    Gravity();
    
    if(force_mult)
    {
        if(force_mult_mode == 1)
        {
            Force_Multiply();
            if(population_size > max_population) force_mult_mode = 0;
        }
        // printf("population_size %6d force_mult_mode %d\n", population_size, force_mult_mode);
    }
    if(repopulate && dynamic_rules)
    {
        // lifetime = max(MAX_CELLS / (population_size + A) - B, 1);
        lifetime = max(max_lifetime - population_size * max_lifetime / max_population, 1);
        // mutation_rate = min(fast_root(max((uint32_t)(mutation_max * mutation_max / population_size * lifetime ), 1)), 62);
        // printf("population_size %6d mutation_rate %5d lifetime %4d\n", population_size, mutation_rate, lifetime);
    }
    max_birth_debt = max(population_size / 10, 100);
    energy_change = energy_gain + energy_loss;
    total_energy_acc += energy_change;
    if(track_energy)
    {
        printf("gen %4d lost %4d sum %4d total %d\n", energy_gain, energy_loss, energy_change, total_energy_acc);
    }
    energy_gain = 0;
    energy_loss = 0;
    
    if(sudden_death && population_size == 0) quit = 1;
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
    
    if(cells[followed_id].active == 0 || followed_id == 0) followed_id = id;
    
    if(parent == 0)
    {
        cells[id].g_id = Genome_Upload_Best(photo);
        if(cells[id].g_id == 0)
        {
            return;
        }
        // Genome_Copy(cells[id].g_id, cells[id].g_id, mutation_rate);
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
        energy_gain += starting_energy;
        if(debug_life) fprintf(stderr, "Cell_Create, energy gain\n");
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
    cells[id].energy_out = 0;
    cells[id].matter_out = 0;
    cells[id].dir = cells[parent].dir;
    cells[id].photo = photo;
    cells[id].prev = cells[parent].prev;
    cells[id].next = parent;
    cells[id].parent = parent;
    cells[id].pc = 0;
    cells[id].used = 1;
    cells[id].active = 1;
    cells[id].acc = 0;
    Stack_Reset(&cells[id].call_stack);
    Stack_Reset(&cells[id].data_stack);
    
    cells[cells[id].prev].next = id;
    cells[parent].prev = id;
    
    population_size++;
    if(photo) plant_pop++;
    else animal_pop++;
    born++;
    
    if(parent == 0) return;
    
    uint8_t dir = cells[id].dir;
    uint8_t opp = mod(dir + 4, 8);
    int16_t dx = dir_to_coords[opp][0];
    int16_t dy = dir_to_coords[opp][1];
    Tile *new = Grid_Get(x, y);
    Tile *par = Grid_Get(cells[parent].x, cells[parent].y);
    
    uint8_t mask_dir = (uint8_t)1 << dir;
    uint8_t mask_opp = (uint8_t)1 << opp;
    
    if(Count_Bits_8(par->links) < max_links
    && Count_Bits_8(new->links) < max_links
    && photo == par_cell->photo)
    {
        par->links |= mask_dir;
        new->links |= mask_opp;
        if(out_in == 0)
        {
            par_cell->energy_out |= mask_dir;
            new_cell->matter_out |= mask_opp;
        }
        else
        {
            par_cell->matter_out |= mask_dir;
            new_cell->energy_out |= mask_opp;
        }
    }
}

void Cell_Destroy(uint32_t id)
{
    if(debug_life) fprintf(stderr, "Cell_Destroy\n"), fflush(stderr);
    
    if(id == 0) return;
    
    if(cells[id].used && cells[id].active == 1)
    {
        population_size--;
        
        if(cells[id].photo)
        {
            last_plant = cells[id].g_id;
            plant_pop--;
        }
        else
        {
            last_animal = cells[id].g_id;
            animal_pop--;
        }
    }
    
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
        cells[neighbor->id].energy_out &= (uint8_t)~mask;
        cells[neighbor->id].matter_out &= (uint8_t)~mask;
    }
    
    genomes[cells[id].g_id].used--;
    
    if(genomes[cells[id].g_id].used == 0) Genome_Destroy(cells[id].g_id);
    
    tile->energy += cells[id].buf_energy;
    tile->matter += cells[id].buf_matter;
    
    cells[id].x = 0;
    cells[id].y = 0;
    cells[id].buf_energy = 0;
    cells[id].buf_matter = 0;
    cells[id].energy_out = 0;
    cells[id].matter_out = 0;
    cells[id].dir = 8;
    cells[id].photo = 0;
    cells[id].prev = 0;
    cells[id].next = 0;
    cells[id].parent = 0;
    cells[id].used = 0;
    cells[id].active = 0;
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

uint16_t Genome_Upload_Best(uint8_t animal_or_plant)
{
    uint16_t g_id = Find_Free_Genome_Id();
    // printf("found g_id %d\n", g_id);
    if(g_id == 0) return 0;
    
    genomes[g_id].used = 1;

    char buf[32];
    if(animal_or_plant)
        snprintf(buf, sizeof(buf), "genomes/best_plant_genome.txt");
    else
        snprintf(buf, sizeof(buf), "genomes/best_animal_genome.txt");
    
    file_ptr = fopen(buf, "r");
    if(file_ptr == NULL)
    {
        // printf("\nBEST_GENOME NOT FOUND\n");
        for(uint8_t g = 0; g < GENOME_SIZE; g++)
        {
            genomes[g_id].genes[g].cmd = rnd() % CMD_COUNT;
            genomes[g_id].genes[g].arg = rnd() % GENOME_SIZE;
        }
    }
    else
    {
        // printf("\nBEST_GENOME FOUND\n");
        for(uint8_t g = 0; g < GENOME_SIZE; g++)
        {
            fscanf(file_ptr, "%i", &integer);
            genomes[g_id].genes[g].cmd = integer;
            
            fscanf(file_ptr, "%i", &integer);
            genomes[g_id].genes[g].arg = integer;
        }
        fclose(file_ptr);
    }
    
    return g_id;
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
    
    if(Genome_Copy(g_id, par_g_id, mutation_rate) == 0) 
    {
        Genome_Destroy(g_id);
        g_id = par_id;
    }
    
    return g_id;
}

void Genome_Destroy(uint16_t g_id)
{
    if(debug_life) fprintf(stderr, "Genome_Destroy\n"), fflush(stderr);
    
    if(g_id == 0) return;
    
    genomes[g_id].used = 0;
    
    if(save_them)
    {
        if(g_id == last_plant && plant_pop == 0)
        {
            Genome_Download_Best(g_id, 1);
        }
        if(g_id == last_animal && animal_pop == 0)
        {
            Genome_Download_Best(g_id, 0);
        }
    }
    
    free_g_id = g_id;
}

void Genome_Download_Best(uint16_t g_id, uint8_t animal_or_plant)
{
    char buf[32];
    if(animal_or_plant)
    {
        printf("the best plant g_id %d\n", g_id);
        snprintf(buf, sizeof(buf), "genomes/best_plant_genome.txt");
        last_plant = 0;
    }
    else
    {
        printf("the best animal g_id %d\n", g_id);
        snprintf(buf, sizeof(buf), "genomes/best_animal_genome.txt");
        last_animal = 0;
    }
    
    file_ptr = NULL;
    file_ptr = fopen(buf, "w");

    for(int i = 0; i < GENOME_SIZE; i++)
    {
        fprintf(file_ptr, "%i\t%i\n", genomes[g_id].genes[i].cmd, genomes[g_id].genes[i].arg);
        
    }
    fclose(file_ptr);
}

uint8_t Genome_Copy(uint16_t g_id_to, uint16_t g_id_from, uint16_t mut_rate)
{
    if(debug_life) fprintf(stderr, "Genome_Copy to %d from %d\n", g_id_to, g_id_from), fflush(stderr);
    uint8_t is_dif = 0;
    
    for(uint8_t g = 0; g < GENOME_SIZE; g++)
    {
        genomes[g_id_to].genes[g].cmd = genomes[g_id_from].genes[g].cmd;
        genomes[g_id_to].genes[g].arg = genomes[g_id_from].genes[g].arg;
        
        if(rnd() % mutation_max < mut_rate) 
        {
            genomes[g_id_to].genes[g].cmd = rnd() % CMD_COUNT;
            if(genomes[g_id_to].genes[g].cmd != genomes[g_id_from].genes[g].cmd) is_dif = 1;
        }
        if(rnd() % mutation_max < mut_rate)
        {
            genomes[g_id_to].genes[g].arg = rnd() % GENOME_SIZE;
            if(genomes[g_id_to].genes[g].arg != genomes[g_id_from].genes[g].arg) is_dif = 1;
        }
    }
    return is_dif;
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
    uint32_t moved;
    Tile *neighbor;
    Tile *itself = Grid_Get(cell->x, cell->y);
    uint8_t mask;
    uint8_t eat_amount = 255;
    uint8_t move_strength = 32;
    uint8_t eaten_matter, eaten_energy;
    
    int16_t x = cell->x;
    int16_t y = cell->y;
    int16_t dx = dir_to_coords[cell->dir][0];
    int16_t dy = dir_to_coords[cell->dir][1];
    
    
    if(debug_life) fprintf(stderr, "Cell_Exec, energy changed\n");
    
    if(life == 0 || cell->active == 0) return;
    
    if(followed_id == id)
    {
        char buf[32];
        snprintf(buf, sizeof(buf), "followed_exec.txt");
        
        file_ptr = NULL;
        file_ptr = fopen(buf, "a");
        
        fprintf(file_ptr, "followed_id %d\n", followed_id);
    }
    
    for(int steps = 0; steps < MAX_STEPS; steps++)
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
            if(debug_life) fprintf(stderr, "CMD_NO_OP\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_NO_OP\n", *pc, gene->arg, cell->acc);
            break;
        case CMD_LABEL:
            if(debug_life) fprintf(stderr, "CMD_LABEL\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LABEL\n", *pc, gene->arg, cell->acc);
            break;
        case CMD_EXEC:
            if(debug_life) fprintf(stderr, "CMD_EXEC\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_EXEC\n", *pc, gene->arg, cell->acc);
            pos = Find_Tag(genome, cell->acc);
            
            if(pos >= 0)
            {
                if(Stack_Push(&cell->call_stack, *pc))
                    *pc = pos;
            }
            break;
        case CMD_RET:
            if(debug_life) fprintf(stderr, "CMD_RET\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_RET\n", *pc, gene->arg, cell->acc);
            if(Stack_Pop(&cell->call_stack, &read))
            {
                *pc = read % GENOME_SIZE;
            }
            break;
        case CMD_PUSH_IMM:
            if(debug_life) fprintf(stderr, "CMD_PUSH_IMM\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_PUSH_IMM\n", *pc, gene->arg, cell->acc);
            Stack_Push(&cell->data_stack, gene->arg);
            break;
        case CMD_PUSH_ACC:
            if(debug_life) fprintf(stderr, "CMD_PUSH_ACC\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_PUSH_ACC\n", *pc, gene->arg, cell->acc);
            Stack_Push(&cell->data_stack, cell->acc);
            break;
        case CMD_POP:
            if(debug_life) fprintf(stderr, "CMD_POP\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_POP\n", *pc, gene->arg, cell->acc);
            Stack_Pop(&cell->data_stack, &cell->acc);
            break;
        case CMD_ADD_POP:
            if(debug_life) fprintf(stderr, "CMD_ADD_POP\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_ADD_POP\n", *pc, gene->arg, cell->acc);
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = cell->acc + read;
                cell->acc = min(temp, 255);
            }
            break;
        case CMD_SUB_POP:
            if(debug_life) fprintf(stderr, "CMD_SUB_POP\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_SUB_POP\n", *pc, gene->arg, cell->acc);
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = cell->acc - read;
                cell->acc = max(temp, 0);
            }
            break;
        case CMD_CMP_POP:
            if(debug_life) fprintf(stderr, "CMD_CMP_POP\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_CMP_POP\n", *pc, gene->arg, cell->acc);
            if(Stack_Pop(&cell->data_stack, &read))
            {
                temp = (cell->acc > read);
                cell->acc = temp;
            }
            break;
        case CMD_SET_PTR:
            if(debug_life) fprintf(stderr, "CMD_SET_PTR\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_SET_PTR\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            mask = (uint8_t)1 << cell->dir;
            if(itself->links & mask || neighbor == itself)
                cells[neighbor->id].pc = mod(gene->arg, GENOME_SIZE);
            break;
        case CMD_MULTIPLY:
            if(debug_life) fprintf(stderr, "CMD_MULTIPLY\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_MULTIPLY\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(dx != 0 && dy != 0
            && rnd() % 1000 > 707) break; 
            
            if(cell->dir != 8)
            {
                cell->acc = 0;
                
                if(itself->matter + neighbor->matter + (neighbor->type == 1) >= req_matter + 1
                && itself->energy + neighbor->energy >= 2
                && itself->matter + neighbor->matter + (neighbor->type == 1) <= max_matter
                && itself->energy + neighbor->energy < 255
                && cells[neighbor->id].used == 0)
                {
                    if(neighbor->matter == 0
                    && neighbor->energy == 0
                    && neighbor->type == 0)
                    {
                        Cell_Create(x + dx, y + dy, id, (gene->arg >> 0) & 1, (gene->arg >> 1) & 1);
                        // Rec_Push_Attempt(x, y, -dx, -dy, max_strength, 0);
                        cell->acc = 255;
                    }
                    else if(neighbor->type == 1)
                    {
                        itself->matter += neighbor->matter + (neighbor->type == 1);
                        neighbor->matter = 0;
                        neighbor->type = 0;
                        itself->energy += neighbor->energy;
                        neighbor->energy = 0;
                        
                        Cell_Create(x + dx, y + dy, id, (gene->arg >> 0) & 1, (gene->arg >> 1) & 1);
                        // Rec_Push_Attempt(x, y, -dx, -dy, max_strength, 0);
                        cell->acc = 255;
                    }
                }
            }
            break;
        case CMD_ROT:
            if(debug_life) fprintf(stderr, "CMD_ROT\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_ROT\n", *pc, gene->arg, cell->acc);
            if(cell->dir == 8)
                cell->dir = rnd() % 8;
            temp = cell->dir + gene->arg;
            cell->dir = mod(temp, 8);
            break;
        case CMD_CENTRE:
            if(debug_life) fprintf(stderr, "CMD_CENTRE\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_CENTRE\n", *pc, gene->arg, cell->acc);
            cell->dir = 8;
            break;
        case CMD_MOVE:
            steps = MAX_STEPS;
            if(debug_life) fprintf(stderr, "CMD_MOVE\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_MOVE\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
        
            temp = cell->photo ? 1 : 1 + cell->acc / move_strength;
            
            if(dx != 0 && dy != 0
            && rnd() % 1000 > 707) break; 
            
            if(cell->dir != 8)
            {
                cell->acc = 0;
                if(push_away)
                    moved = Rec_Push_Away(cell->x, cell->y, dx, dy, temp, 0);
                else
                    moved = Rec_Push(cell->x, cell->y, dx, dy, temp, 0);
                if(moved)
                {
                    cell->acc = 255;
                    
                    // old_energy = itself->energy;
                    // itself->energy = max(itself->energy - moved, 0);
                    
                    // if(itself->energy - old_energy > 0) energy_gain += itself->energy - old_energy;
                    // if(itself->energy - old_energy < 0) energy_loss += itself->energy - old_energy;
                }
            }
            
            break;
        case CMD_EAT:
            steps = MAX_STEPS;
            if(debug_life) fprintf(stderr, "CMD_EAT\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_EAT\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            read = cell->photo ? eat_amount : eat_amount;
            
            if(dx != 0 && dy != 0
            && rnd() % 1000 > 707) break; 
            
            cell->acc = 0;
            
            eaten_matter = 0;
            eaten_energy = 0;
            if(neighbor->type == 1 && neighbor != itself
            && (Count_Bits_8(neighbor->links) <= Count_Bits_8(itself->links)
            || neighbor->id == 0
            || cells[neighbor->id].active == 0)
            // && (neighbor->matter < itself->matter
            // || neighbor->id == 0)
            // || neighbor->energy < itself->energy)
            )
            {
                if(neighbor->id != 0// && cell->photo == 0
                )
                {
                    Cell_Destroy(neighbor->id);
                    next_id = cell->next;
                    break;
                }
                temp = itself->matter + cell->buf_matter + 1;
                if(neighbor->matter == 0 && neighbor->energy == 0
                && neighbor->type == 1)
                {
                    if(neighbor->id != 0 && cells[neighbor->id].used)
                    {
                        Cell_Destroy(neighbor->id);
                        next_id = cell->next;
                    }
                    else if(temp <= max_matter)
                    {
                        // printf("x %d y %d temp %d itself %d buf %d\n", x, y, temp, itself->matter, cell->buf_matter);
                        cell->buf_matter += 1;
                        Grid_Set(x + dx, y + dy, 0, 0);
                        eaten_matter += 1;
                    }
                    break;
                }
                
                
                if(neighbor->matter >= read && itself->matter + cell->buf_matter + read <= max_matter)
                {
                    eaten_matter += read;
                    cell->buf_matter += read;
                    neighbor->matter -= read;
                    cell->acc += 127;
                }
                else if(itself->matter + cell->buf_matter + neighbor->matter > max_matter)
                {
                    eaten_matter += max_matter - itself->matter - cell->buf_matter;;
                    neighbor->matter -= max_matter - itself->matter - cell->buf_matter;
                    cell->buf_matter += max_matter - itself->matter - cell->buf_matter;
                    cell->acc += 127;
                }
                else
                {
                    eaten_matter += neighbor->matter;
                    cell->buf_matter += neighbor->matter;
                    neighbor->matter = 0;
                    cell->acc += 127;
                }
                
                if(cell->photo == 0)
                {
                    if(neighbor->energy >= read && itself->energy + cell->buf_energy < 255 - read / eat_div)
                    {
                        eaten_energy += read;
                        cell->buf_energy += read;
                        neighbor->energy -= read;
                        cell->acc += 127;
                    }
                    else if(itself->energy + cell->buf_energy + neighbor->energy / eat_div > 254)
                    {
                        eaten_energy += 254 - itself->energy - cell->buf_energy;
                        neighbor->energy -= (254 - itself->energy - cell->buf_energy) * eat_div;
                        cell->buf_energy += 254 - itself->energy - cell->buf_energy;
                        cell->acc += 127;
                    }
                    else 
                    {
                        eaten_energy += neighbor->energy;
                        cell->buf_energy += neighbor->energy;
                        neighbor->energy = 0;
                        cell->acc += 127;
                    }
                }
                eaten_matter = eaten_matter * 255 / max_matter;
                // if(rnd() % 256 < eaten_matter)
                //     Rec_Push_Attempt(x, y, dx, dy, max_strength, 0);
            }
            break;
        case CMD_LOOK_TYPE:
            if(debug_life) fprintf(stderr, "CMD_LOOK_TYPE\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_TYPE\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->type;
            break;
        case CMD_LOOK_GNM:
            if(debug_life) fprintf(stderr, "CMD_LOOK_GNM\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_GNM\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = 255 * (cells[neighbor->id].g_id == cell->g_id);
            break;
        case CMD_LOOK_LINK:
            if(debug_life) fprintf(stderr, "CMD_LOOK_LINK\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_LINK\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = 0;
            if(cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                
                if(itself->links & mask)
                    cell->acc = 255;
            }
            break;
        case CMD_LOOK_MEMB:
            if(debug_life) fprintf(stderr, "CMD_LOOK_MEMB\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_MEMB\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = Is_Membrane(cell->x + dx, cell->y + dy);
            break;
        case CMD_LOOK_MAT:
            if(debug_life) fprintf(stderr, "CMD_LOOK_MAT\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_MAT\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->matter;
            break;
        case CMD_LOOK_NRG:
            if(debug_life) fprintf(stderr, "CMD_LOOK_NRG\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_NRG\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->energy;
            break;
        case CMD_LOOK_LGHT:
            if(debug_life) fprintf(stderr, "CMD_LOOK_LGHT\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_LGHT\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = neighbor->light;
            break;
        case CMD_LOOK_ACC:
            if(debug_life) fprintf(stderr, "CMD_LOOK_ACC\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_LOOK_ACC\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            cell->acc = cells[neighbor->id].acc;
            break;
        case CMD_DETACH:
            if(debug_life) fprintf(stderr, "CMD_DETACH\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_DETACH\n", *pc, gene->arg, cell->acc);
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
                if(cell->energy_out & mask)
                    cell->energy_out &= (uint8_t)~mask;
                if(cell->matter_out & mask)
                    cell->matter_out &= (uint8_t)~mask;
            }
            break;
        case CMD_ATTACH:
            if(debug_life) fprintf(stderr, "CMD_ATTACH\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_ATTACH\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(Count_Bits_8(itself->links) >= max_links
            || Count_Bits_8(neighbor->links) >= max_links) break;
            
            if(neighbor->type == 1 && cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                itself->links |= mask;
                
                mask = (uint8_t)1 << mod(cell->dir + 4, 8);
                neighbor->links |= mask;
            }
            
            break;
        case CMD_OUTLET_OFF:
            if(debug_life) fprintf(stderr, "CMD_OUTLET_OFF\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_OUTLET_OFF\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            if(cell->dir != 8)
            {
                mask = (uint8_t)1 << cell->dir;
                if(cell->energy_out & mask && (gene->arg >> 0) & 1)
                    cell->energy_out &= (uint8_t)~mask;
                if(cell->matter_out & mask && (gene->arg >> 1) & 1)
                    cell->matter_out &= (uint8_t)~mask;
            }
            break;
        case CMD_OUTLET_ON:
            if(debug_life) fprintf(stderr, "CMD_OUTLET_ON\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_OUTLET_ON\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            temp = gene->arg % 2;
            
            if(Count_Bits_8(itself->links) >= max_links
            || Count_Bits_8(neighbor->links) >= max_links) break;
            
            if(neighbor->id != 0 && cell->dir != 8 && cells[neighbor->id].photo == cell->photo)
            {
                mask = (uint8_t)1 << cell->dir;
                itself->links |= mask;
                
                mask = (uint8_t)1 << mod(cell->dir + 4, 8);
                neighbor->links |= mask;
            
                mask = (uint8_t)1 << cell->dir;
                if((gene->arg >> 0) & 1)
                    cell->energy_out |= mask;
                if((gene->arg >> 1) & 1)
                    cell->matter_out |= mask;
            }
            break;
        case CMD_SET_PHERO:
            if(debug_life) fprintf(stderr, "CMD_SET_PHERO\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_SET_PHERO\n", *pc, gene->arg, cell->acc);
            dx = dir_to_coords[cell->dir][0];
            dy = dir_to_coords[cell->dir][1];
            neighbor = Grid_Get(cell->x + dx, cell->y + dy);
            itself = Grid_Get(cell->x, cell->y);
            
            Phero_Set(cell->x, cell->y, gene->arg * MAX_PHEROMONES / 255, cell->acc);
            break;
        case CMD_GET_PHERO:
            if(debug_life) fprintf(stderr, "CMD_GET_PHERO\n"), fflush(stderr);
            if(followed_id == id) fprintf(file_ptr, "pc %d\t%d\t%d\tCMD_GET_PHERO\n", *pc, gene->arg, cell->acc);
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
    
    if(followed_id == id)
    {
        fflush(file_ptr);
        fclose(file_ptr);
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
    
    uint8_t value, error = 0;
    
    value = itself->matter + cell->buf_matter;
    if(value >= 0 && value <= max_matter)
        itself->matter += cell->buf_matter;
    else error = 1;
    
    value = itself->energy + cell->buf_energy;
    if(value >= 0 && value <= 255)
        itself->energy += cell->buf_energy;
    else error = 2;
    
    if(error == 1) 
    {
        printf("matter error!!! x %d y %d %d %d\n", cell->x, cell->y, itself->matter, cell->buf_matter);
        printf("%d\n", 1 / 0);
    }
    if(error == 2) 
    {
        printf("energy error!!!\n");
        printf("%d\n", 1 / 0);
    }
    
    cell->buf_energy = 0;
    cell->buf_matter = 0;
    
    if(rnd() % lifetime == 0 && nat_death && cell->used == 1)
    {
        if(
        (  itself->energy == 0 
        || itself->energy == 255)
        ) 
        {
            if(cell->active == 1)
            {
                cell->active = 0;
                
                population_size--;
                if(cell->photo)
                {
                    last_plant = cell->g_id;
                    plant_pop--;
                }
                else
                {
                    last_animal = cell->g_id;
                    animal_pop--;
                }
                return;
            }
        }
        else if(cell->active == 0)
        {
            cell->active = 1;
            population_size++;
            if(cell->photo)
            {
                plant_pop++;
            }
            else
            {
                animal_pop++;
            }
        }
    }
    
    int8_t photo_threshold = -1;
    uint8_t thermo_cond = (itself->matter) * 8 / max_matter;
    uint8_t heat_loss = min(Is_Membrane(cell->x, cell->y), thermo_cond);
    uint8_t light_blocking = (itself->matter) * 8 / max_matter;
    uint8_t light = itself->light * light_blocking * 8 / max_light;
    int16_t new_energy, old_energy = itself->energy, delta_energy = 0;
    
    if(life && cell->active)
    {
        if(cell->photo == 1)
        {
            new_energy = (int16_t)(itself->energy - 1 + light);
            delta_energy = min(max(new_energy, 0), 255) - itself->energy;
            // if(delta_energy > 0) printf("%d\n", delta_energy);
        }
        else
        {
            new_energy = (int16_t)(itself->energy - 1 - max(heat_loss - light, 0));
            delta_energy = max(new_energy, 0) - itself->energy;
            // if(delta_energy < -1) printf("%d\n", delta_energy);
        }
    }
    
    if(delta_energy > 0) itself->energy += delta_energy;
    if(delta_energy < 0) itself->energy += delta_energy;
    
    if(delta_energy > 0) energy_gain += delta_energy;
    if(delta_energy < 0) energy_loss += delta_energy;
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
    
    uint8_t spread_thin = 0;
    x = cell->x, y = cell->y;
    
    if(Count_Bits_8(itself->links) == 0) return;
    if(Count_Bits_8(cell->energy_out) == 0
    && Count_Bits_8(cell->matter_out) == 0) return;
    
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        mask = (uint8_t)1 << dir;
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        cell_n = &cells[neighbor->id];
        
        if(cell->energy_out & mask
        && neighbor->id != 0
        && cell_n->used)
        {
            neighbor_amount++;
        }
    }
    
    {
        desired_energy = -(int16_t)((itself->energy - 1) * (itself->energy > 1));
    }
    
    if(neighbor_amount == 0) return;
    
    spread_energy = (int16_t)(desired_energy / neighbor_amount);
    
    spread_thin =0;
    if(desired_energy < neighbor_amount) spread_thin = 1;
    
    if(spread_thin)
    {
        uint8_t dir = cell->dir;
        spread_energy = desired_energy;
        mask = (uint8_t)1 << dir;
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        cell_n = &cells[neighbor->id];
        
        if(cell->energy_out & mask
        && neighbor->id != 0
        && cell_n->used)
        {
            res_energy = (int16_t)(neighbor->energy + cell_n->buf_energy - spread_energy);
            
            ediff = spread_energy;
            
            if(res_energy > 254)
            {
                ediff = neighbor->energy + cell_n->buf_energy - 254;
                res_energy = 254;
            }
            s_ediff = (ediff);
            
            if(life
            && itself->energy + cell->buf_energy + s_ediff > -1 && itself->energy + cell->buf_energy + s_ediff < 256
            && neighbor->energy + cell_n->buf_energy - s_ediff > -1 && neighbor->energy + cell_n->buf_energy - s_ediff < 256
            )
            {
                itself->energy += s_ediff;
                cell_n->buf_energy -= s_ediff;
            }
        }
    }
    else
    {
        for(uint8_t dir = 0; dir < 8; dir++)
        {
            mask = (uint8_t)1 << dir;
            dx = dir_to_coords[dir][0];
            dy = dir_to_coords[dir][1];
            neighbor = Grid_Get(x + dx, y + dy);
            cell_n = &cells[neighbor->id];
            
            if(cell->energy_out & mask
            && neighbor->id != 0
            && cell_n->used)
            {
                res_energy = (int16_t)(neighbor->energy + cell_n->buf_energy - spread_energy);
                
                ediff = spread_energy;
                
                if(res_energy > 254)
                {
                    ediff = neighbor->energy + cell_n->buf_energy - 254;
                    res_energy = 254;
                }
                s_ediff = (ediff);
                
                if(life
                && itself->energy + cell->buf_energy + s_ediff > -1 && itself->energy + cell->buf_energy + s_ediff < 256
                && neighbor->energy + cell_n->buf_energy - s_ediff > -1 && neighbor->energy + cell_n->buf_energy - s_ediff < 256
                )
                {
                    itself->energy += s_ediff;
                    cell_n->buf_energy -= s_ediff;
                }
            }
        }
    }
    
    for(uint8_t dir = 0; dir < 8; dir++)
    {
        mask = (uint8_t)1 << dir;
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        cell_n = &cells[neighbor->id];
        
        if(cell->matter_out & mask
        && neighbor->id != 0
        && cell_n->used)
        {
            neighbor_amount++;
        }
    }
    
    {
        desired_matter = -(int16_t)itself->matter;
    }
    
    if(neighbor_amount == 0) return;
    
    spread_matter = (int16_t)(desired_matter / neighbor_amount);
    
    spread_thin = 0;
    if(desired_matter < neighbor_amount) spread_thin = 1;
    
    if(spread_thin)
    {
        uint8_t dir = cell->dir;
        spread_matter = desired_matter;
        mask = (uint8_t)1 << dir;
        dx = dir_to_coords[dir][0];
        dy = dir_to_coords[dir][1];
        neighbor = Grid_Get(x + dx, y + dy);
        cell_n = &cells[neighbor->id];
        
        if(cell->matter_out & mask
        && neighbor->id != 0
        && cell_n->used)
        {
            res_matter = (int16_t)(neighbor->matter + cell_n->buf_matter - spread_matter);
            
            mdiff = spread_matter;
            
            if(res_matter > max_matter)
            {
                mdiff = neighbor->matter + cell_n->buf_matter - max_matter;
                res_matter = max_matter;
            }
            
            s_mdiff = (mdiff);
            
            if(life
            && itself->matter + cell->buf_matter + s_mdiff > -1 && itself->matter + cell->buf_matter + s_mdiff <= max_matter
            && neighbor->matter + cell_n->buf_matter - s_mdiff > -1 && neighbor->matter + cell_n->buf_matter - s_mdiff <= max_matter
            )
            {
                itself->matter += s_mdiff;
                cell_n->buf_matter -= s_mdiff;
            }
        }   
    }
    else
    {
        for(uint8_t dir = 0; dir < 8; dir++)
        {
            mask = (uint8_t)1 << dir;
            dx = dir_to_coords[dir][0];
            dy = dir_to_coords[dir][1];
            neighbor = Grid_Get(x + dx, y + dy);
            cell_n = &cells[neighbor->id];
            
            if(cell->matter_out & mask
            && neighbor->id != 0
            && cell_n->used)
            {
                res_matter = (int16_t)(neighbor->matter + cell_n->buf_matter - spread_matter);
                
                mdiff = spread_matter;
                
                if(res_matter > max_matter)
                {
                    mdiff = neighbor->matter + cell_n->buf_matter - max_matter;
                    res_matter = max_matter;
                }
                
                s_mdiff = (mdiff);
                
                if(life
                && itself->matter + cell->buf_matter + s_mdiff > -1 && itself->matter + cell->buf_matter + s_mdiff <= max_matter
                && neighbor->matter + cell_n->buf_matter - s_mdiff > -1 && neighbor->matter + cell_n->buf_matter - s_mdiff <= max_matter
                )
                {
                    itself->matter += s_mdiff;
                    cell_n->buf_matter -= s_mdiff;
                }
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
    for(int y = border; y < grid_height - border; y++)
    {
        for(int x = border; x < grid_width - border; x++)
        {
            if(rnd() % 1000 < n
            )
            {
                if(Find_Free_Genome_Id() != 0)
                {
                    tile = Grid_Get(x, y);
                    
                    if(cells[tile->id].used && tile->id)
                    {
                        Cell_Destroy(tile->id);
                    }
                    
                    if(tile->type == 0)
                    {
                        Grid_Set(x, y, 0, 0);
                        Cell_Create(x, y, 0, rnd() % 2, 0);
                    }
                }
            }
        }
    }
    Illuminate();
    Border();
}

void Reanimate(int n)
{
    Tile *tile;
    int16_t energy_delta;
    for(int y = border; y < grid_height - border; y++)
    {
        for(int x = border; x < grid_width - border; x++)
        {
            if(rnd() % 1000 < n
            )
            {
                if(Find_Free_Genome_Id() != 0)
                {
                    tile = Grid_Get(x, y);
                    
                    if(cells[tile->id].used == 0 && tile->type == 1
                    // && tile->matter == starting_matter
                    && Is_Membrane(x, y)
                    )
                    {
                        energy_delta = 0 - Grid_Get(x, y)->energy;
                        if(energy_delta > 0) energy_gain += energy_delta;
                        if(energy_delta < 0) energy_loss += energy_delta;
                        if(debug_life) fprintf(stderr, "Reanimate, energy changed\n");
                        Grid_Set(x, y, 0, 0);
                        Cell_Create(x, y, 0, rnd() % 2, 0);
                    }
                }
            }
        }
    }
    Illuminate();
    Border();
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
                Cell_Create(cells[id].x + dx, cells[id].y + dy, id, cells[id].photo, rnd() % 2);
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

void Life_Reset(uint16_t n)
{
    Tile *tile;
    int16_t energy_delta;
    for(int y = border; y < grid_height - border; y++)
    {
        for(int x = border; x < grid_width - border; x++)
        {
            tile = Grid_Get(x, y);
            if(rnd() % 1000 < n)
            {
                if(cells[tile->id].used && tile->id != 0)
                {
                    Cell_Destroy(tile->id);
                }
                
                energy_delta = 0 - Grid_Get(x, y)->energy;
                if(energy_delta > 0) energy_gain += energy_delta;
                if(energy_delta < 0) energy_loss += energy_delta;
                if(debug_life) fprintf(stderr, "Life_Reset, energy changed\n");
                Grid_Set(x, y, 0, 0);
            }
        }
    }
    total_cycles = 0;
    cycles = 0;
}

void Gravity()
{
    uint8_t local_debug = 0;
    Tile *tile;
    if(gravity && timer % grav_period == 0) 
    {
        if(local_debug) printf("\ngravity\n");
        Tile *upper_left, *upper, *upper_right;
        int32_t ret;
        uint32_t rcm_count = 0;
        uint16_t x, y;
        for(uint32_t n = 0; n < grav_rate; n++)
        {
            x = rnd() % grid_width;
            y = rnd() % grid_height;
            
            tile = Grid_Get(x, y);
            if(tile->type == 1)
            {
                Rec_Push_Attempt(x, y, 0, 1, max_strength, 1);
            }
        }
    }
}

void Illuminate()
{
    if(lighting && (long_timer % lighting_period == 0 || pause))
    {
        Tile *tile;
        uint16_t direction;
        
        direction = 2048 * (long_timer % day_length) / day_length;
        
        // printf("%d %d %d\n", direction, direction / 256, direction % 256);
        
        if(sudden_death) sun_light = 0;
        else sun_light = max(abs(mod(long_timer + day_length / 2, day_length) - day_length / 2) * (max_light + night_depth) * 2 / day_length - night_depth, 0);
        
        for(int y = 0; y < grid_height; y++)
        {
            for(int x = 0; x < grid_width; x++)
            {
                tile = Grid_Get(x, y);
                tile->light = 0;
            }
        }
        
        if(sun_light == 0) return;
        
        for(int y = 0; y < grid_height; y++)
        {
            tile = Grid_Get(0, y);
            tile->light = 255;
            
            tile = Grid_Get(grid_width - 1, y);
            tile->light = 255;
        }
        for(int x = 0; x < grid_width; x++)
        {
            tile = Grid_Get(x, 0);
            tile->light = 255;
            
            tile = Grid_Get(x, grid_height - 1);
            tile->light = 255;
        }
        
        max_light_strength = max(grid_width, grid_height);
        
        for(int y = 0; y < grid_height; y++)
        {
            for(int x = 0; x < grid_width; x++)
            {
                tile = Grid_Get(x, y);
                if(tile->light == 0)
                {
                    Rec_Find_Light(x, y, max_light_strength, direction, 0);
                }
            }
        }
        
        for(int y = 0; y < grid_height; y++)
        {
            for(int x = 0; x < grid_width; x++)
            {
                tile = Grid_Get(x, y);
                tile->rec_str = 0;
            }
        }
    }
}