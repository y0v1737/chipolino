#include "includes/common.h"
#include "targets/lpc.h"
#include "targets/nrf52.h"
#include "targets/stm32f4xx.h"
#include "targets/rh850.h"
#include "tusb.h"
#include <ctype.h>

void user_btn_callback();
uint32_t glitch_loop();

log_info_t log_info = {
    .sig = 0x55,
    .len = 0,
    .type = LOG_INFO_RAW,
};

app_t app = {
    .start_offset = 0,
    .end_offset = 0,
    .start_width = 0,
    .end_width = 0,
    .step = 1,
    .attempt = 1,
    .log_info = &log_info,
};

target_t none_tgt = {
    .name = "none",
    .rst_delay_tick = 0,
    .check_delay_us = 0,
};

target_t *targets_list[] = {
    &none_tgt,
    &stm32f401_spi_rdp1_tgt, &stm32f401_rdp2_tgt,
    &lpc2148_tgt,
    &lpc1343_tgt,
    &nrf52_mosfet_tgt,
    &rh850_tgt,
};

void worker_core1()
{
    uint work_cmd;
    uint32_t fifo_val;

    uint32_t g = multicore_fifo_pop_blocking();
    if (g == FLAG_CORE_VALUE)
    {
        multicore_fifo_push_blocking(FLAG_CORE_VALUE);
    }

    while (1)
    {
        BIT_CLEAR(app.state, STOP_CMD_BIT);
        fifo_val = multicore_fifo_pop_blocking();
        if (fifo_val == 'C')
        {
            printf("rx command C\r\n");
            if (BIT_CHECK(app.state, GLITCH_CMD_BIT))
            {
                printf("glitch start\r\n");
                BIT_CLEAR(app.state, GLITCH_CMD_BIT);
                BIT_SET(app.state, GLITCHING_BIT);
                set_leds_color(WHITE_COLOR, OFF_COLOR);
                if (glitch_loop())
                {
                    BIT_SET(app.state, GLITCH_SUCC_BIT);
                    printf("------------------------\r\n");
                    printf("Glitch OK done\r\n");
                    printf("Offset %d, Width %d\r\n", app.cur_offset, app.cur_width);
                    printf("------------------------\r\n");
                    set_leds_color(GREEN_COLOR, OFF_COLOR);
                }
                else
                {
                    printf("------------------------\r\n");
                    printf("Glitch BAD done\r\n");
                    printf("------------------------\r\n");
                    set_leds_color(RED_COLOR, OFF_COLOR);
                }
                BIT_CLEAR(app.state, GLITCHING_BIT);
            }
        }
    }
}

void register_command(command_t new_command)
{
    command_t *new_command_ptr = &(registered_commands[reg_cmd_len]);
    memcpy(new_command_ptr, &new_command, sizeof(command_t));
    reg_cmd_len++;
}

bool get_string_timeout_us(char (*buffer)[100], uint32_t timeout)
{
    if (tud_cdc_available())
    {
        for (int i = 0; i < 100; i++)
        {
            int usb_in_value = getchar_timeout_us(timeout);
            if (usb_in_value != PICO_ERROR_TIMEOUT)
            {
                char usb_in_char = usb_in_value & 0xff;
                if (!isspace(usb_in_char))
                {

                    (*buffer)[i] = usb_in_char;
                }
                else
                {
                    return true;
                }
            }
            else
            {
                return false;
            }
        }
    }
    else
    {
        return false;
    }
}

void parse_cmd_from_usb()
{
    BIT_CLEAR(app.state, GLITCH_ADDR_CMD_BIT);

    char cur_cmdname[100] = "";
    bool cmd_has_all_args = true;
    uint32_t counter = 0;
    command_t *cur_potential_cmd = &(registered_commands[0]);
    while (!get_string_timeout_us(&cur_cmdname, 100))
    {
        sleep_ms(100);
    }
    printf("%s ", cur_cmdname);
    for (int i = 0; i < reg_cmd_len; i++)
    {
        cur_potential_cmd = &(registered_commands[i]);
        if (strncmp(cur_potential_cmd->name, cur_cmdname, sizeof(cur_potential_cmd->name)) == 0)
        {
            break;
        }
    }
    for (uint8_t cmd_no = 0; cmd_no < cur_potential_cmd->arglen; cmd_no++)
    {
        memset(&command_args[cmd_no][0], 0, sizeof(command_args[cmd_no]));
        if (!get_string_timeout_us(&command_args[cmd_no], 100))
        {
            cmd_has_all_args = false;
            printf("\r\nNeed %d more arguments", cur_potential_cmd->arglen - cmd_no);
            break;
        }
        else
        {
            printf("%s ", command_args[cmd_no]);
        }
    }
    printf("\r\n");
    if (cmd_has_all_args)
    {
        gpio_put(PICO_DEFAULT_LED_PIN, false);        
        cur_potential_cmd->func();
    }
}

