 #include "includes/ws2812.h"
 #include "includes/common.h"

 ws2812_config_t ws2812_config = {
     .pin = GP27_LED_RGD 
 };
 
 void ws2812_init(ws2812_config_t *ws2812_config) {
     gpio_init(ws2812_config->pin);
     gpio_set_dir(ws2812_config->pin, 1);
 }
 
 void ws2812_set_pixel(ws2812_config_t *ws2812_config, uint32_t set_color) {
     uint8_t index = 0;
     uint8_t bit = 0;
     uint8_t bits = 24;
 
     while (bits--) {
         if (set_color & (1 << bit++)) {
             // one
             for (index = 0; index <= 20; index++)
                 gpio_put(ws2812_config->pin, 1);
             for (index = 0; index <= 4; index++)
                 gpio_put(ws2812_config->pin, 0);
         } else {
             // zero
             for (index = 0; index <= 6; index++)
                 gpio_put(ws2812_config->pin, 1);
             for (index = 0; index <= 20; index++)
                 gpio_put(ws2812_config->pin, 0);
         }
     }
 }