#include "rh850.h"

target_t rh850_tgt = {
        .name = "rh850",
        .rst_delay_tick = 100000000,
        .check_delay_us = 100,        
        .count_edge_pio = 8, // clk count
        .board_init = rh850_board_init,
        .sync_checker = rh850_sync,
        .rst_state = rh850_reset_state,
        .glitcher = &g_gnd_cnt,
        .load_pio_prog = gnd_cnt_load_pio_prog,
        .glitch = gnd_cnt_glitch,
        .unlock_checker = rh850_is_unlock,
};

uint32_t rh850_board_init()
{
    pio_clear_instruction_memory(DEFAULT_GLITCHER_PIO);
    uart_init(RH850_UART, 9600);
    gpio_set_function(RH850_PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(RH850_PIN_UART_RX, GPIO_FUNC_UART);
    sleep_us(100);
};

uint32_t rh850_sync()
{
    uint8_t a, b, c;

    clr_uart_fifo(RH850_UART);
    uint8_t tx_data1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x55}; 
    uart_tx_mem(RH850_UART, tx_data1, sizeof(tx_data1));
    sleep_ms(50);

    a = getc_uart(RH850_UART, 1000);
    b = getc_uart(RH850_UART, 1000);
    if (a == 0xC1 || b == 0xC1)
    {  
        // printf("--------- get C1\r\n");
        clr_uart_fifo(RH850_UART);
        uint8_t tx_data3[] = {0x01, 0x00, 0x01, 0x38, 0xC7, 0x03}; 
        uart_tx_mem(RH850_UART, tx_data3, sizeof(tx_data3));
        sleep_ms(50); // 50

        clr_uart_fifo(RH850_UART);
        uint8_t tx_data4[] = {0x81, 0x00, 0x01, 0x38, 0xC7, 0x03}; 
        uart_tx_mem(RH850_UART, tx_data4, sizeof(tx_data4));
        sleep_ms(50); //50

        a = getc_uart(RH850_UART, 1000);
        b = getc_uart(RH850_UART, 1000);
        c = getc_uart(RH850_UART, 1000);
        // printf("--------- get dev type %02x %02x %02x\r\n", a, b, c); // synced dev get type
        if(a == 0x81)
        {            
            clr_uart_fifo(RH850_UART); // ok
            uint8_t tx_data5[] = {0x01, 0x00, 0x09, 0x32, 0x00, 0xf4, 0x24, 0x00, 0x04, 0xC4, 0xB4, 0x00, 0x31, 0x03}; 
            uart_tx_mem(RH850_UART, tx_data5, sizeof(tx_data5));
            sleep_ms(30); // 50

            a = getc_uart(RH850_UART, 1000);
            b = getc_uart(RH850_UART, 1000);
            c = getc_uart(RH850_UART, 1000);
            // printf("--------- get set freq %02x %02x %02x\r\n", a, b, c);
            if(a == 0x81)
            {
                clr_uart_fifo(RH850_UART);
                uint8_t tx_data6[] = {0x81, 0x00, 0x01, 0x32, 0xCD, 0x03}; 
                uart_tx_mem(RH850_UART, tx_data6, sizeof(tx_data6));
                sleep_ms(30); //50

                a = getc_uart(RH850_UART, 1000);
                b = getc_uart(RH850_UART, 1000);
                c = getc_uart(RH850_UART, 1000);
                // printf("--------- get freq %02x %02x %02x\r\n", a, b, c); // synced dev get type
                if(a == 0x81)
                {
                    clr_uart_fifo(RH850_UART);
                    uint8_t tx_data7[] = {0x01, 0x00, 0x05, 0x34, 0x00, 0x00, 0x25, 0x80, 0x22, 0x03}; 
                    uart_tx_mem(RH850_UART, tx_data7, sizeof(tx_data7));
                    sleep_ms(20); //20

                    a = getc_uart(RH850_UART, 1000);
                    b = getc_uart(RH850_UART, 1000);
                    c = getc_uart(RH850_UART, 1000);
                    // printf("--------- get set baudrate %02x %02x %02x\r\n", a, b, c); 
                    if(a == 0x81)
                    {
                        sleep_ms(1);
                        clr_uart_fifo(RH850_UART);
                        printf("rh850 synced\r\n");
                        return 1;
                    }
                }                
            }
        }    
    }
    log_info_str("SYNC_FAIL", 9);
    printf("Sync rh850 fail <<<----------------\r\n");
    return 0;
}


