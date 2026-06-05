#pragma once
#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 128×64 pixel framebuffer */
#define SSD1306_WIDTH   128
#define SSD1306_HEIGHT   64

esp_err_t ssd1306_init(void);
void      ssd1306_clear(void);
void      ssd1306_flush(void);
void      ssd1306_draw_pixel(uint8_t x, uint8_t y, uint8_t on);
void      ssd1306_draw_hline(uint8_t x, uint8_t y, uint8_t w);
void      ssd1306_draw_vline(uint8_t x, uint8_t y, uint8_t h);
void      ssd1306_fill_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void      ssd1306_draw_rect(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
void      ssd1306_draw_string(uint8_t x, uint8_t y, const char *str, uint8_t font_size);
void      ssd1306_set_contrast(uint8_t contrast);

#ifdef __cplusplus
}
#endif
