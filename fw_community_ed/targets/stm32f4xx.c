#include "stm32f4xx.h"

///////////////////////////////////////////////////////
// STM32f411/STM32F411
target_t stm32f401_spi_rdp1_tgt ={
        .name = "stm32f401_spi_rdp1",
        .rst_delay_tick = 1000000,
        .check_delay_us = 100,
        .count_edge_pio = 23, // clk count
        .ap_id = 0x2BA01477,
        .glitcher = &g_gnd_cnt,
        .sync_checker = stm_spi_sync,
        .unlock_checker = stm_spi_is_unlock,
        .rst_state = stm_rst_state,
        .board_init = stm_spi_init,
        .load_pio_prog = stm_rdp1_load_pio_prog,
        .glitch = stm_rdp1_glitch,
};

target_t stm32f401_rdp2_tgt ={
        .name = "stm32f401_rdp2",
        .rst_delay_tick = 1000000,
        .check_delay_us = 5000,
        .ap_id = 0x2BA01477,
        .glitcher = &g_gnd_3v3,
        .sync_checker = stm_rdp2_sync,
        .unlock_checker = stm_rdp2_is_unlock,
        .rst_state = stm_rst_state,
        .board_init = stm_rdp2_board_init,
        .load_pio_prog = stm_rdp2_load_pio_prog,
        .glitch = stm_rdp2_glitch,
};
///////////////////////////////////////////////////////