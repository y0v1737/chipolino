#include "stm_lib.h"

uint8_t tmp_in_buff[MAX_LEN_SPI_BUF] = {0};
uint8_t cmd_blhello[] = {0x5A, 0x00, 0x55, 0x79};
uint8_t cmd_rmem[] = {0x5A, 0x11, 0xEE};
uint8_t cmd_rmem1[] = {0x00, 0x00, 0x79};
uint8_t cmd_rmem_addr_pkt[] = {0x1f, 0xff, 0x00, 0x00, 0xE0};
uint8_t cmd_rmem_nbyte_pkt[] = {0xFF, 0x00};
uint8_t mbuff[0x200] = {};

void stm_get_rdp()
{
    pio_clear_instruction_memory(DEFAULT_GLITCHER_PIO);
    init_swd_pico();
    stm32_get_rdp();
}

void stm_set_rdp(uint8_t level)
{
    pio_clear_instruction_memory(DEFAULT_GLITCHER_PIO);
    init_swd_pico();
    stm32_set_rdp(level);
}

uint32_t stm_spi_init()
{
    spi_init(MCU_SPI, 1 * 1000 * 1000);
    spi_set_format(MCU_SPI, 8, SPI_CPOL_0, SPI_CPHA_1, SPI_MSB_FIRST);
    gpio_set_function(GP16_SPI0_RX_UART0_TX, GPIO_FUNC_SPI);
    gpio_set_function(GP17_SPI0_nCS_UART0_RX, GPIO_FUNC_SPI);
    gpio_set_function(GP18_SPI0_CLK, GPIO_FUNC_SPI);
    gpio_set_function(GP19_SPI0_TX, GPIO_FUNC_SPI);
}

uint32_t stm_uart_init()
{
    uart_init(MCU_UART, 115200);
    gpio_set_function(MCU_PIN_UART_TX, GPIO_FUNC_UART);
    gpio_set_function(MCU_PIN_UART_RX, GPIO_FUNC_UART);
    uart_set_format(MCU_UART,8,1,UART_PARITY_EVEN);
}

uint32_t stm_spi_sync()
{
    spi_write_read_blocking(MCU_SPI, cmd_blhello, tmp_in_buff, 4);
    if (tmp_in_buff[0] == 0xA5 && tmp_in_buff[2] == 0x79)
    {
        return 1;
    }
    return 0;
}

uint32_t stm_spi_is_unlock()
{
    uint32_t addr = 0x1fff0000;
    if (BIT_CHECK(app.state, GLITCH_ADDR_CMD))
    {
        addr = app.dump_addr1;
    }
    app.dump_addr1 = addr;
    uint8_t addr_1, addr_2, addr_3, addr_4;
    addr_1 = ((addr >> 24) & 0xFF);
    addr_2 = ((addr >> 16) & 0xFF);
    addr_3 = ((addr >> 8) & 0xFF);
    addr_4 = (addr & 0xFF);
    uint8_t xor = (addr_1 ^ addr_2 ^ addr_3 ^ addr_4);
    cmd_rmem_addr_pkt[0] = addr_1;
    cmd_rmem_addr_pkt[1] = addr_2;
    cmd_rmem_addr_pkt[2] = addr_3;
    cmd_rmem_addr_pkt[3] = addr_4;
    cmd_rmem_addr_pkt[4] = xor;
    sleep_us(1000);
    spi_write_read_blocking(MCU_SPI, cmd_rmem, tmp_in_buff, 3);
    spi_write_read_blocking(MCU_SPI, cmd_rmem1, tmp_in_buff, 3);
    if (tmp_in_buff[1] == 0x79)
    {
        log_info_str("ACK_CMD", 7);
        printf("resp cmd = %x\r\n", tmp_in_buff[1]);
        sleep_us(1000);
        spi_write_read_blocking(MCU_SPI, cmd_rmem_addr_pkt, tmp_in_buff, 5);
        spi_write_read_blocking(MCU_SPI, cmd_rmem1, tmp_in_buff, 3);
        if (tmp_in_buff[1] == 0x79)
        {
            printf("resp addr = %x\r\n", tmp_in_buff[1]);
            sleep_us(1000);
            spi_write_read_blocking(MCU_SPI, cmd_rmem_nbyte_pkt, tmp_in_buff, 2);
            spi_write_read_blocking(MCU_SPI, cmd_rmem1, tmp_in_buff, 3);
            if (tmp_in_buff[1] == 0x79)
            {
                printf("resp size = %x\r\n", tmp_in_buff[1]);
                sleep_us(1000);
                spi_write_read_blocking(MCU_SPI, mbuff, mbuff, 257);
                printf("DUMP DATA\r\n");
                for (int i = 0; i < 0x100; i++)
                {
                    app.memory[i] = mbuff[i+1];
                    printf("IND = 0x%02x , val = %02x\r\n", i, app.memory[i]);
                }
                log_info_str("DUMP", 4);
                printf("\r\nDUMP DONE\r\n");
                return 1;
            }
            else
            {
                printf("Err num bytes = %x\r\n", tmp_in_buff[1]);
            }
        }
        else
        {
            printf("Err addr = %x\r\n", tmp_in_buff[1]);
        }
    }
    else if (tmp_in_buff[1] == 0x1F)
    {
        printf("Сmd resp NACK\r\n");
        log_info_str("NACK", 4);
    }
    else
    {
        printf("Сmd resp ERR = %x\r\n", tmp_in_buff[1]);
        log_info_str("ERR", 3);
    }
    return 0;
}

