#include "includes/swd_pico.h"
#include "p_swd.pio.h"

uint32_t offset_swd_probe;

uint32_t init_swd_pico()
{
    pio_clear_instruction_memory(DEFAULT_SWD_PIO);
    offset_swd_probe = pio_add_program(DEFAULT_SWD_PIO, &p_swd_program);
    p_swd_program_init(DEFAULT_SWD_PIO, DEFAULT_SWD_SM, offset_swd_probe, PROBE_PIN_SWCLK, PROBE_PIN_SWDIO);

    gpio_pull_up(PROBE_PIN_SWDIO);

    pio_gpio_init(DEFAULT_SWD_PIO, PROBE_PIN_SWCLK);
    pio_gpio_init(DEFAULT_SWD_PIO, PROBE_PIN_SWDIO);

    pio_sm_set_enabled(DEFAULT_SWD_PIO, DEFAULT_SWD_SM, true);
}

static inline int parity_u32(uint32_t x)
{
#ifdef __GNUC__
    return __builtin_parityl(x);
#else
    x ^= x >> 16;
    x ^= x >> 8;
    x ^= x >> 4;
    x ^= x >> 2;
    x ^= x >> 1;
    return x & 1;
#endif
}

static inline uint8_t swd_cmd(bool is_read, bool is_ap, uint8_t regnum)
{
    uint8_t cmd = (is_ap ? SWD_CMD_APNDP : 0)
        | (is_read ? SWD_CMD_RNW : 0)
        | ((regnum & 0xc) << 1);

    /* 8 cmd bits 4:1 may be set */
    if (parity_u32(cmd))
        cmd |= SWD_CMD_PARITY;

    /* driver handles START, STOP, and TRN */

    return cmd;
}

uint8_t swd_seq_jtag_to_swd[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x9e, 0xe7,
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
    0x00
};
uint32_t swd_seq_jtag_to_swd_len = 136;

void probe_read_mode(void) {
    pio_sm_exec(DEFAULT_SWD_PIO, DEFAULT_SWD_SM, pio_encode_jmp(offset_swd_probe + p_swd_offset_in_posedge));
    while(DEFAULT_SWD_PIO->dbg_padoe & (1 << PROBE_PIN_SWDIO));
}

void probe_write_mode(void) {
    pio_sm_exec(DEFAULT_SWD_PIO, DEFAULT_SWD_SM, pio_encode_jmp(offset_swd_probe + p_swd_offset_out_negedge));
    while(!(DEFAULT_SWD_PIO->dbg_padoe & (1 << PROBE_PIN_SWDIO)));
}

void probe_write_bits(uint bit_count, uint32_t data_byte) {
    pio_sm_put_blocking(DEFAULT_SWD_PIO, DEFAULT_SWD_SM, bit_count - 1);
    pio_sm_put_blocking(DEFAULT_SWD_PIO, DEFAULT_SWD_SM, data_byte);
    pio_sm_get_blocking(DEFAULT_SWD_PIO, DEFAULT_SWD_SM);
}

void probe_write_bits_u8(uint bit_count, uint8_t * src) {
    while (bit_count > 0) {
        uint cur_bit_count = 32;
        if (bit_count < cur_bit_count)
            cur_bit_count = bit_count;
        probe_write_bits(cur_bit_count, *(uint32_t *)src);
        src += 4;
        bit_count -= cur_bit_count;
    }
}

uint32_t probe_read_bits(uint bit_count) {
    pio_sm_put_blocking(DEFAULT_SWD_PIO, DEFAULT_SWD_SM, bit_count - 1);
    uint32_t data = pio_sm_get_blocking(DEFAULT_SWD_PIO, DEFAULT_SWD_SM);
    uint32_t data_shifted = data;
    if (bit_count < 32) {
        data_shifted = data >> (32 - bit_count);
    }
    return data_shifted;
}

uint32_t _swd_do_cmd(uint32_t cmd, uint32_t value, uint8_t * ack) {
    uint32_t r;
    cmd |= SWD_CMD_START | SWD_CMD_PARK;
    probe_write_mode();
    probe_write_bits(8, cmd);
    if (cmd & SWD_CMD_RNW) {
        probe_read_mode();
        *ack = probe_read_bits(4);
        r = probe_read_bits(32);
        probe_read_bits(2);
    } else {
        uint32_t parity = parity_u32(value);
        probe_read_mode();
        probe_read_bits(5);
        probe_write_mode();
        probe_write_bits(32, value);
        probe_write_bits(1, parity);
    }
    return r;
}

uint32_t swd_do_cmd(uint32_t cmd, uint32_t value) {
    uint8_t ack;
    return _swd_do_cmd(cmd, value, &ack);
}

uint32_t swd_check() {
    probe_write_mode();
    probe_write_bits_u8(swd_seq_jtag_to_swd_len, swd_seq_jtag_to_swd);
    return swd_do_cmd(swd_cmd(true, false, 0x0), 0x0);
}

