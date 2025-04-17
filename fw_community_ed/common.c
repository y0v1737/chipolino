#include "includes/common.h"

uint8_t uart_buff_rx[UART_BUFF_LEN];

void set_leds_color(uint32_t color1, uint32_t color2)
{
    ws2812_set_pixel(&ws2812_config, color2 & 0xFFFFFF);
    // sleep_us(10); \\ kill interrupt work
    ws2812_set_pixel(&ws2812_config, color1 & 0xFFFFFF);
}

void clear_board()
{
    pio_clear_instruction_memory(DEFAULT_GLITCHER_PIO);
    memset(app.memory, 0x00, MAX_LEN_DUMP_MEM);
}

uint32_t swd_ext_set(bool state)
{
    gpio_init(GP11_SWD_CTRL);
    gpio_set_dir(GP11_SWD_CTRL, GPIO_OUT);
    gpio_put(GP11_SWD_CTRL, state);
}

void trst_pin_set(bool state)
{    
    gpio_init(GP3_TARGET_RESET);
    gpio_set_dir(GP3_TARGET_RESET, GPIO_OUT);
    gpio_put(GP3_TARGET_RESET, state);
}

void target_nrst(bool state)
{    
    gpio_init(GP3_TARGET_RESET);
    gpio_set_dir(GP3_TARGET_RESET, GPIO_OUT);
    gpio_put(GP3_TARGET_RESET, !state);
}

void ctrl_3v3_pin_set(bool state)
{
    gpio_init(GP4_3V3_CTRL);
    gpio_set_dir(GP4_3V3_CTRL, GPIO_OUT);
    gpio_put(GP4_3V3_CTRL, state);
}

void ctrl_ext_vcc_3v3_ln_set(bool state)
{
    gpio_init(GP6_VCC_EXT_3V3_LN_CTRL);
    gpio_set_dir(GP6_VCC_EXT_3V3_LN_CTRL, GPIO_OUT);
    gpio_put(GP6_VCC_EXT_3V3_LN_CTRL, state);
}

void gpio_pin_set(uint32_t gp, bool state)
{
    gpio_init(gp);
    gpio_set_dir(gp, GPIO_OUT);
    gpio_put(gp, state);
}

uint32_t uart_read_buff(uart_inst_t * uart, uint8_t* buff)
{
    uint32_t cnt_rx = 0;
    while (uart_is_readable(uart)) 
    {
        buff[cnt_rx] = uart_getc(uart);
        cnt_rx++;
        if (cnt_rx == UART_BUFF_LEN-1)
        {
            while (uart_is_readable(uart)) 
                uart_getc(uart);
            return cnt_rx;
        }
    }
    return cnt_rx;
}

uint32_t clr_uart_fifo(uart_inst_t * uart)
{
    while (uart_is_readable(uart)) 
    {
        uart_getc(uart);
    }
}

uint8_t getc_uart(uart_inst_t * uart, uint32_t time_wait)
{
    uint8_t ch;
    sleep_us(time_wait);
    if (uart_is_readable(uart)) 
    {
        ch = uart_getc(uart);
        return ch;
    }
    return 0;
}

uint32_t uart_tx_mem(uart_inst_t * uart, uint8_t* buff_tx, uint32_t size)
{
    for (uint32_t i = 0; i < size; i++)
    {
        uart_putc(uart, buff_tx[i]);
    }
}

uint32_t uart_tx_print(uart_inst_t * uart, uint8_t* buff_tx, uint32_t time_wait)
{
    uint8_t ch;
    uart_puts(uart, buff_tx); 
    sleep_us(time_wait);
    ch = uart_read_buff(uart, uart_buff_rx);
    printf("%s\r\n", uart_buff_rx);
}

uint32_t uart_send_wait(uart_inst_t * uart, uint8_t* buff_tx, uint8_t* buff_rx, uint32_t time_wait)
{
    uint8_t ch;

    uart_puts(uart, buff_tx);
    sleep_us(time_wait);
    ch = uart_read_buff(uart, uart_buff_rx);   
    for (uint i = 0; i < ch; i++)
    {
        printf("%x %c\r\n", uart_buff_rx[i], uart_buff_rx[i]);
    }
    if (!strncmp(uart_buff_rx, buff_rx, strlen(buff_rx)))
    {
        return 1;
    }
    return 0;
}

void log_info_str(uint8_t* str, uint8_t size)
{
    app.log_info->type = LOG_INFO_STR;
    if (size > LEN_LOG_INFO_MAX)
        size = LEN_LOG_INFO_MAX;
    app.log_info->len = size;
    strncpy(app.log_info->data, str, size);
}

void log_info_raw(uint8_t* data, uint8_t size)
{
    app.log_info->type = LOG_INFO_RAW;
    if (size > LEN_LOG_INFO_MAX)
        size = LEN_LOG_INFO_MAX;
    app.log_info->len = size;
    memcpy(app.log_info->data, data, size);
}







