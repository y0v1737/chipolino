#ifndef STM_LIB_H
#define STM_LIB_H

#include "../includes/common.h"

void stm_get_rdp();
void stm_set_rdp(uint8_t level);

uint32_t stm_spi_init();
uint32_t stm_uart_init();

uint32_t stm_spi_sync();
uint32_t stm_uart_sync();
uint32_t stm_rdp2_sync();

uint32_t stm_spi_is_unlock();
uint32_t stm_uart_is_unlock();
uint32_t stm_rdp2_is_unlock();

uint32_t stm_rst_state();
uint32_t stm_rdp2_board_init();

uint32_t stm_rdp1_glitch(uint32_t offset, uint32_t width);
uint32_t stm_rdp1_load_pio_prog();

uint32_t stm_rdp2_load_pio_prog();
uint32_t stm_rdp2_glitch(uint32_t offset, uint32_t width);


#endif 