void swd_init() {
    probe_write_mode();
    probe_write_bits_u8(swd_seq_jtag_to_swd_len, swd_seq_jtag_to_swd);
    // read IDCODE
    swd_do_cmd(swd_cmd(true, false, 0x0), 0x0);
    // clear sticky bits
    swd_do_cmd(swd_cmd(false, false, 0x0), 0xf << 1);
    // read CTRL/STAT
    swd_do_cmd(swd_cmd(true, false, 0x4), 0x0);
    // enable debug power domain
    swd_do_cmd(swd_cmd(false, false, 0x4), 0x50000000);
    // read CTRL/STAT
    swd_do_cmd(swd_cmd(true, false, 0x4), 0x0);
    // write DP_SELECT with 0
    swd_do_cmd(swd_cmd(false, false, 0x8), 0x0);
    // write MEMAP CSW 0x23000002
    swd_do_cmd(swd_cmd(false, true, 0x0), 0x23000002);
}

uint32_t ahb_ap_check() {
    probe_write_mode();
    probe_write_bits_u8(swd_seq_jtag_to_swd_len, swd_seq_jtag_to_swd);
    // read IDCODE
    swd_do_cmd(swd_cmd(true, false, 0x0), 0x0);
    // clear sticky bits
    swd_do_cmd(swd_cmd(false, false, 0x0), 0xf << 1);
    // read CTRL/STAT
    swd_do_cmd(swd_cmd(true, false, 0x4), 0x0);
    // enable debug power domain
    swd_do_cmd(swd_cmd(false, false, 0x4), 0x50000000);
    // read CTRL/STAT
    swd_do_cmd(swd_cmd(true, false, 0x4), 0x0);
    // write DP_SELECT with 0
    swd_do_cmd(swd_cmd(false, false, 0x8), (0 << 24) | (0xf << 4));
    // read AHB-AP id
    swd_do_cmd(swd_cmd(true, true, 0xc), 0x0);
    return swd_do_cmd(swd_cmd(true, false, 0xc), 0x0);
}

uint32_t nrf52_erase()
{
    probe_write_mode();
    probe_write_bits_u8(swd_seq_jtag_to_swd_len, swd_seq_jtag_to_swd);
    // read IDCODE
    swd_do_cmd(swd_cmd(true, false, 0x0), 0x0);
    // clear sticky bits
    swd_do_cmd(swd_cmd(false, false, 0x0), 0xf << 1);
    // read CTRL/STAT
    swd_do_cmd(swd_cmd(true, false, 0x4), 0x0);
    // enable debug power domain
    swd_do_cmd(swd_cmd(false, false, 0x4), 0x50000000);
    // read CTRL/STAT
    swd_do_cmd(swd_cmd(true, false, 0x4), 0x0);
    // write DP_SELECT with 0
    swd_do_cmd(swd_cmd(false, false, 0x8), (0 << 24) | (0xf << 4));
    // read AHB-AP id
    swd_do_cmd(swd_cmd(true, true, 0xc), 0x0);
    swd_do_cmd(swd_cmd(true, false, 0xc), 0x0);

    swd_do_cmd(swd_cmd(false, false, 0x8), (1 << 24) | (0x0 << 4));
    swd_do_cmd(swd_cmd(false, true, 0x4), 0x1);    
}

uint32_t nrf52_aprotect_en()
{

}

void probe_assert_reset(bool state)
{
    gpio_put(GP3_TARGET_RESET, state);
}

uint32_t swd_memread(uint32_t addr, uint8_t * ack, int delay) {
    probe_assert_reset(true);
    sleep_us(delay);
    // write MEMAP TAR
    swd_do_cmd(swd_cmd(false, true, 0x4), addr);
    uint32_t v = 0;
    probe_write_mode();
    probe_write_bits(10, v);
    // read MEMAP DRW
    swd_do_cmd(swd_cmd(true, true, 0xc), 0x0);
    sleep_us(40);
    probe_assert_reset(false);
    // read RDUBF DRW
    uint32_t value = _swd_do_cmd(swd_cmd(true, false, 0xc), 0x0, ack);
    return value;
}

uint32_t swd_memread_noreset(uint32_t addr) {
    // write MEMAP TAR
    swd_do_cmd(swd_cmd(false, true, 0x4), addr);
    probe_write_mode();
    probe_write_bits(10, 0);
    // read MEMAP DRW
    swd_do_cmd(swd_cmd(true, true, 0xc), 0x0);
    probe_write_mode();
    probe_write_bits(10, 0);
    // read RDBUFF DRW
    uint32_t value = swd_do_cmd(swd_cmd(true, false, 0xc), 0x0);
    probe_write_mode();
    probe_write_bits(10, 0);
    return value;
}


// void swd_memwrite(uint32_t addr, uint32_t value, int delay) {
//     probe_assert_reset(true);
//     sleep_us(delay);

