#include "stm32f4xx.h"

///////////////////////////////////////////////////////
target_t stm32f4xx_spi_rdp1_tgt ={
        .name = "stm32f4xx_spi_rdp1",
        .rst_delay_tick = 1000000,
        .check_delay_us = 100,
        .count_edge_pio = 23, // clk count
        .ap_id = 0x2BA01477,
        .sync_checker = stm_spi_sync,
        .unlock_checker = stm_spi_is_unlock,
        .rst_state = stm_rst_state,
        .board_init = stm_spi_init,
        .glitcher = &g_gnd_cnt,
        .load_pio_prog = stm_rdp1_load_pio_prog,
        .glitch = stm_rdp1_glitch,
};

target_t stm32f4xx_uart_rdp1_tgt = {
        .name = "stm32f4xx_uart_rdp1",
        .rst_delay_tick = 1000000,
        .check_delay_us = 100,
        .count_edge_pio = 5, // clk count
        .ap_id = 0x2BA01477,
        .sync_checker = stm_uart_sync,
        .unlock_checker = stm_uart_is_unlock,
        .rst_state = stm_rst_state,
        .board_init = stm_uart_init,
        .glitcher = &g_gnd_cnt,
        .load_pio_prog = stm_rdp1_load_pio_prog,
        .glitch = stm_rdp1_glitch,
};

target_t stm32f4xx_rdp2_tgt ={
        .name = "stm32f4xx_rdp2",
        .rst_delay_tick = 1000000,
        .check_delay_us = 5000,
        .ap_id = 0x2BA01477,
        .sync_checker = stm_rdp2_sync,
        .unlock_checker = stm_rdp2_is_unlock,
        .rst_state = stm_rst_state,
        .board_init = stm_rdp2_board_init,
        .glitcher = &g_gnd_3v3,
        .load_pio_prog = stm_rdp2_load_pio_prog,
        .glitch = stm_rdp2_glitch,
};
///////////////////////////////////////////////////////
