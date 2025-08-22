#ifndef RH850_H
#define RH850_H

#include "../includes/common.h"

extern target_t rh850_ser_tgt;
extern target_t rh850_rp_tgt;
extern target_t rh850_id_tgt;

uint32_t rh850_board_init();
uint32_t rh850_sync();
uint32_t rh850_is_unlock();
uint32_t rh850_reset_state();

uint32_t rh850_check_rp();

uint32_t rh850_rp_15_sync();
uint32_t rh850_rp_15_is_unlock();

uint32_t rh850_id_30_sync();
uint32_t rh850_id_30_is_unlock();

#endif