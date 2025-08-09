#include "includes/glitcher.h"
#include "includes/common.h"

glitcher_t g_3v3_rst = {    
    .pio = DEFAULT_GLITCHER_PIO,
    .pio_sm = DEFAULT_GLITCHER_SM,
    .pio_program = p_3v3_rst_program,
    .pio_program_entry = p_3v3_rst_offset_entry,
    .pio_program_load_offset = 0,
    .program_get_default_config = p_3v3_rst_program_get_default_config,
};

glitcher_t g_gnd_cnt = {    
    .pio = DEFAULT_GLITCHER_PIO,
    .pio_sm = DEFAULT_GLITCHER_SM,
    .pio_program = p_gnd_cnt_program,
    .pio_program_entry = p_gnd_cnt_offset_entry,
    .pio_program_load_offset = 0,
    .program_get_default_config = p_gnd_cnt_program_get_default_config,
};

glitcher_t g_gnd_cnt_2g = {    
    .pio = DEFAULT_GLITCHER_PIO,
    .pio_sm = DEFAULT_GLITCHER_SM,
    .pio_program = p_gnd_cnt_2g_program,
    .pio_program_entry = p_gnd_cnt_2g_offset_entry,
    .pio_program_load_offset = 0,
    .program_get_default_config = p_gnd_cnt_2g_program_get_default_config,
};

glitcher_t g_gnd_gpio = {    
    .pio = DEFAULT_GLITCHER_PIO,
    .pio_sm = DEFAULT_GLITCHER_SM,
    .pio_program = p_gnd_gpio_program,
    .pio_program_entry = p_gnd_gpio_offset_entry,
    .pio_program_load_offset = 0,
    .program_get_default_config = p_gnd_gpio_program_get_default_config,
};

glitcher_t g_gnd_3v3 = {    
    .pio = DEFAULT_GLITCHER_PIO,
    .pio_sm = DEFAULT_GLITCHER_SM,
    .pio_program = p_gnd_3v3_program,
    .pio_program_entry = p_gnd_3v3_offset_entry,
    .pio_program_load_offset = 0,
    .program_get_default_config = p_gnd_3v3_program_get_default_config,
};

glitcher_t g_3v3_cnt = {    
    .pio = DEFAULT_GLITCHER_PIO,
    .pio_sm = DEFAULT_GLITCHER_SM,
    .pio_program = p_3v3_cnt_program,
    .pio_program_entry = p_3v3_cnt_offset_entry,
    .pio_program_load_offset = 0,
    .program_get_default_config = p_3v3_cnt_program_get_default_config,
};


/////////////////////////////////////////////////////
// for STM, RH850
// load po function for glitcher_t g_gnd_cnt
uint32_t gnd_cnt_load_pio_prog()
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    glc->pio_program_load_offset = pio_add_program(glc->pio, &glc->pio_program);
    pio_gpio_init(glc->pio, GP28_ADC2);
    pio_gpio_init(glc->pio, GP5_MOSFET_CTRL);
    pio_sm_config c = glc->program_get_default_config(glc->pio_program_load_offset);
    sm_config_set_in_pins(&c, GP28_ADC2);
    sm_config_set_sideset_pins(&c, GP5_MOSFET_CTRL);
    sm_config_set_out_shift(&c, false, false, 32);
    sm_config_set_clkdiv(&c, 1);
    pio_sm_set_consecutive_pindirs(glc->pio, glc->pio_sm, GP5_MOSFET_CTRL, 1, true);
    pio_sm_init(glc->pio, glc->pio_sm, glc->pio_program_load_offset, &c);
    pio_sm_set_enabled(glc->pio, glc->pio_sm, true);
    sleep_us(500);
    return 1;
};

