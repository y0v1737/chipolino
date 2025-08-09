cmake_minimum_required(VERSION 3.13)
include(pico_sdk_import.cmake)
project(chipolino_project C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(PICO_COPY_TO_RAM 1)
pico_sdk_init()

add_executable(chipolino
  chipolino.c
  targets/stm_lib.c
  targets/stm32f4xx.c
  targets/lpc.c
  targets/nrf52.c
  targets/rh850.c
  glitcher.c
  swd_pico.c
  common.c
  ws2812.c
  jtag.c
)

pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/p_3v3_rst.pio)
pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/p_gnd_gpio.pio)
pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/p_swd.pio)
pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/p_gnd_cnt.pio)
pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/p_gnd_cnt_2g.pio)
pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/p_gnd_3v3.pio)
pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/p_3v3_cnt.pio)
pico_generate_pio_header(chipolino ${CMAKE_CURRENT_LIST_DIR}/pio_sm/pwm.pio)

pico_enable_stdio_usb(chipolino 1)
pico_enable_stdio_uart(chipolino 0)
pico_add_extra_outputs(chipolino)
target_link_libraries(chipolino pico_stdlib pico_bootrom hardware_pio hardware_clocks hardware_spi pico_multicore)
