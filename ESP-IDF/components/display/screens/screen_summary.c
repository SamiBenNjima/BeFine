#include "display.h"
#include "ssd1306.h"
#include <stdio.h>

static const char *score_label(uint8_t score)
{
    switch (score) {
        case 4: return "A";
        case 3: return "B";
        case 2: return "C";
        default: return "F";
    }
}

void screen_summary_render(float fev1, float pef, uint8_t score)
{
    ssd1306_clear();
    ssd1306_draw_string(20, 2, "Resultats", 1);
    ssd1306_draw_hline(0, 12, 128);

    char buf[24];
    snprintf(buf, sizeof(buf), "FEV1 : %.2f L", fev1);
    ssd1306_draw_string(4, 18, buf, 1);

    snprintf(buf, sizeof(buf), "PEF  : %.0f L/min", pef);
    ssd1306_draw_string(4, 30, buf, 1);

    snprintf(buf, sizeof(buf), "Score: %s", score_label(score));
    ssd1306_draw_string(4, 42, buf, 1);

    /* Score badge (large character at right) */
    ssd1306_draw_string(108, 34, score_label(score), 2);

    ssd1306_flush();
}
