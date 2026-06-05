#include "display.h"
#include "ssd1306.h"
#include <stdio.h>
#include <math.h>

#define FLOW_FULL_SCALE_LPM  600.0f
#define BAR_X   0
#define BAR_Y   18
#define BAR_W   110
#define BAR_H   20

void screen_flow_render(float flow_lpm, bool is_inhaling)
{
    ssd1306_clear();

    /* Title */
    ssd1306_draw_string(0, 2, is_inhaling ? "[3/4] INHALATION" : "[4/4] APNEE", 1);
    ssd1306_draw_hline(0, 13, 128);

    /* Bargraph border */
    ssd1306_draw_rect(BAR_X, BAR_Y, BAR_W + 2, BAR_H + 2);

    /* Bargraph fill */
    float ratio = flow_lpm / FLOW_FULL_SCALE_LPM;
    if (ratio > 1.0f) ratio = 1.0f;
    if (ratio < 0.0f) ratio = 0.0f;
    uint8_t fill_w = (uint8_t)(ratio * (float)BAR_W);
    if (fill_w > 0) {
        ssd1306_fill_rect(BAR_X + 1, BAR_Y + 1, fill_w, BAR_H);
    }

    /* Numeric value */
    char buf[20];
    snprintf(buf, sizeof(buf), "%.1f L/min", flow_lpm);
    ssd1306_draw_string(0, 46, buf, 1);

    ssd1306_flush();
}
