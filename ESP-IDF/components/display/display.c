/**
 * @file  display.c
 * @brief High-level display task and state dispatcher.
 *
 * Runs on Core 0 at 10 Hz.
 * Reads the current session state and delegates rendering to a screen module.
 */

#include "display.h"
#include "ssd1306.h"
#include "session.h"
#include "befine_config.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "DISPLAY";

/* Current display state — written by FSM, read by display_task */
static volatile session_state_t s_current_state = SESSION_STATE_IDLE;
static volatile float           s_flow_lpm      = 0.0f;
static volatile bool            s_is_inhaling   = false;
static const session_context_t *s_ctx           = NULL;

esp_err_t display_init(void)
{
    return ssd1306_init();
}

void display_show_splash(void)
{
    ssd1306_clear();
    ssd1306_draw_string(20, 10, "BeFine", 2);
    ssd1306_draw_string(10, 30, "Smart Inhaler", 1);
    ssd1306_draw_string(8,  48, "v1.0  Initialising", 1);
    ssd1306_flush();
}

void display_set_brightness(uint8_t level)
{
    ssd1306_set_contrast(level);
}

/* Called by session FSM on every state transition */
void display_update_state(session_state_t s, const session_context_t *ctx)
{
    s_current_state = s;
    s_ctx = ctx;
}

/* Called by BLE notify task with latest flow data */
void display_update_flow(float flow_lpm, bool is_inhaling)
{
    s_flow_lpm    = flow_lpm;
    s_is_inhaling = is_inhaling;
}

/* 10 Hz display refresh task */
void display_task(void *pvParameters)
{
    (void)pvParameters;
    TickType_t last_wake = xTaskGetTickCount();

    ESP_LOGI(TAG, "Display task started (10 Hz)");

    for (;;) {
        switch (s_current_state) {
            case SESSION_STATE_IDLE:
                screen_idle_render();
                break;
            case SESSION_STATE_SHAKING:
            case SESSION_STATE_INSERTING:
                screen_guided_render(s_current_state);
                break;
            case SESSION_STATE_INHALING:
            case SESSION_STATE_HOLDING:
                screen_flow_render((float)s_flow_lpm, (bool)s_is_inhaling);
                break;
            case SESSION_STATE_COMPLETE:
                if (s_ctx) {
                    screen_summary_render(s_ctx->last_result.fev1_l,
                                          s_ctx->last_result.pef_lpm,
                                          s_ctx->last_result.technique_score);
                }
                break;
            case SESSION_STATE_ERROR:
                ssd1306_clear();
                ssd1306_draw_string(20, 20, "ERREUR", 1);
                ssd1306_draw_string(8,  40, "Reinitialiser", 1);
                ssd1306_flush();
                break;
            default:
                break;
        }
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(100));  /* 10 Hz */
    }
}
