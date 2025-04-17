#include "nrf52.h"

target_t nrf52_mosfet_tgt = {
    .name = "nrf52_mosfet",
    .rst_delay_tick = 800000,
    .check_delay_us = 30000,
    .ap_id = 0x24770011,
    .glitcher = &g_gnd_3v3,
    .sync_checker = nrf52_sync,
    .unlock_checker = nrf52_is_unlock,
    .rst_state = nrf52_mosfet_rst_state,
    .board_init = nrf52_board_init,
    .load_pio_prog = gnd_3v3_load_pio_prog,
    .glitch = gnd_3v3_glitch,
};

uint32_t nrf52_board_init()
{
    swd_ext_set(false);
    ctrl_ext_vcc_3v3_ln_set(false);
    ctrl_3v3_pin_set(false);
    init_swd_pico();
    sleep_us(500);
} 

uint32_t nrf52_sync()
{
    target_t *tar = app.target;
    uint32_t ap_id = ahb_ap_check();
    ap_id = ahb_ap_check();
    ap_id = ahb_ap_check();
    ap_id = ahb_ap_check();
    printf("Target ID %x\r\n", ap_id);
    uint32_t new_dword = (ap_id & 0x000000ff) << 24 | (ap_id & 0x0000ff00) << 8 | (ap_id & 0x00ff0000) >> 8 | (ap_id & 0xff000000) >> 24;
    log_info_raw((uint8_t*)(&new_dword), 4);
    if (ap_id == tar->ap_id || ap_id == AP_ID_LOCK)
        return 1;
    return 0;
}

uint32_t nrf52_is_unlock()
{    
    target_t *tar = app.target;
    uint32_t ap_id = ahb_ap_check();
    ap_id = ahb_ap_check();
    ap_id = ahb_ap_check();
    ap_id = ahb_ap_check();
    printf("Target ID %x\r\n", ap_id);
    uint32_t new_dword = (ap_id & 0x000000ff) << 24 | (ap_id & 0x0000ff00) << 8 | (ap_id & 0x00ff0000) >> 8 | (ap_id & 0xff000000) >> 24;
    log_info_raw((uint8_t*)(&new_dword), 4);
    if (ap_id == tar->ap_id)
        return 1;
    return 0;
}

uint32_t nrf52_mosfet_rst_state()
{
    ctrl_3v3_pin_set(false);
    sleep_us(100000);
    ctrl_3v3_pin_set(true);
    sleep_us(100);
    return 1;
}
