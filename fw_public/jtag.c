#include "includes/jtag.h"

void jtag_init() {
    gpio_pin_set(TCK_PIN, false);
    gpio_pin_set(TMS_PIN, false);
    gpio_pin_set(TDI_PIN, false);

    gpio_init(TDO_PIN);
    gpio_set_dir(TDO_PIN, GPIO_IN);
}

void jtag_delay() {
    // sleep_us(10);
    sleep_us(50);
}

void jtag_clock_pulse() {
    gpio_put(TCK_PIN, 1);
    jtag_delay();
    gpio_put(TCK_PIN, 0);
    jtag_delay();
}


void jtag_reset_tap() {
    gpio_put(TMS_PIN, 1);
    for (int i = 0; i < 5; i++) {
        jtag_clock_pulse();
    }
}

void jtag_enter_shift_dr() {
    gpio_put(TMS_PIN, 1); // Run-Test/Idle -> Select-DR-Scan
    jtag_clock_pulse();
    gpio_put(TMS_PIN, 0); // Select-DR-Scan -> Capture-DR
    jtag_clock_pulse();
    gpio_put(TMS_PIN, 0); // Capture-DR -> Shift-DR
    jtag_clock_pulse();
}


void jtag_exit_shift_dr() {
    gpio_put(TMS_PIN, 1); // Exit1-DR -> Update-DR
    jtag_clock_pulse();
    gpio_put(TMS_PIN, 0); // Update-DR -> Run-Test/Idle
    jtag_clock_pulse();
}

void jtag_tck_pulses(uint32_t pulses)
{
    for (uint32_t i = 0; i < pulses; i++)
        jtag_clock_pulse();
}

uint64_t jtag_shift_bit(uint64_t data, uint8_t len) 
{
    uint8_t tmp;
    uint64_t resp = 0;
    for (int i = 0; i < len; i++) {
        gpio_put(TDI_PIN, (data >> i) & 1);
        tmp = gpio_get(TDO_PIN);
        resp |= ((uint64_t)tmp << i);
        gpio_put(TMS_PIN, i == len - 1);
        jtag_clock_pulse();
    }
    return resp;
}

void jtag_enter_shift_ir() {
    gpio_put(TMS_PIN, 1); // Run-Test/Idle -> Select-IR-Scan
    jtag_clock_pulse();
    gpio_put(TMS_PIN, 1); // Select-IR-Scan -> Select-IR-Scan
    jtag_clock_pulse();
    gpio_put(TMS_PIN, 0); // Select-IR-Scan -> Capture-IR 
    jtag_clock_pulse();
    gpio_put(TMS_PIN, 0); // Capture-IR -> Shift-IR
    jtag_clock_pulse();
}

void jtag_exit_shift_ir() {
    gpio_put(TMS_PIN, 1); // Exit1-IR -> Update-IR
    jtag_clock_pulse();
    gpio_put(TMS_PIN, 0); // Update-IR -> Run-Test/Idle
    jtag_clock_pulse();
}

// Считывание 32-битного IDCODE
uint32_t jtag_read_idcode() 
{
    uint32_t idcode = 0;
    uint32_t idcode1 = 0;
    uint64_t resp = 0;

    jtag_reset_tap();
    gpio_put(TMS_PIN, 0); // Test-Logic-Reset -> Run-Test/Idle
    jtag_clock_pulse();

    jtag_enter_shift_dr();
    resp = jtag_shift_bit((uint64_t)0x0, 64);
    idcode = resp & 0xFFFFFFFF;
    idcode1 = resp >> 32;
    jtag_exit_shift_dr() ;
    return idcode;
}
/////////////////////////////////////////////////////////////////////
