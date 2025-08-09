#ifndef _WS2812_H
#define _WS2812_H

#include "pico/stdlib.h"

typedef struct {
    uint8_t pin;
} ws2812_config_t;

extern ws2812_config_t ws2812_config;

void ws2812_init(ws2812_config_t *ws2812_config);
void ws2812_set_pixel(ws2812_config_t *ws2812_config, uint32_t set_color);

#endif