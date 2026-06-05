#include "display.h"
#include "ssd1306.h"

void screen_idle_render(void)
{
    ssd1306_clear();
    ssd1306_draw_string(30, 4,  "BeFine", 1);
    ssd1306_draw_hline(0, 14, 128);
    ssd1306_draw_string(4,  22, "Pret  Appuyez", 1);
    ssd1306_draw_string(4,  36, "sur Demarrer", 1);
    ssd1306_draw_string(4,  52, "Batterie: OK", 1);
    ssd1306_flush();
}
