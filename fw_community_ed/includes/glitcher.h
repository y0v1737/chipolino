#ifndef GLITCHER_H
#define GLITCHER_H

#include "hardware/pio.h"

typedef struct {    
    PIO pio; 
    uint32_t pio_sm;
    pio_program_t pio_program;
    uint32_t pio_program_entry; 
    uint32_t pio_program_load_offset;
    pio_sm_config (*program_get_default_config)(uint offset);
}glitcher_t;

extern glitcher_t g_3v3_rst;
extern glitcher_t g_gnd_gpio;
extern glitcher_t g_gnd_cnt;
extern glitcher_t g_gnd_cnt_2g;
extern glitcher_t g_gnd_3v3;
extern glitcher_t g_3v3_cnt;

uint32_t gnd_cnt_load_pio_prog();
uint32_t gnd_cnt_glitch(uint32_t offset, uint32_t width);

uint32_t gnd_3v3_load_pio_prog();
uint32_t gnd_3v3_glitch(uint32_t offset, uint32_t width);

uint32_t f_3v3_rst_load_pio_prog();
uint32_t f_3v3_rst_glitch(uint32_t offset, uint32_t width);

uint32_t gnd_gpio_load_pio_prog();
uint32_t gnd_gpio_glitch(uint32_t offset, uint32_t width);

uint32_t f_3v3_cnt_load_pio_prog();
uint32_t f_3v3_cnt_glitch(uint32_t offset, uint32_t width);


#endif 