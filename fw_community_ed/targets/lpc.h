#ifndef LPC_H
#define LPC_H

#include "../includes/common.h"

// #define LPC_UART_WAIT_US 15000 // lpc1343 ok
#define LPC_UART_WAIT_US 50000    // lpc2148 ok

extern target_t lpc2148_tgt;
extern target_t lpc1343_tgt;

uint32_t lpc_rst_state();

uint32_t lpc2148_board_init();
uint32_t lpc2148_sync();
uint32_t lpc2148_is_unlock();

uint32_t lpc1343_board_init();
uint32_t lpc1343_sync();
uint32_t lpc1343_is_unlock();

#endif 