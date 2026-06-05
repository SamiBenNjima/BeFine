/**
 * @file  sensors.c
 * @brief Aggregated sensor task — reads SDP3x, SHT4x, MPU6050 at 50 Hz.
 *
 * Runs on Core 1 (APP_CPU) at priority BEFINE_TASK_PRIO_SENSOR.
 * Pushes sensor_data_t frames onto g_sensor_queue for the DSP task.
 * SHT4x is sampled at 1 Hz (every 50 sensor cycles) — its conversion
 * time (9 ms) would break the 20 ms budget if sampled at 50 Hz.
 */

#include "sensors.h"
#include "sdp3x.h"
#include "sht4x.h"
#include "mpu6050.h"
#include "befine_config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

extern QueueHandle_t g_sensor_queue;

static const char *TAG = "SENSORS";

/* Shared last SHT4x reading — updated at 1 Hz */
static sht4x_data_t s_env = {.temperature_c = 25.0f, .humidity_pct = 50.0f};

/* ── Initialise all sensors ─────────────────────────────────────────────── */
esp_err_t sensors_init(void)
{
    esp_err_t ret;

    ret = sdp3x_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SDP3x init failed");
        return ret;
    }
    ret = sht4x_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SHT4x init failed");
        return ret;
    }
    ret = mpu6050_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "MPU6050 init failed");
        return ret;
    }

    ESP_LOGI(TAG, "All sensors initialised");
    return ESP_OK;
}

/* ── 50 Hz acquisition task ─────────────────────────────────────────────── */
void sensor_task(void *pvParameters)
{
    (void)pvParameters;
    TickType_t  last_wake     = xTaskGetTickCount();
    uint32_t    cycle_count   = 0;

    ESP_LOGI(TAG, "Sensor task started (%d Hz)", BEFINE_SAMPLE_RATE_HZ);

    for (;;) {
        sensor_data_t frame = {0};

        /* ── SDP3x — every cycle (50 Hz) ──────────────────────────────── */
        sdp3x_measurement_t sdp;
        if (sdp3x_read_measurement(&sdp) == ESP_OK) {
            frame.differential_pa = sdp.differential_pa;
            frame.sensor_temp_c   = sdp.temperature_c;
        } else {
            ESP_LOGW(TAG, "SDP3x read error — using last value");
        }

        /* ── SHT4x — every 50 cycles (1 Hz) ───────────────────────────── */
        if (cycle_count % BEFINE_SAMPLE_RATE_HZ == 0) {
            sht4x_data_t env;
            if (sht4x_read(&env) == ESP_OK) {
                s_env = env;
            }
        }
        frame.ambient_temp_c = s_env.temperature_c;
        frame.humidity_pct   = s_env.humidity_pct;

        /* ── MPU6050 — every cycle (50 Hz) ─────────────────────────────── */
        mpu6050_raw_t imu;
        if (mpu6050_read_accel(&imu) == ESP_OK) {
            frame.accel_mag_g = mpu6050_accel_magnitude(&imu);
            frame.is_shaking  = (frame.accel_mag_g > BEFINE_SHAKE_THRESHOLD_G);
        }

        frame.timestamp_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);

        /* ── Push to DSP queue (drop if full — non-blocking) ─────────── */
        if (xQueueSend(g_sensor_queue, &frame, 0) != pdTRUE) {
            ESP_LOGD(TAG, "sensor_queue full — frame dropped");
        }

        cycle_count++;
        vTaskDelayUntil(&last_wake, pdMS_TO_TICKS(BEFINE_SAMPLE_PERIOD_MS));
    }
}
