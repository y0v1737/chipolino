#ifndef RK3588_H
#define RK3588_H

#include "../includes/common.h"

extern target_t rk3588_tgt;

uint32_t rk3588_board_init();
uint32_t rk3588_sync();
uint32_t rk3588_is_unlock();
uint32_t rk3588_rst_state();

#endif 