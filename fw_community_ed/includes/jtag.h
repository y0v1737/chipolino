#ifndef _JTAG_H
#define _JTAG_H

#include "common.h"

#define TCK_PIN     GP18_SPI0_CLK
#define TMS_PIN     GP17_SPI0_nCS_UART0_RX
#define TDI_PIN     GP19_SPI0_TX
#define TDO_PIN     GP16_SPI0_RX_UART0_TX

void jtag_init();
void jtag_reset_tap();
uint32_t jtag_read_idcode();

void jtag_tck_pulses(uint32_t pulses);
void jtag_clock_pulse();

void jtag_enter_shift_ir();
void jtag_exit_shift_ir();

void jtag_enter_shift_dr();
void jtag_exit_shift_dr();

uint64_t jtag_shift_bit(uint64_t data, uint8_t len);


#endif