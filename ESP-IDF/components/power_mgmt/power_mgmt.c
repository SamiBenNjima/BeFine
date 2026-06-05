/**
 * @file  power_mgmt.c
 * @brief Power management — three profiles + auto deep-sleep after idle timeout.
 *
 * Transitions:
 *   ACTIVE (session in progress or just connected)
 *   → IDLE  (no session, BLE connected, no activity for 30s)
 *   → SLEEP (no session, BLE disconnected or 2 min idle)
 *   → DEEP SLEEP (no activity for BEFINE_IDLE_SLEEP_MIN minutes)
 *
 * Wake-up sources from deep sleep:
 *   - GPIO edge (QRE spray insert pin)
 *   - RTC timer (30s — for BLE beacon pulse)
 */

#include "power_mgmt.h"
#include "session.h"
#include "display.h"
#include "befine_config.h"
#include "esp_log.h"
#include "esp_pm.h"
#include "esp_sleep.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "POWER_MGT";

/* ── Power profiles ─────────────────────────────────────────────────────── */
static const esp_pm_config_t PM_ACTIVE = {
    .max_freq_mhz       = 240,
    .min_freq_mhz       = 80,
    .light_sleep_enable = false,
};

static const esp_pm_config_t PM_IDLE = {
    .max_freq_mhz       = 80,
    .min_freq_mhz       = 10,
    .light_sleep_enable = true,
};

static const esp_pm_config_t PM_SLEEP_PREP = {
    .max_freq_mhz       = 40,
    .min_freq_mhz       = 10,
    .light_sleep_enable = true,
};

/* Idle timer — ticks at last user/session activity */
static volatile TickType_t s_last_activity_tick = 0;
static volatile power_mode_t s_mode = POWER_MODE_ACTIVE;

/* ── Public API ─────────────────────────────────────────────────────────── */

esp_err_t power_mgmt_init(void)
{
    s_last_activity_tick = xTaskGetTickCount();
    return esp_pm_configure(&PM_ACTIVE);
}

void power_mgmt_touch(void)
{
    s_last_activity_tick = xTaskGetTickCount();
    if (s_mode != POWER_MODE_ACTIVE) {
        power_mgmt_set_mode(POWER_MODE_ACTIVE);
    }
}

void power_mgmt_set_mode(power_mode_t mode)
{
    s_mode = mode;
    switch (mode) {
        case POWER_MODE_ACTIVE:
            esp_pm_configure(&PM_ACTIVE);
            display_set_brightness(255);
            ESP_LOGI(TAG, "Mode: ACTIVE (240 MHz)");
            break;

        case POWER_MODE_IDLE:
            esp_pm_configure(&PM_IDLE);
            display_set_brightness(64);
            ESP_LOGI(TAG, "Mode: IDLE (80 MHz, light sleep)");
            break;

        case POWER_MODE_SLEEP:
            esp_pm_configure(&PM_SLEEP_PREP);
            display_set_brightness(16);
            ble_set_adv_interval(1285, 1285);   /* 1.285 s — minimal BLE power */
            ESP_LOGI(TAG, "Mode: SLEEP (40 MHz, extended adv)");
            break;
    }
}

static void check_idle_timeout(void)
{
    TickType_t elapsed = xTaskGetTickCount() - s_last_activity_tick;
    uint32_t elapsed_ms = (uint32_t)(elapsed * portTICK_PERIOD_MS);
    uint32_t sleep_ms = (uint32_t)(BEFINE_IDLE_SLEEP_MIN * 60 * 1000);

    /* Progressive power-down */
    if (elapsed_ms > 30000 && s_mode == POWER_MODE_ACTIVE) {
        if (session_fsm_get_state() == SESSION_STATE_IDLE) {
            power_mgmt_set_mode(POWER_MODE_IDLE);
        }
    }
    if (elapsed_ms > 120000 && s_mode == POWER_MODE_IDLE) {
        power_mgmt_set_mode(POWER_MODE_SLEEP);
    }

    /* Deep sleep after BEFINE_IDLE_SLEEP_MIN minutes */
    if (elapsed_ms > sleep_ms) {
        ESP_LOGI(TAG, "Entering deep sleep after %lu ms idle", (unsigned long)elapsed_ms);
        esp_sleep_enable_gpio_wakeup();
        esp_sleep_enable_timer_wakeup(
            (uint64_t)BEFINE_SLEEP_WAKEUP_SEC * 1000000ULL);
        esp_deep_sleep_start();
        /* Never returns */
    }
}

/* ── 1 Hz monitoring task ────────────────────────────────────────────────── */
void power_mgmt_task(void *pvParameters)
{
    (void)pvParameters;
    ESP_LOGI(TAG, "Power management task started");

    for (;;) {
        /* Touch the activity timer whenever a session is active */
        if (session_fsm_get_state() != SESSION_STATE_IDLE &&
            session_fsm_get_state() != SESSION_STATE_COMPLETE) {
            power_mgmt_touch();
        }
        check_idle_timeout();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
