#ifndef RH850_H
#define RH850_H

#include "../includes/common.h"

extern target_t rh850_ser_tgt;

uint32_t rh850_board_init();
uint32_t rh850_sync();
uint32_t rh850_is_unlock();
uint32_t rh850_reset_state();

#endif