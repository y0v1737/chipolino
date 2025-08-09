#ifndef NRF52_H
#define NRF52_H

#include "../includes/common.h"

#define AP_ID_LOCK 0x23000000UL

extern target_t nrf52_tgt;


uint32_t nrf52_board_init();
uint32_t nrf52_sync();
uint32_t nrf52_is_unlock();
uint32_t nrf52_mosfet_rst_state();

#endif 