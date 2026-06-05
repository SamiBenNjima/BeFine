#pragma once
/**
 * @file  sensors.h
 * @brief Aggregated sensor interface for BeFine firmware.
 *
 * Exposes:
 *  - sensor_data_t  : one frame of raw sensor readings (50 Hz)
 *  - sensors_init() : initialise all three sensors on the shared I2C bus
 *  - sensor_task()  : FreeRTOS task — reads all sensors at BEFINE_SAMPLE_RATE_HZ
 *                     and pushes sensor_data_t onto g_sensor_queue
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/* ── Raw aggregated sample (one frame at 50 Hz) ─────────────────────────── */
typedef struct {
    float    differential_pa;   /**< SDP31 — differential pressure (Pa)      */
    float    sensor_temp_c;     /**< SDP31 — on-chip temperature (°C)        */
    float    ambient_temp_c;    /**< SHT41 — ambient temperature (°C)        */
    float    humidity_pct;      /**< SHT41 — relative humidity (%rH)         */
    float    accel_mag_g;       /**< MPU6050 — acceleration magnitude (g)    */
    bool     is_shaking;        /**< MPU6050 — shake threshold exceeded       */
    uint32_t timestamp_ms;      /**< esp_log_timestamp() at time of read      */
} sensor_data_t;

/* ── Public API ─────────────────────────────────────────────────────────── */

/**
 * @brief  Initialise all sensors (SDP3x, SHT4x, MPU6050) on the I2C bus.
 *         Must be called after i2c_driver_install().
 * @return ESP_OK on success, ESP_ERR_* on sensor probe failure.
 */
esp_err_t sensors_init(void);

/**
 * @brief  FreeRTOS task — 50 Hz sensor acquisition loop.
 *         Reads all sensors and pushes sensor_data_t to g_sensor_queue.
 *         Pinned to APP_CPU (Core 1), priority BEFINE_TASK_PRIO_SENSOR.
 */
void sensor_task(void *pvParameters);

/* ── Individual driver headers (included by sensors.c, exposed for tests) ─ */
#include "sdp3x.h"
#include "sht4x.h"
#include "mpu6050.h"

#ifdef __cplusplus
}
#endif
