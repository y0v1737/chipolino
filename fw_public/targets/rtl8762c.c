#include "rtl8762c.h"
#include "p_squarewave.pio.h"


#define BOOT_PIN GP26
#define CLOCK_PIN GP2

#define FLASH_MODE 1
#define ROM_MODE 0
#define DEFAULT_CLOCK 5.0

#define BOOT_DELAY 500
#define POWER_OFF_DELAY 100000


target_t rtl8762c_tgt = {
    .name = "rtl8762c",
    .rst_delay_tick = 8000000,
    .check_delay_us = 3000,
    .ap_id = 0x2ba01477,
    .glitcher = &g_gnd_3v3,
    .sync_checker = rtl8762c_sync,
    .unlock_checker = rtl8762c_is_unlock,
    .rst_state = rtl8762c_mosfet_rst_state,
    .board_init = rtl8762c_board_init,
    .load_pio_prog = gnd_3v3_load_pio_prog,
    .glitch = gnd_3v3_glitch,
};

PIO pio = DEFAULT_FREE_PIO;
uint sm = DEFAULT_FREE_SM;
int offset;

int boot_mode = FLASH_MODE;
bool pio_inited = false;

bool setup_pio()
{
    if (pio_inited)
        return 1;
    offset = pio_add_program(pio, &squarewave_program);
    if (offset < 0)
    {
        return 0;
    }
    pio_sm_config c = squarewave_program_get_default_config(offset);
    sm_config_set_sideset_pins(&c, CLOCK_PIN);
    sm_config_set_clkdiv(&c, CLK_MAIN_KHZ / 2000.0 / DEFAULT_CLOCK);
    pio_sm_init(pio, sm, offset, &c);    
    pio_sm_set_consecutive_pindirs(pio, sm, CLOCK_PIN, 1, true);
    pio_gpio_init(pio, CLOCK_PIN);
    return 1;
}

void run_clock()
{
    pio_sm_set_enabled(pio, sm, true);
}

void stop_clock()
{
    pio_sm_set_enabled(pio, sm, false);
}


uint32_t rtl8762c_board_init()
{
    swd_ext_set(false);
    ctrl_ext_vcc_3v3_ln_set(false);
    ctrl_3v3_pin_set(false);
    init_swd_pico();
    gpio_pin_set(BOOT_PIN, boot_mode);
    sleep_us(BOOT_DELAY);
} 

uint32_t rtl8762c_sync()
{
    return 1;
}

uint32_t rtl8762c_is_unlock()
{    
    target_t *tar = app.target;
    uint32_t ap_id = swd_check();
    if (ap_id == tar->ap_id)
    {
        printf("Target ID: 0x%x\r\n", ap_id);
        gpio_init(MCU_PIN_UART_TX);
        gpio_set_dir(MCU_PIN_UART_TX, GPIO_IN);
        gpio_init(MCU_PIN_UART_RX);
        gpio_set_dir(MCU_PIN_UART_RX, GPIO_IN);
        swd_ext_set(true);
        setup_pio();
        run_clock();
        return 1;
    }
    return 0;
}

uint32_t rtl8762c_mosfet_rst_state()
{   
    ctrl_3v3_pin_set(false);
    sleep_us(POWER_OFF_DELAY);
    ctrl_3v3_pin_set(true);
    sleep_us(BOOT_DELAY);
    return 1;
}

uint8_t rtl_uart_read(uint32_t timeout)
{
    if(uart_is_readable_within_us(MCU_UART, timeout))
    {
        uint8_t c = uart_getc(MCU_UART);
        return c;
    }
    return 0;
}


void rtl8762c_on()
{
    // if (!setup_pio())
    // {
    //     printf(">> PIO START FAILED! <<\n");
    //     return;
    // }
    // pio_inited = true;
    // run_clock();
    ctrl_3v3_pin_set(false);
    gpio_init(BOOT_PIN);
    gpio_set_dir(BOOT_PIN, GPIO_OUT);
    gpio_pin_set(BOOT_PIN, ROM_MODE);
    sleep_ms(100);
    ctrl_3v3_pin_set(true);
    target_nrst(true);
    sleep_ms(1000);
    
    gpio_init(BOOT_PIN);
    gpio_set_dir(BOOT_PIN, GPIO_IN);
    gpio_init(MCU_PIN_UART_TX);
    gpio_set_dir(MCU_PIN_UART_TX, GPIO_IN);
    gpio_init(MCU_PIN_UART_RX);
    gpio_set_dir(MCU_PIN_UART_RX, GPIO_IN);

}

void rtl8762c_off()
{
    // stop_clock();
    ctrl_3v3_pin_set(false);
    target_nrst(false);
}

void rtl8762c_cmd_parser(char* cmd)
{
    if (!memcmp(cmd, "ON", sizeof("ON")))
    {
        rtl8762c_on();
    }
    else if (!memcmp(cmd, "OFF", sizeof("OFF")))
    {
        rtl8762c_off();
    }  
}