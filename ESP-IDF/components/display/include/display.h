#pragma once
#include "esp_err.h"
#include "session.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t display_init(void);
void      display_show_splash(void);
void      display_task(void *pvParameters);
void      display_update_state(session_state_t s, const session_context_t *ctx);
void      display_set_brightness(uint8_t level);   /* 0=off 255=max */

/* Screen renderers — called from display_task */
void screen_idle_render   (void);
void screen_guided_render (session_state_t step);
void screen_flow_render   (float flow_lpm, bool is_inhaling);
void screen_summary_render(float fev1, float pef, uint8_t score);

#ifdef __cplusplus
}
#endif
