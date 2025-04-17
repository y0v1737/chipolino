#include "lpc.h"

    target_t  lpc2148_tgt = {
        .name = "lpc2148",
        .rst_delay_tick = MS2TICKS(15),
        .check_delay_us = 2000,
        .glitcher = &g_3v3_rst,
        .board_init = lpc2148_board_init,
        .rst_state = lpc_rst_state,
        .sync_checker = lpc2148_sync,
        .unlock_checker = lpc2148_is_unlock, 
        .load_pio_prog = f_3v3_rst_load_pio_prog,
        .glitch = f_3v3_rst_glitch,
    };

target_t  lpc1343_tgt = {
    .name = "lpc1343",
    .rst_delay_tick = MS2TICKS(15),
    .check_delay_us = 2000,
    .glitcher = &g_3v3_rst,
    .sync_checker = lpc1343_sync,
    .unlock_checker = lpc1343_is_unlock,
    .rst_state = lpc_rst_state,
    .board_init = lpc1343_board_init,
    .load_pio_prog = f_3v3_rst_load_pio_prog,
    .glitch = f_3v3_rst_glitch,
};

uint32_t lpc2148_board_init()
{
    uart_init(LPC_UART, 9600);
    gpio_set_function(LPC_PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(LPC_PIN_UART_RX, GPIO_FUNC_UART);
}

uint32_t lpc1343_board_init()
{
    uart_init(LPC_UART, 115200);
    gpio_set_function(LPC_PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(LPC_PIN_UART_RX, GPIO_FUNC_UART);
}

uint32_t lpc2148_sync()
{      
    clr_uart_fifo(LPC_UART);
    if (!uart_send_wait(LPC_UART, "?\r\n", "Synchronized\r\n", LPC_UART_WAIT_US)) {return 0;}
    if (!uart_send_wait(LPC_UART, "Synchronized\r\n", "Synchronized\r\nOK\r\n", LPC_UART_WAIT_US)) {return 0;}
    if (!uart_send_wait(LPC_UART, "1000\r\n", "1000\r\nOK\r\n", LPC_UART_WAIT_US)) {return 0;}
    return 1;           
}

uint32_t lpc1343_sync()
{      
    clr_uart_fifo(LPC_UART);
    if (!uart_send_wait(LPC_UART, "?\r\n", "Synchronized\r\n", LPC_UART_WAIT_US)) {return 0;}
    if (!uart_send_wait(LPC_UART, "Synchronized\r\n", "Synchronized\rOK\r\n", LPC_UART_WAIT_US)) {return 0;}
    if (!uart_send_wait(LPC_UART, "1000\r\n", "1000\rOK\r\n", LPC_UART_WAIT_US)) {return 0;}
    return 1;     
}

uint32_t lpc2148_is_unlock()
{
    if (lpc2148_sync())
    {         
        if (uart_send_wait(LPC_UART, "R 0 4\r\n", "R 0 4\r\n0\r\n", LPC_UART_WAIT_US))
        {       
            uart_puts(LPC_UART, "OK\r\n");  
            clr_uart_fifo(RH850_UART);
            gpio_init(LPC_PIN_UART_RX);
            gpio_set_dir(LPC_PIN_UART_RX, GPIO_IN);
            gpio_init(LPC_PIN_UART_TX);
            gpio_set_dir(LPC_PIN_UART_TX, GPIO_IN);   
            log_info_str("UNLOCK", 6); 
            return 1;
        }
        log_info_str("LOCK", 4);
    }
    else
    {
        log_info_str("ERROR", 5);
    }    
    return 0;
}

uint32_t lpc1343_is_unlock()
{
    if (lpc1343_sync())
    {   
        if (uart_send_wait(LPC_UART, "R 0 4\r\n", "R 0 4\r0\r\n", LPC_UART_WAIT_US))
        {       
            uart_puts(LPC_UART, "OK\r\n");    
            clr_uart_fifo(RH850_UART);
            gpio_init(LPC_PIN_UART_RX);
            gpio_set_dir(LPC_PIN_UART_RX, GPIO_IN);
            gpio_init(LPC_PIN_UART_TX);
            gpio_set_dir(LPC_PIN_UART_TX, GPIO_IN);
            log_info_str("UNLOCK", 6);
            return 1;
        }
        log_info_str("LOCK", 4);
    }
    else
    {
        log_info_str("ERROR", 5);
    }
    return 0;
}

uint32_t lpc_rst_state()
{
    target_nrst(false);
    ctrl_3v3_pin_set(true);
    sleep_us(2000);
    target_nrst(true);
    sleep_us(2000);
}