
#ifndef SWD_PICO_H
#define SWD_PICO_H

#include "common.h"

#define PROBE_PIN_SWCLK GP0_SWCLK_PIO
#define PROBE_PIN_SWDIO GP1_SWDIO_PIO

#define SWD_CMD_START   (1 << 0)    /* always set */
#define SWD_CMD_APNDP   (1 << 1)    /* set only for AP access */
#define SWD_CMD_RNW     (1 << 2)    /* set only for read access */
#define SWD_CMD_A32     (3 << 3)    /* bits A[3:2] of register addr */
#define SWD_CMD_PARITY  (1 << 5)    /* parity of APnDP|RnW|A32 */
#define SWD_CMD_STOP    (0 << 6)    /* always clear for synch SWD */
#define SWD_CMD_PARK    (1 << 7)    /* driven high by host */

uint32_t init_swd_pico();
uint32_t swd_check();
void swd_init();
uint32_t ahb_ap_check();
uint32_t nrf52_erase();
uint32_t nrf52_aprotect_en();
void probe_write_mode(void);
void probe_write_bits(uint bit_count, uint32_t data_byte);
uint32_t swd_memread_noreset(uint32_t addr);
uint32_t swd_memread(uint32_t addr, uint8_t * ack, int delay);
void swd_memwrite_noreset(uint32_t addr, uint32_t value);
void stm32_set_rdp(uint8_t level);
uint8_t stm32_get_rdp();

#endif 