//     

//     swd_do_cmd(swd_cmd(false, true, 0x4), addr);

//     uint32_t v = 0;
//     probe_write_mode();
//     probe_write_bits(10, v);

//     // write MEMAP DRW
//     swd_do_cmd(swd_cmd(false, true, 0xc), value);

//     probe_write_mode();
//     probe_write_bits(10, v);

//     // read MEMAP DRW
//     swd_do_cmd(swd_cmd(true, true, 0xc), 0x0);
//     //probe_write_bits(8, swd_cmd(false, true, 0xc) | SWD_CMD_START | SWD_CMD_PARK);

//     sleep_us(40);
//     probe_assert_reset(false);
// }

void swd_memwrite_noreset(uint32_t addr, uint32_t value) {
    // write DP_SELECT with 0
    swd_do_cmd(swd_cmd(false, false, 0x8), (0 << 24) | (0xf << 4));
    // write MEMAP TAR
    swd_do_cmd(swd_cmd(false, true, 0x4), addr);
    probe_write_mode();
    probe_write_bits(10, 0);
    // write MEMAP DRW
    swd_do_cmd(swd_cmd(false, true, 0xc), value);
    probe_write_mode();
    probe_write_bits(10, 0);
}

bool stm32_read_bsy()
{
    swd_do_cmd(swd_cmd(false,true,0x4),0x40023c0c);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(true,true,0xc),0x0);
    probe_write_mode();
    probe_write_bits(10, 0);
    uint32_t flash_status = swd_do_cmd(swd_cmd(true,true,0xc),0x0);
    probe_write_mode();
    probe_write_bits(10, 0);

    while(flash_status & (1<<16)){
        sleep_ms(100);
        swd_do_cmd(swd_cmd(false,true,0x4),0x40023c0c);
        probe_write_mode();
        probe_write_bits(10, 0);
        swd_do_cmd(swd_cmd(true,true,0xc),0x0);
        probe_write_mode();
        probe_write_bits(10, 0);
        flash_status = swd_do_cmd(swd_cmd(true,true,0xc),0x0);
        probe_write_mode();
        probe_write_bits(10, 0);
    }

    return true;
}

void stm32_set_rdp(uint8_t level)
{
    probe_write_mode();
    probe_write_bits_u8(swd_seq_jtag_to_swd_len, swd_seq_jtag_to_swd);
    // read IDCODE
    swd_do_cmd(swd_cmd(true, false, 0x0), 0x0);
    // DAP target prepare
    swd_do_cmd(swd_cmd(false,false,0x8),0x0);
    // Enable debug power domain
    swd_do_cmd(swd_cmd(false,false,0x4),0x50000000);
    // write MEMAP CSW
    swd_do_cmd(swd_cmd(false,true,0x0),0x23000052);
    // Writing magical keys
    swd_do_cmd(swd_cmd(false,true,0x4),0x40023c08);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(false,true,0xc),0x08192A3B);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(false,true,0x4),0x40023c08);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(false,true,0xc),0x4C5D6E7F);
    probe_write_mode();
    probe_write_bits(10, 0);
    // Waiting for flash controller to be ready to write in flash
    stm32_read_bsy();
    // Writing RDP value
    swd_do_cmd(swd_cmd(false,true,0x4),0x40023c14);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(false,true,0xc),0xfff00ee | level << 8);
    probe_write_mode();
    probe_write_bits(10, 0);
    // Waiting for flash controller to write in flash
    stm32_read_bsy();
    printf("Done\r\n");
}

uint8_t stm32_get_rdp()
{
    probe_write_mode();
    probe_write_bits_u8(swd_seq_jtag_to_swd_len, swd_seq_jtag_to_swd);
    // read IDCODE
    swd_do_cmd(swd_cmd(true, false, 0x0), 0x0);
    // DAP target prepare
    swd_do_cmd(swd_cmd(false,false,0x8),0x0);
    // Enable debug power domain
    swd_do_cmd(swd_cmd(false,false,0x4),0x50000000);
    // write MEMAP CSW
    swd_do_cmd(swd_cmd(false,true,0x0),0x23000052);
    // Writing magical keys
    swd_do_cmd(swd_cmd(false,true,0x4),0x40023c08);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(false,true,0xc),0x08192A3B);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(false,true,0x4),0x40023c08);
    probe_write_mode();
    probe_write_bits(10, 0);
    swd_do_cmd(swd_cmd(false,true,0xc),0x4C5D6E7F);
    probe_write_mode();
    probe_write_bits(10, 0);
    // Waiting for flash controller to be ready to write in flash
    stm32_read_bsy();
    // Reading rdp level from memory
    uint32_t ob_data = swd_memread_noreset(0x40023c14);
    printf("OB data = %x\r\n",ob_data);
    uint8_t level = (ob_data & 0xff00) >> 8;
    printf("Level = 0x%X\r\n",level);
    return level;
}