void handle_stop()
{
    BIT_SET(app.state, STOP_CMD_BIT);
}

void handle_bootrom()
{
    reset_usb_boot(0, 0);
}

void handle_state()
{
    printf("--- state start ---\r\n");
    printf("state: 0x%x\r\n", app.state);
    printf("cur_offset: %d\r\n", app.cur_offset);
    printf("cur_width: %d\r\n", app.cur_width);
    printf("log_info: ");
    printf("%02x", app.log_info->sig);
    printf("%02x", app.log_info->len);
    printf("%02x", app.log_info->type);
    for (uint8_t i = 0; i < app.log_info->len; i++)
        printf("%02x", app.log_info->data[i]);
    printf("\r\n");
    printf("--- state end ---\r\n");
}

void handle_swd()
{
    if (atoi(command_args[0]) == 1)
        swd_ext_set(true);
    if (atoi(command_args[0]) == 0)
        swd_ext_set(false);
}

void handle_gpio()
{
    if (atoi(command_args[1]) == 1)
        gpio_pin_set(atoi(command_args[0]), true);
    if (atoi(command_args[1]) == 0)
        gpio_pin_set(atoi(command_args[0]), false);
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

void handle_trigger()
{
    if (!strcmp(command_args[0], "1v2"))
        set_level_shifter(V_1V2);
    else if (!strcmp(command_args[0], "1v8"))
        set_level_shifter(V_1V8);
    else if (!strcmp(command_args[0], "3v3"))
        set_level_shifter(V_3V3);
    else
        set_level_shifter(V_Z);
}

uint32_t is_target_support(char *tgt)
{
    uint32_t i = 0;
    for (i = 0; i < LENGTH(targets_list); i++)
    {
        if (!strcmp(targets_list[i]->name, tgt))
        {
            return i;
        }
    }
    return 0;
}

void set_target_by_no(app_t *a, uint32_t tgt_no)
{
    a->target_no = tgt_no;
    a->target = targets_list[tgt_no];
}

void cmd_status_hndr()
{
    printf("--- status start ---\r\n");
    printf("Target: %s\r\n", app.target->name);
    printf("app.target_no: %d\r\n", app.target_no);
    printf("app.state: 0x%x\r\n", app.state);
    printf("reset delay: %d\r\n", app.target->rst_delay_tick);
    printf("check delay: %d\r\n", app.target->check_delay_us);
    printf("start_offset: %d\r\n", app.start_offset);
    printf("end_offset: %d\r\n", app.end_offset);
    printf("start_width: %d\r\n", app.start_width);
    printf("end_width: %d\r\n", app.end_width);

    printf("step: %d\r\n", app.step);
    printf("attempt: %d\r\n", app.attempt);
    printf("cur_offset: %d\r\n", app.cur_offset);
    printf("cur_width: %d\r\n", app.cur_width);

    if (gpio_get(GP11_SWD_CTRL))
        printf("swd_external: true\r\n");
    else
        printf("swd_external: false\r\n");

    VOLT_SEL v = get_level_shifter();
    if (v == V_1V2)
        printf("level shifter: 1.2v\r\n");
    else if (v == V_1V8)
        printf("level shifter: 1.8v\r\n");
    else if (v == V_3V3)
        printf("level shifter: 3.3v\r\n");
    else if (v == V_Z)
        printf("level shifter: Z\r\n");
    else
        printf("level shifter: unknown\r\n");

    printf("--- status end ---\r\n");
}

void handle_status()
{
    cmd_status_hndr();
}

uint32_t glitch_arg_save(app_t *a, char *arg0, char *arg1, char *arg2, char *arg3)
{
    a->start_offset = atoi(arg0);
    a->end_offset = atoi(arg1);
    a->start_width = atoi(arg2);
    a->end_width = atoi(arg3);
    return 1;
}

void err_cmd()
{
    printf("\r\nERROR cmd\r\n");
}

uint32_t glitch_loop()
{
    for (app.cur_offset = app.start_offset; (app.cur_offset < app.end_offset) && (!BIT_CHECK(app.state, STOP_CMD_BIT)); app.cur_offset+=app.step)
    {
        for (app.cur_width = app.start_width; (app.cur_width < app.end_width) && (!BIT_CHECK(app.state, STOP_CMD_BIT)); app.cur_width++)
        {
            for (uint8_t try = 0; try < 1; try+=app.attempt)
            {
                printf("%d, %d\r\n", app.cur_offset, app.cur_width);
                app.target->glitch(app.cur_offset, app.cur_width);                
                if (app.target->unlock_checker())
                {
                    return 1;
                }
            }
        }
    }
    return 0;
}

void handle_glitch()
{
    BIT_CLEAR(app.state, SYNC_BIT);
    BIT_CLEAR(app.state, GLITCH_SUCC_BIT);
    BIT_CLEAR(app.state, GLITCH_ADDR_CMD);
    if (is_target_support(command_args[0]))
    {
        set_target_by_no(&app, is_target_support(command_args[0]));
        if (glitch_arg_save(&app, command_args[1], command_args[2], command_args[3], command_args[4]))
        {
            if (BIT_CHECK(app.state, GLITCH_ADDR_CMD_BIT)) // GLITCH_ADDR
            {
                BIT_SET(app.state, GLITCH_ADDR_CMD);
                app.dump_addr = strtoul(command_args[5], NULL, 16);
            }
            clear_board();
            app.target->board_init();
            app.target->rst_state();
            if (app.target->sync_checker())
            {
                app.target->load_pio_prog();
                BIT_SET(app.state, SYNC_BIT);
                BIT_SET(app.state, GLITCH_CMD_BIT);                
                multicore_fifo_push_blocking('C');
            }
            else
            {
                printf("Target no sync\r\n");
            }
        }
        else
        {
            err_cmd();
        }
    }
    else
    {
        err_cmd();
    }
}

void handle_glitch_addr()
{
    BIT_SET(app.state, GLITCH_ADDR_CMD_BIT);
    handle_glitch();
}

void handle_get_memory()
{
    printf("--- get memory start ---\r\n");
    printf("addr 0x%x\r\n", app.dump_addr);
    printf("mem ");
    for (int i = 0; i < 0x200; i++)
    {
        printf("%02x", app.memory[i]);
    }
    printf("\r\n");
    printf("--- get memory end ---\r\n");
}

void handle_set_rdp()
{
    printf("\r\n%s\r\n", command_args[0]);
    if (atoi(command_args[0]) == 0)
    {
        stm_set_rdp(0xAA);
    }
    else if (atoi(command_args[0]) == 1)
    {
        stm_set_rdp(0xBB);
    }
    else if (atoi(command_args[0]) == 2)
    {
        stm_set_rdp(0xCC);
    }
}

void handle_set_step()
{
    app.step = atoi(command_args[0]);
}

void handle_set_attempt()
{
    app.attempt = atoi(command_args[0]);
}

void handle_self_test()
{
    set_leds_color(OFF_COLOR, OFF_COLOR);
    set_leds_color(OFF_COLOR, OFF_COLOR);
    for(uint8_t i = 0; i < 3; i++)
    {
        gpio_pin_set(GP4_3V3_CTRL, true);
        gpio_pin_set(GP6_VCC_EXT_3V3_LN_CTRL, true);
        gpio_pin_set(GP7_1V2_CTRL, true);
        gpio_pin_set(GP8_1V8_CTRL, true);
        gpio_pin_set(GP9_5V_CTRL, true);
        set_leds_color(PINK_COLOR, PINK_COLOR);
        swd_ext_set(true);
        set_level_shifter(V_1V2);
        sleep_ms(1000);

        set_leds_color(WHITE_COLOR, WHITE_COLOR);
        set_level_shifter(V_1V8);
        sleep_ms(1000);

        set_leds_color(GREEN_COLOR, GREEN_COLOR);
        set_level_shifter(V_3V3);
        sleep_ms(1000);

        gpio_pin_set(GP4_3V3_CTRL, false);
        gpio_pin_set(GP6_VCC_EXT_3V3_LN_CTRL, false);
        gpio_pin_set(GP7_1V2_CTRL, false);
        gpio_pin_set(GP8_1V8_CTRL, false);
        gpio_pin_set(GP9_5V_CTRL, false);
        set_leds_color(OFF_COLOR, OFF_COLOR);
        swd_ext_set(false);
        set_level_shifter(V_Z);
        sleep_ms(1000);
    }    
}

void handle_unknown_cmd()
{
    printf("Unknown command\n");
}

void register_commands()
{
    register_command((command_t){"STOP", 0, &handle_stop});
    register_command((command_t){"GLITCH", 5, &handle_glitch});
    register_command((command_t){"GLITCH_ADDR", 6, &handle_glitch_addr});
    register_command((command_t){"BOOTROM", 0, &handle_bootrom});
    register_command((command_t){"STATE", 0, &handle_state});
    register_command((command_t){"SWD", 1, &handle_swd});
    register_command((command_t){"STEP", 1, &handle_set_step});
    register_command((command_t){"ATTEMPT", 1, &handle_set_attempt});
    register_command((command_t){"GPIO", 2, &handle_gpio});
    register_command((command_t){"TRIG", 1, &handle_trigger});
    register_command((command_t){"STATUS", 0, &handle_status});
    register_command((command_t){"GET_MEMORY", 0, &handle_get_memory});
    register_command((command_t){"GET_RDP", 0, &stm_get_rdp});
    register_command((command_t){"SET_RDP", 1, &handle_set_rdp});
    register_command((command_t){"SELF_TEST", 0, &handle_self_test});
    register_command((command_t){"", 0, &handle_unknown_cmd});
}

void user_btn_callback() {
    // sleep_ms() kill interrupt work and another same func
    // print PANIC string to UART
    if (gpio_get(GP11_SWD_CTRL))
    {
        swd_ext_set(false);
        set_leds_color(PINK_COLOR, OFF_COLOR);
    }        
    else
    {
        swd_ext_set(true);
        set_leds_color(ORANGE_COLOR, OFF_COLOR);        
    }
}

void board_init()
{
    ws2812_init(&ws2812_config);
    swd_ext_set(false);
    set_level_shifter(V_1V8);    
    ctrl_ext_vcc_3v3_ln_set(false);
    target_nrst(false);
    ctrl_3v3_pin_set(false);

    gpio_init(GP28_ADC2);
    gpio_set_dir(GP28_ADC2, GPIO_IN);

    gpio_init(GP22_BTN);
    gpio_set_dir(GP22_BTN, GPIO_IN);
    gpio_set_irq_enabled_with_callback(GP22_BTN, GPIO_IRQ_EDGE_RISE, true, &user_btn_callback);

    register_commands();
    sleep_ms(10);
    set_leds_color(OFF_COLOR, OFF_COLOR);
    set_leds_color(OFF_COLOR, OFF_COLOR);

    gpio_init(GP16_SPI0_RX_UART0_TX);
    gpio_set_dir(GP16_SPI0_RX_UART0_TX, GPIO_IN);
    gpio_init(GP17_SPI0_nCS_UART0_RX);
    gpio_set_dir(GP17_SPI0_nCS_UART0_RX, GPIO_IN); 

    gpio_init(GP12_I2C0_SDA_UART0_TX);
    gpio_set_dir(GP12_I2C0_SDA_UART0_TX, GPIO_IN);
    gpio_init(GP13_I2C0_SCL_UART0_RX);
    gpio_set_dir(GP13_I2C0_SCL_UART0_RX, GPIO_IN); 
}

int main()
{
    set_sys_clock_khz(CLK_MAIN_KHZ, true);

    stdio_usb_init();
    stdio_set_translate_crlf(&stdio_usb, false);
    multicore_launch_core1(worker_core1);

    multicore_fifo_push_blocking(FLAG_CORE_VALUE);
    uint32_t g = multicore_fifo_pop_blocking();
    board_init();
    while (!stdio_usb_connected())
    {
        sleep_ms(100);
    }
    if (g != FLAG_CORE_VALUE)
    {
        printf("Core 1: not start\n");
    }

    while (true)
    {
        parse_cmd_from_usb();
    }
}