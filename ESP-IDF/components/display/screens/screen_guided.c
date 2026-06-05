#include "display.h"
#include "ssd1306.h"
#include "session.h"

void screen_guided_render(session_state_t step)
{
    ssd1306_clear();
    ssd1306_draw_string(4, 2, "Procedure guidee", 1);
    ssd1306_draw_hline(0, 12, 128);

    switch (step) {
        case SESSION_STATE_SHAKING:
            ssd1306_draw_string(10, 20, "[1/4] Agiter", 1);
            ssd1306_draw_string(4,  36, "Secouez le", 1);
            ssd1306_draw_string(4,  48, "dispositif", 1);
            break;
        case SESSION_STATE_INSERTING:
            ssd1306_draw_string(10, 20, "[2/4] Inserer", 1);
            ssd1306_draw_string(4,  36, "Inserez la", 1);
            ssd1306_draw_string(4,  48, "cartouche", 1);
            break;
        default:
            break;
    }
    ssd1306_flush();
}
