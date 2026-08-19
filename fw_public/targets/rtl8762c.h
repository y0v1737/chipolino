#ifndef RTL8762C_H
#define RTL8762C_H

#include "../includes/common.h"

extern target_t rtl8762c_tgt;


uint32_t rtl8762c_board_init();
uint32_t rtl8762c_sync();
uint32_t rtl8762c_is_unlock();
uint32_t rtl8762c_mosfet_rst_state();
void     rtl8762c_cmd_parser(char*);

#endif 