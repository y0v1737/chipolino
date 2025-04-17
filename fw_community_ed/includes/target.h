#ifndef TARGET_H
#define TARGET_H

#include "glitcher.h"

# define MAX_TARGET_NAME_LEN 0x20

typedef struct {
    char name[MAX_TARGET_NAME_LEN];    
    uint32_t (*board_init)();
    uint32_t (*rst_state)();
    uint32_t (*sync_checker)();
    uint32_t (*unlock_checker)();
    uint32_t (*load_pio_prog)();
    uint32_t (*dump)(uint32_t addr);
    uint32_t (*glitch)(uint32_t offset, uint32_t width);
    glitcher_t* glitcher;
    uint32_t rst_delay_tick;
    uint32_t check_delay_us;
    uint32_t count_edge_pio;
    uint32_t ap_id;
}target_t;

#endif 