// glitch function for glitcher_t g_gnd_cnt
uint32_t gnd_cnt_glitch(uint32_t offset, uint32_t width)
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    tar->rst_state();
    if (tar->sync_checker())
    {
        pio_sm_exec_wait_blocking(glc->pio, glc->pio_sm, pio_encode_jmp(glc->pio_program_load_offset + glc->pio_program_entry));
        pio_sm_put_blocking(glc->pio, glc->pio_sm, tar->count_edge_pio); // clk count
        pio_sm_put_blocking(glc->pio, glc->pio_sm, offset);
        pio_sm_put_blocking(glc->pio, glc->pio_sm, width);
        sleep_us(tar->check_delay_us);
    }
}
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// for nrf52 with MOSFET
// load po function for glitcher_t g_gnd_3v3
uint32_t gnd_3v3_load_pio_prog()
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    glc->pio_program_load_offset = pio_add_program(glc->pio, &glc->pio_program);
    pio_gpio_init(glc->pio, GP3_TARGET_RESET);
    pio_gpio_init(glc->pio, GP4_3V3_CTRL);
    pio_gpio_init(glc->pio, GP5_MOSFET_CTRL);
    pio_sm_set_consecutive_pindirs(glc->pio, glc->pio_sm, GP3_TARGET_RESET, 3, true);
    pio_sm_config c = glc->program_get_default_config(glc->pio_program_load_offset);
    sm_config_set_sideset_pins(&c, GP3_TARGET_RESET);
    sm_config_set_out_shift(&c, false, false, 32);
    sm_config_set_clkdiv(&c, 1);
    sm_config_set_set_pins(&c, GP3_TARGET_RESET, 3);
    pio_sm_init(glc->pio, glc->pio_sm, glc->pio_program_load_offset, &c);
    pio_sm_set_enabled(glc->pio, glc->pio_sm, true);
}

// glitch function for glitcher_t g_gnd_3v3
uint32_t gnd_3v3_glitch(uint32_t offset, uint32_t width)
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    pio_sm_exec_wait_blocking(glc->pio, glc->pio_sm, pio_encode_jmp(glc->pio_program_load_offset + glc->pio_program_entry));
    pio_sm_put_blocking(glc->pio, glc->pio_sm, tar->rst_delay_tick);
    pio_sm_put_blocking(glc->pio, glc->pio_sm, offset);
    pio_sm_put_blocking(glc->pio, glc->pio_sm, width);
    while (!pio_interrupt_get(glc->pio, 0))
    {
        tight_loop_contents();
    }
    pio_interrupt_clear(glc->pio, 0);
    sleep_us(tar->check_delay_us);
}
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// for LPC
// load po function for glitcher_t g_3v3_rst
uint32_t f_3v3_rst_load_pio_prog()
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    glc->pio_program_load_offset = pio_add_program(glc->pio, &glc->pio_program);
    pio_gpio_init(glc->pio, GP3_TARGET_RESET);
    pio_gpio_init(glc->pio, GP4_3V3_CTRL);
    pio_gpio_init(glc->pio, GP5_MOSFET_CTRL);
    pio_sm_set_consecutive_pindirs(glc->pio, glc->pio_sm, GP3_TARGET_RESET, 3, true);
    pio_sm_config c = glc->program_get_default_config(glc->pio_program_load_offset);
    sm_config_set_sideset_pins(&c, GP3_TARGET_RESET);
    sm_config_set_out_shift(&c, false, false, 32);
    sm_config_set_clkdiv(&c, 1);
    sm_config_set_set_pins(&c, GP3_TARGET_RESET, 3);
    pio_sm_init(glc->pio, glc->pio_sm, glc->pio_program_load_offset, &c);
    pio_sm_set_enabled(glc->pio, glc->pio_sm, true);
}

// glitch function for glitcher_t g_3v3_rst
uint32_t f_3v3_rst_glitch(uint32_t offset, uint32_t width)
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    pio_sm_exec_wait_blocking(glc->pio, glc->pio_sm, pio_encode_jmp(glc->pio_program_load_offset + glc->pio_program_entry));    
    pio_sm_put_blocking(glc->pio, glc->pio_sm, tar->rst_delay_tick);
    pio_sm_put_blocking(glc->pio, glc->pio_sm, offset);
    pio_sm_put_blocking(glc->pio, glc->pio_sm, width);
    while(!pio_interrupt_get(glc->pio, 0)) {
        tight_loop_contents();
    }
    pio_interrupt_clear(glc->pio, 0);  
    sleep_us(tar->check_delay_us); 
}
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// for nrf52 with MUX glitch
// load po function for glitcher_t g_gnd_gpio
uint32_t gnd_gpio_load_pio_prog()
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    glc->pio_program_load_offset = pio_add_program(glc->pio, &glc->pio_program);
    pio_gpio_init(glc->pio, GP3_TARGET_RESET);
    pio_gpio_init(glc->pio, GP4_3V3_CTRL);
    pio_gpio_init(glc->pio, GP5_MOSFET_CTRL);
    pio_sm_set_consecutive_pindirs(glc->pio, glc->pio_sm, GP3_TARGET_RESET, 3, true);
    pio_sm_config c = glc->program_get_default_config(glc->pio_program_load_offset);
    sm_config_set_sideset_pins(&c, GP3_TARGET_RESET);
    sm_config_set_out_shift(&c, false, false, 32);
    sm_config_set_clkdiv(&c, 1);
    sm_config_set_set_pins(&c, GP3_TARGET_RESET, 3);
    pio_sm_init(glc->pio, glc->pio_sm, glc->pio_program_load_offset, &c);
    pio_sm_set_enabled(glc->pio, glc->pio_sm, true);    
}