uint32_t rh850_check_rp()
{
    // printf("--------- rh850_check_rp\r\n");
    if(rh850_sync())
    {
        // printf("--------- sync OK\r\n");
        uint8_t tmp[0x10];    
        uint8_t tx_data2[] = {0x01, 0x00, 0x01, 0x00, 0xFF, 0x03};    
        uart_tx_mem(RH850_UART, tx_data2, sizeof(tx_data2));
        sleep_ms(10);
        tmp[0] = getc_uart(RH850_UART, 1000);
        if (tmp[0] == 0x81)
        {
            tmp[1] = getc_uart(RH850_UART, 1000);
            tmp[2] = getc_uart(RH850_UART, 1000);
            tmp[3] = getc_uart(RH850_UART, 1000);
            tmp[4] = getc_uart(RH850_UART, 1000);
            tmp[5] = getc_uart(RH850_UART, 1000);
            tmp[6] = getc_uart(RH850_UART, 1000);
            tmp[7] = getc_uart(RH850_UART, 1000);   
            log_info_raw(tmp, 8);    
            // printf("--------- inquiry OK\r\n");
            // printf("%02x %02x %02x %02x %02x %02x %02x %02x\r\n", tmp[0], tmp[1], tmp[2], tmp[3],tmp[4], tmp[5], tmp[6], tmp[7]);
            if (tmp[1] == 0x00 && tmp[2] == 0x01 && tmp[3] == 0x00 && tmp[4] == 0xFF) // if unlock
            {
                ///////////////////////////////////////////////////////////////  
                uint8_t tx_data3[] = {0x01, 0x00, 0x01, 0x21, 0xDE, 0x03};    
                uart_tx_mem(RH850_UART, tx_data3, sizeof(tx_data3));
                sleep_ms(10);
                tmp[0] = getc_uart(RH850_UART, 1000);
                if (tmp[0] == 0x81)
                {
                    tmp[1] = getc_uart(RH850_UART, 1000);
                    tmp[2] = getc_uart(RH850_UART, 1000);
                    tmp[3] = getc_uart(RH850_UART, 1000);
                    tmp[4] = getc_uart(RH850_UART, 1000);
                    tmp[5] = getc_uart(RH850_UART, 1000);         
                    log_info_raw(tmp, 6);
                    // printf("--------- cmd 21 OK\r\n");
                    // printf("%02x %02x %02x %02x %02x %02x\r\n", tmp[0], tmp[1], tmp[2], tmp[3], tmp[4], tmp[5]);
                    if (tmp[1] == 0x00 && tmp[2] == 0x01 && tmp[3] == 0x21 && tmp[4] == 0xDE && tmp[5] == 0x03) // if resp cmd 21 ok
                    {           
                        uart_tx_mem(RH850_UART, tmp, 6);
                        sleep_ms(10);
                        tmp[0] = getc_uart(RH850_UART, 1000);
                        tmp[1] = getc_uart(RH850_UART, 1000);
                        tmp[2] = getc_uart(RH850_UART, 1000);
                        tmp[3] = getc_uart(RH850_UART, 1000);
                        tmp[4] = getc_uart(RH850_UART, 1000);
                        // printf("%02x %02x %02x %02x %02x %02x\r\n", tmp[0], tmp[1], tmp[2], tmp[3],tmp[4], tmp[5]);
                        printf("%02x\r\n", tmp[4]);
                        if (tmp[0] == 0x81 && tmp[1] == 0x00 && tmp[2] == 0x02 && tmp[3] == 0x21)
                        {
                            if (tmp[4] & 0x80)
                            {
                                log_info_str("READ_OK", 7);
                                clr_uart_fifo(RH850_UART);
                                gpio_init(RH850_PIN_UART_RX);
                                gpio_set_dir(RH850_PIN_UART_RX, GPIO_IN);
                                gpio_init(RH850_PIN_UART_TX);
                                gpio_set_dir(RH850_PIN_UART_TX, GPIO_IN);           
                                return 1;
                            }
                            else
                            {
                                log_info_str("READ_PROT", 9);
                            }
                        }
                    }        
                    clr_uart_fifo(RH850_UART);
                    return 0;
                }
                log_info_str("ERROR", 5);
                printf("Target reset\r\n");
                return 0;
            ///////////////////////////////////////////////////////////// 
            }   
        }
    }
    return 0;
}

uint32_t rh850_reset_state()
{
    ctrl_3v3_pin_set(true);
    target_nrst(false);
    sleep_ms(10);
    target_nrst(true);
    sleep_ms(50);
    return 1;
};

uint32_t rh850_is_unlock()
{
    uint8_t tmp[0x10];    
    uint8_t tx_data2[] = {0x01, 0x00, 0x01, 0x00, 0xFF, 0x03};    
    uart_tx_mem(RH850_UART, tx_data2, sizeof(tx_data2));
    sleep_ms(50);
    tmp[0] = getc_uart(RH850_UART, 1000);
    if (tmp[0] == 0x81)
    {
        tmp[1] = getc_uart(RH850_UART, 1000);
        tmp[2] = getc_uart(RH850_UART, 1000);
        tmp[3] = getc_uart(RH850_UART, 1000);
        tmp[4] = getc_uart(RH850_UART, 1000);
        tmp[5] = getc_uart(RH850_UART, 1000);
        tmp[6] = getc_uart(RH850_UART, 1000);
        tmp[7] = getc_uart(RH850_UART, 1000);    
        
        log_info_raw(tmp, 8);

        printf("%02x %02x %02x %02x %02x %02x %02x %02x\r\n", tmp[0], tmp[1], tmp[2], tmp[3],tmp[4], tmp[5], tmp[6], tmp[7]);
        if (tmp[1] == 0x00 && tmp[2] == 0x01 && tmp[3] == 0x00 && tmp[4] == 0xFF) // if unlock
        {
            clr_uart_fifo(RH850_UART);
            gpio_init(RH850_PIN_UART_RX);
            gpio_set_dir(RH850_PIN_UART_RX, GPIO_IN);
            gpio_init(RH850_PIN_UART_TX);
            gpio_set_dir(RH850_PIN_UART_TX, GPIO_IN);           
            return 1;
        }        
        clr_uart_fifo(RH850_UART);
        return 0;
    }
    log_info_str("ERROR", 5);
    printf("Target reset\r\n");
    return 0;
}