uint32_t stm_uart_sync()
{
    clr_uart_fifo(MCU_UART);
    uart_putc(MCU_UART, 0x7F);
    uint8_t c = getc_uart(MCU_UART, 1000);
    clr_uart_fifo(MCU_UART);
    if (c == 0x79)
        return 1;

    uart_putc(MCU_UART, 0x7F);
    c = getc_uart(MCU_UART, 1000);
    clr_uart_fifo(MCU_UART);
    if (c == 0x79)
        return 1;

    uart_putc(MCU_UART, 0x7F);
    c = getc_uart(MCU_UART, 1000);
    clr_uart_fifo(MCU_UART);
    if (c == 0x79)
        return 1;
    return 0;
}

uint32_t stm_rdp2_board_init()
{
    init_swd_pico();
    // stm_rst_state();
    // swd_init();
    // uint32_t a = swd_check();
    // printf("ap-id 0x%x\r\n", a);
}

uint32_t stm_rdp2_sync()
{
    target_t *tar = app.target;
    swd_init();
    uint32_t ap_id = swd_check();
    printf("ID %x\r\n", ap_id);
    uint32_t new_dword = (ap_id & 0x000000ff) << 24 | (ap_id & 0x0000ff00) << 8 | (ap_id & 0x00ff0000) >> 8 | (ap_id & 0xff000000) >> 24;
    log_info_raw((uint8_t*)(&new_dword), 4);
    return 1;
}

uint32_t stm_uart_is_unlock()
{
    uint8_t c = 0;
    uint8_t d = 0;
    uint8_t e = 0;
    uint32_t addr = 0x1fff0000;
    if (BIT_CHECK(app.state, GLITCH_ADDR_CMD))
    {
        addr = app.dump_addr1;
    }
    app.dump_addr1 = addr;
    uint8_t addr_1, addr_2, addr_3, addr_4;
    addr_1 = ((addr >> 24) & 0xFF);
    addr_2 = ((addr >> 16) & 0xFF);
    addr_3 = ((addr >> 8) & 0xFF);
    addr_4 = (addr & 0xFF);
    uint8_t xor = (addr_1 ^ addr_2 ^ addr_3 ^ addr_4);
    clr_uart_fifo(MCU_UART);
    uart_putc(MCU_UART, 0x11);
    uart_putc(MCU_UART, 0xEE);
    c = getc_uart(MCU_UART, 500);
    d = getc_uart(MCU_UART, 500);
    e = getc_uart(MCU_UART, 500);
    if (c == 0x79 && d == 0x00)
    {
        log_info_str("ACK_CMD", 7);
        printf("resp cmd = %x\r\n", c);
        uart_putc(MCU_UART, addr_1);
        uart_putc(MCU_UART, addr_2);
        uart_putc(MCU_UART, addr_3);
        uart_putc(MCU_UART, addr_4);
        uart_putc(MCU_UART, xor);
        c = getc_uart(MCU_UART, 1000);
        d = getc_uart(MCU_UART, 1000);
        if (c == 0x79 && d == 0x00)
        {
            printf("resp addr = %x\r\n", c);
            uart_putc(MCU_UART, 0xFF);
            uart_putc(MCU_UART, 0x00);
            c = getc_uart(MCU_UART, 400);
            if (c == 0x79)
            {
                printf("resp size = %x\r\n", c);
                printf("DUMP DATA\r\n");
                sleep_us(100);
                for (int i = 0; i < 0x100; i++)
                {               
                    app.memory[i] = uart_getc(MCU_UART);
                    printf("IND = 0x%02x , val = %02x\r\n", i, app.memory[i]);                    
                }
                log_info_str("DUMP", 4);
                printf("\r\nDUMP DONE\r\n");
                return 1;
            }
            printf("err addr = %x %x\r\n", c, d);
            printf("DUMP ERROR\r\n");
        }
        printf("DUMP ERROR\r\n");
        return 0;
    }
    else if (c == 0x1F)
    {
        printf("cmd resp NACK\r\n");
        log_info_str("NACK", 4);
    }
    else
    {
        printf("cmd resp ERR = %x\r\n", c);
        log_info_str("ERR", 3);
    }
    return 0;
}

uint32_t stm_rst_state()
{
    ctrl_3v3_pin_set(true);
    target_nrst(false);
    sleep_us(10000);
    target_nrst(true);
    sleep_us(120000); // 100000 -427, no 407, 120000- 407, 446
    // sleep_us(200000);
    return 1;
}

uint32_t stm_rdp2_load_pio_prog()
{
    gnd_3v3_load_pio_prog();
}

uint32_t stm_rdp2_glitch(uint32_t offset, uint32_t width)
{
    gnd_3v3_glitch(offset, width);
}

uint32_t stm_rdp1_load_pio_prog()
{
    gnd_cnt_load_pio_prog();
}

uint32_t stm_rdp1_glitch(uint32_t offset, uint32_t width)
{
    gnd_cnt_glitch(offset, width);
}

uint32_t stm_rdp2_is_unlock()
{
    target_t *tar = app.target;
    swd_init();
    uint32_t ap_id = swd_check();
    printf("ID %x\r\n", ap_id);
    uint32_t new_dword = (ap_id & 0x000000ff) << 24 | (ap_id & 0x0000ff00) << 8 | (ap_id & 0x00ff0000) >> 8 | (ap_id & 0xff000000) >> 24;
    log_info_raw((uint8_t*)(&new_dword), 4);
    if (ap_id == tar->ap_id)
        return 1;
    return 0;
}