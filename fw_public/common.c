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
    pio_clear_instruction_memory(DEFAULT_FREE_PIO);
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

uint32_t set_level_shifter(VOLT_SEL volt)
{
    gpio_init(GP14_TRIG_PWR_SEL_0);
    gpio_set_dir(GP14_TRIG_PWR_SEL_0, GPIO_OUT);
    gpio_put(GP14_TRIG_PWR_SEL_0, BIT_CHECK(volt, 0));

    gpio_init(GP15_TRIG_PWR_SEL_1);
    gpio_set_dir(GP15_TRIG_PWR_SEL_1, GPIO_OUT);
    gpio_put(GP15_TRIG_PWR_SEL_1, BIT_CHECK(volt, 1));
}

VOLT_SEL get_level_shifter()
{
    return (gpio_get(GP15_TRIG_PWR_SEL_1) << 1) | gpio_get(GP14_TRIG_PWR_SEL_0);
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
    if (!mutex_app_log_info)
    {
        app.log_info->type = LOG_INFO_STR;
        if (size > LEN_LOG_INFO_MAX)
            size = LEN_LOG_INFO_MAX;
        app.log_info->len = size;
        strncpy(app.log_info->data, str, size);
    }
}

void log_info_raw(uint8_t* data, uint8_t size)
{
    if (!mutex_app_log_info)
    {
        app.log_info->type = LOG_INFO_RAW;
        if (size > LEN_LOG_INFO_MAX)
            size = LEN_LOG_INFO_MAX;
        app.log_info->len = size;
        memcpy(app.log_info->data, data, size);
    }
}

uint32_t get_uart_edges(const uint8_t* data, size_t length) 
{
    int count = 0;
    int prev_bit = -1;
    for (size_t i = 0; i < length; i++) {
        for (int bit = 0; bit < 8; bit++) 
        {
            int current_bit = (data[i] >> bit) & 1;
            if (prev_bit == 0 && current_bit == 1) 
            {
                count++;
            }
            prev_bit = current_bit;
        }
        prev_bit = -1;
    }

    uint32_t count2 = 0;
    for (size_t i = 0; i < length; i++) {
        if (data[i] & 0x01) {
            count2++;
        }
        if (!(data[i] & 0x80)) {
            count2++;
        }
    }

    return count + count2;
}

// Write period to the input shift register
void pio_pwm_set_period(PIO pio, uint sm, uint32_t period) {
    pio_sm_set_enabled(pio, sm, false);
    pio_sm_put_blocking(pio, sm, period);
    pio_sm_exec(pio, sm, pio_encode_pull(false, false));
    pio_sm_exec(pio, sm, pio_encode_out(pio_isr, 32));
    pio_sm_set_enabled(pio, sm, true);
}

// Write `level` to TX FIFO. State machine will copy this into X.
void pio_pwm_set_level(PIO pio, uint sm, uint32_t level) {
    pio_sm_put_blocking(pio, sm, level);
}

void pwm_init()
{
    PIO pio = DEFAULT_FREE_PIO;
    int sm = DEFAULT_FREE_SM;
    uint offset = pio_add_program(pio, &pwm_program);
    pwm_program_init(pio, sm, offset, GP2);
    pio_pwm_set_period(pio, sm, (1u << 15) + 800);
    pio_pwm_set_level(pio, sm, 400); // 4.8us for 250MHz
}

void emfi_trig_pin_init()
{
    gpio_init(EMFI_READY_PIN);
    gpio_set_dir(EMFI_READY_PIN, GPIO_IN);
}

uint8_t is_emfi_ready()
{
    return gpio_get(EMFI_READY_PIN);
}
