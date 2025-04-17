#ifndef COMMON_H
#define COMMON_H
      
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/bootrom.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"
#include "hardware/uart.h"
#include "hardware/pio.h"
#include "hardware/spi.h"

#include "p_3v3_rst.pio.h"
#include "p_gnd_gpio.pio.h"
#include "p_gnd_cnt.pio.h"
#include "p_gnd_cnt_2g.pio.h"
#include "p_gnd_3v3.pio.h"
#include "p_3v3_cnt.pio.h"

#include "ws2812.h"
#include "target.h"
#include "swd_pico.h"

#define STM_SPI spi0
#define LPC_UART uart0
#define STM_UART uart0
#define RH850_UART uart0
#define ESP_SPI spi0
#define GD_UART uart0

#define GD_PIN_UART_TX GP12_I2C0_SDA_UART0_TX
#define GD_PIN_UART_RX GP13_I2C0_SCL_UART0_RX

#define STM_PIN_UART_TX GP12_I2C0_SDA_UART0_TX
#define STM_PIN_UART_RX GP13_I2C0_SCL_UART0_RX

#define RH850_PIN_UART_TX GP12_I2C0_SDA_UART0_TX
#define RH850_PIN_UART_RX GP13_I2C0_SCL_UART0_RX

#define LPC_PIN_UART_TX GP16_SPI0_RX_UART0_TX
#define LPC_PIN_UART_RX GP17_SPI0_nCS_UART0_RX

#define DEFAULT_SWD_PIO pio0
#define DEFAULT_SWD_SM 0

#define DEFAULT_GLITCHER_PIO pio1
#define DEFAULT_GLITCHER_SM 0

#define MAX_LEN_SPI_BUF 0x100
#define MAX_LEN_DUMP_MEM 0x1000

#define BIT_CHECK(var,pos) ((var) & (1<<(pos)))
#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))

#define LENGTH(ARRAY) (sizeof(ARRAY) / sizeof(ARRAY[0]))

#define CLK_MAIN_KHZ 250000UL

#define US2TICKS(us) (uint32_t)((us)*CLK_MAIN_KHZ/1000UL)
#define MS2TICKS(ms) (uint32_t)((ms)*CLK_MAIN_KHZ/1000000UL)

#define FLAG_CORE_VALUE 123
#define USB_ARG_MAX_LEN 0x80
#define USB_STR_LEN 0x200
#define UART_BUFF_LEN 0x200

#define STOP_CMD_BIT 0
#define GLITCH_CMD_BIT 1
#define GLITCHING_BIT 2
#define SYNC_BIT 3
#define GLITCH_SUCC_BIT 4
#define GLITCH_FAIL_BIT 5
#define GLITCH_ADDR_CMD 6
#define GLITCH_ADDR_CMD_BIT 7

#define GP0_SWCLK_PIO                            0
#define GP1_SWDIO_PIO                            1
#define GP2                                      2
#define GP3_TARGET_RESET                         3
#define GP4_3V3_CTRL                             4
#define GP5_MOSFET_CTRL                          5
#define GP6_VCC_EXT_3V3_LN_CTRL                  6
#define GP7_1V2_CTRL                             7 
#define GP8_1V8_CTRL                             8
#define GP9_5V_CTRL                              9
#define GP10_RP_TRIGGER                         10
#define GP11_SWD_CTRL                           11
#define GP12_I2C0_SDA_UART0_TX                  12
#define GP13_I2C0_SCL_UART0_RX                  13
#define GP14_TRIG_PWR_SEL_0                     14
#define GP15_TRIG_PWR_SEL_1                     15
#define GP16_SPI0_RX_UART0_TX                   16
#define GP17_SPI0_nCS_UART0_RX                  17
#define GP18_SPI0_CLK                           18
#define GP19_SPI0_TX                            19
#define GP20_I2C0_SDA                           20
#define GP21_I2C0_SCL                           21
#define GP22_BTN                                22
#define GP26                                    26
#define GP27_LED_RGD                            27
#define GP28_ADC2                               28

// BRG blue red green
#define GREEN_COLOR  0b000000000000000011111000
#define BLUE_COLOR   0b111110000000000000000000 
#define RED_COLOR    0b000000001111100000000000 
#define WHITE_COLOR  0b111100001111000011110000 
#define PINK_COLOR   0b111100001111100000000000 
#define ORANGE_COLOR 0b000000001111100011100000 
#define OFF_COLOR    0b000000000000000000000000 

#define LOG_INFO_STR 1
#define LOG_INFO_RAW 2
#define LEN_LOG_INFO_MAX 0x10
typedef struct {
    uint8_t sig;
    uint8_t len;
    uint8_t type;
    uint8_t data[LEN_LOG_INFO_MAX];
}log_info_t;

typedef struct {
    uint32_t state;
    uint32_t cmd;
    uint32_t cur_offset;
    uint32_t cur_width;
    uint32_t start_offset;
    uint32_t end_offset;
    uint32_t start_width;
    uint32_t end_width; 
    uint8_t step;
    uint32_t attempt;
    uint32_t target_no;
    uint32_t dump_addr;
    uint8_t memory[MAX_LEN_DUMP_MEM];
    log_info_t* log_info;
    target_t* target;
    uint32_t (*on_succ)();
    uint32_t (*on_fail)();
    uint32_t (*on_user_stop)();
}app_t;

extern app_t app;

typedef struct {
    char name[20];
    int arglen;
    void (*func)(void);
} command_t;

command_t registered_commands[20];
char command_args[6][100];
uint32_t reg_cmd_len;

typedef enum {
    V_1V2,
    V_1V8,
    V_3V3,
    V_Z
}VOLT_SEL;

uint8_t str_arg[USB_STR_LEN];

void clear_board();
void set_leds_color(uint32_t color1, uint32_t color2);
uint32_t swd_ext_set(bool state);
void trst_pin_set(bool state);
void target_nrst(bool state);
void ctrl_3v3_pin_set(bool state);
void ctrl_ext_vcc_3v3_ln_set(bool state);
void gpio_pin_set(uint32_t gp, bool state);

uint32_t uart_read_buff(uart_inst_t * uart, uint8_t* buff);
uint32_t uart_send_wait(uart_inst_t * uart, uint8_t* buff_tx, uint8_t* buff_rx, uint32_t time_wait);
uint32_t uart_tx_mem(uart_inst_t * uart, uint8_t* buff_tx, uint32_t size);
uint32_t clr_uart_fifo(uart_inst_t * uart);
uint8_t getc_uart(uart_inst_t * uart, uint32_t time_wait);
uint32_t uart_tx_print(uart_inst_t * uart, uint8_t* buff_tx, uint32_t time_wait);
void log_info_str(uint8_t* str, uint8_t size);
void log_info_raw(uint8_t* data, uint8_t size);

#endif 
