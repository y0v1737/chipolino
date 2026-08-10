#include "rk3588.h"
#include "uart_rx.pio.h"    

#define UART_BAUD      1500000

static volatile uint offset;

target_t rk3588_tgt = {
    .name = "rk3588",
    .rst_delay_tick = 1000000,
    .check_delay_us = 100,
    .count_edge_pio = 17,
    .sync_checker = rk3588_sync,
    .unlock_checker = rk3588_is_unlock,
    .rst_state = rk3588_rst_state,
    .board_init = rk3588_board_init,
    .glitcher = &g_gnd_cnt,
    .load_pio_prog = gnd_cnt_load_pio_prog,
    .glitch = gnd_cnt_glitch,
};

uint32_t init_uart_pio()
{
    offset = pio_add_program(DEFAULT_FREE_PIO, &uart_rx_mini_program);
    uart_rx_mini_program_init(
        DEFAULT_FREE_PIO,
        DEFAULT_FREE_SM,
        offset,
        MCU_PIN_UART_RX,
        UART_BAUD
    );
}

static inline void uart_rx_mini_reset(PIO pio, uint sm, uint offset)
{
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_clear_fifos(pio, sm);
    pio_sm_restart(pio, sm);
    pio_sm_exec(pio, sm, pio_encode_jmp(offset));
    pio_sm_set_enabled(pio, sm, true);
}

uint32_t rk3588_board_init()
{ 
    init_uart_pio();
    sleep_us(500);
} 

uint32_t rk3588_sync()
{
    return 1;
}

uint32_t rk3588_is_unlock()
{        
    uint8_t tmp[0x10];
    uint64_t start_time = time_us_64();
    while (!gpio_get(MCU_PIN_UART_RX) && (time_us_64()-start_time < 50000) )
    {
        tight_loop_contents();
    }        

    uart_rx_mini_reset(DEFAULT_FREE_PIO, DEFAULT_FREE_SM, offset);
    for (int i = 0; i < 10; i++) 
    {
        uart_rx_program_getc_timeout(DEFAULT_FREE_PIO, DEFAULT_FREE_SM, 1000, &(tmp[i]));
    }

    printf("%c%c%c\r\n", tmp[0], tmp[1], tmp[2]);
    // if (tmp[0]=='D' || tmp[1]=='D' || tmp[2]=='A') // check `DDA` string
    if (tmp[0]=='D' || tmp[1]=='D') // check just `DD`
    {
        return 1;
    }  
    return 0;    
}

uint32_t rk3588_rst_state()
{
    clr_uart_fifo(MCU_UART);
    target_nrst(false);
    sleep_ms(50);
    target_nrst(true);
    sleep_ms(2);
    return 1;
}