// glitch function for glitcher_t g_gnd_gpio
uint32_t gnd_gpio_glitch(uint32_t offset, uint32_t width)
{    
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    pio_sm_exec_wait_blocking(glc->pio, glc->pio_sm, pio_encode_jmp(glc->pio_program_load_offset + glc->pio_program_entry));    
    pio_sm_put_blocking(glc->pio, glc->pio_sm, tar->rst_delay_tick);
    pio_sm_put_blocking(glc->pio, glc->pio_sm, offset);
    pio_sm_put_blocking(glc->pio, glc->pio_sm, width);
    while(!pio_interrupt_get(glc->pio, 0)) {
        tight_loop_contents();
    }
    pio_interrupt_clear(glc->pio, 0);  
    sleep_us(tar->check_delay_us); 
}
/////////////////////////////////////////////////////


/////////////////////////////////////////////////////
// for GD32
// load pio function for glitcher_t g_3v3_cnt
uint32_t f_3v3_cnt_load_pio_prog()
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;
    glc->pio_program_load_offset = pio_add_program(glc->pio, &glc->pio_program);
    pio_gpio_init(glc->pio, GP28_ADC2);
    pio_gpio_init(glc->pio, GP3_TARGET_RESET);
    pio_gpio_init(glc->pio, GP4_3V3_CTRL);
    pio_gpio_init(glc->pio, GP5_MOSFET_CTRL);
    pio_sm_set_consecutive_pindirs(glc->pio, glc->pio_sm, GP3_TARGET_RESET, 3, true);
    pio_sm_config c = glc->program_get_default_config(glc->pio_program_load_offset);
    sm_config_set_in_pins(&c, GP28_ADC2);
    sm_config_set_sideset_pins(&c, GP3_TARGET_RESET);
    sm_config_set_out_shift(&c, false, false, 32);
    sm_config_set_clkdiv(&c, 1);
    pio_sm_set_consecutive_pindirs(glc->pio, glc->pio_sm, GP3_TARGET_RESET, 3, true);
    pio_sm_init(glc->pio, glc->pio_sm, glc->pio_program_load_offset, &c);
    pio_sm_set_enabled(glc->pio, glc->pio_sm, true);
    sleep_us(500);
    return 1;
};

// glitch function for glitcher_t g_3v3_cnt
uint32_t f_3v3_cnt_glitch(uint32_t offset, uint32_t width)
{
    target_t *tar = app.target;
    glitcher_t *glc = tar->glitcher;

    pio_sm_exec_wait_blocking(glc->pio, glc->pio_sm, pio_encode_jmp(glc->pio_program_load_offset + glc->pio_program_entry));
    sleep_ms(10);
    pio_sm_put_blocking(glc->pio, glc->pio_sm, 0xAA);
    sleep_ms(200);
    // tar->rst_state();
    if (tar->sync_checker())
    {
        // pio_sm_exec_wait_blocking(glc->pio, glc->pio_sm, pio_encode_jmp(glc->pio_program_load_offset + glc->pio_program_entry));
        pio_sm_put_blocking(glc->pio, glc->pio_sm, tar->count_edge_pio); // clk count
        pio_sm_put_blocking(glc->pio, glc->pio_sm, offset);
        pio_sm_put_blocking(glc->pio, glc->pio_sm, width);
        sleep_us(tar->check_delay_us);
    }
}
/////////////////////////////////////////////////////