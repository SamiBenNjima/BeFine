/**
 * @file  sht4x.c
 * @brief Driver for the Sensirion SHT41 temperature & humidity sensor.
 *
 * Communication : I2C, address 0x44
 * Measurement   : High-repeatability single-shot (command 0xFD)
 * Update rate   : 1 Hz (polled from sensor_task every 50 cycles)
 */

#include "sht4x.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "befine_config.h"
#include <math.h>

static const char *TAG = "SHT4X";

#define SHT4X_CMD_MEASURE_HIGH  0xFDu   /**< High-repeatability measurement */
#define SHT4X_MEAS_DELAY_MS     10u     /**< Measurement time ≈ 8.3 ms      */
#define SHT4X_READ_LEN          6u      /**< T_MSB T_LSB T_CRC H_MSB H_LSB H_CRC */

esp_err_t sht4x_init(void)
{
    /* SHT4x requires no explicit init — just verify it ACKs */
    uint8_t cmd = SHT4X_CMD_MEASURE_HIGH;
    esp_err_t ret = i2c_master_write_to_device(
        BEFINE_I2C_PORT, BEFINE_SHT4X_ADDR,
        &cmd, 1, pdMS_TO_TICKS(50)
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Device not found at 0x%02X: %s",
                 BEFINE_SHT4X_ADDR, esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "SHT41 detected");
    }
    vTaskDelay(pdMS_TO_TICKS(SHT4X_MEAS_DELAY_MS));
    return ret;
}

esp_err_t sht4x_read(sht4x_data_t *out)
{
    /* Send measure command */
    uint8_t cmd = SHT4X_CMD_MEASURE_HIGH;
    esp_err_t ret = i2c_master_write_to_device(
        BEFINE_I2C_PORT, BEFINE_SHT4X_ADDR,
        &cmd, 1, pdMS_TO_TICKS(10)
    );
    if (ret != ESP_OK) return ret;

    /* Wait for conversion */
    vTaskDelay(pdMS_TO_TICKS(SHT4X_MEAS_DELAY_MS));

    /* Read result */
    uint8_t raw[SHT4X_READ_LEN];
    ret = i2c_master_read_from_device(
        BEFINE_I2C_PORT, BEFINE_SHT4X_ADDR,
        raw, sizeof(raw), pdMS_TO_TICKS(10)
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read error: %s", esp_err_to_name(ret));
        return ret;
    }

    uint16_t t_raw  = ((uint16_t)raw[0] << 8) | raw[1];
    uint16_t rh_raw = ((uint16_t)raw[3] << 8) | raw[4];

    out->temperature_c = -45.0f + 175.0f * ((float)t_raw  / 65535.0f);
    out->humidity_pct  =  -6.0f + 125.0f * ((float)rh_raw / 65535.0f);

    /* Clamp humidity to [0, 100] %rH */
    if (out->humidity_pct < 0.0f)   out->humidity_pct = 0.0f;
    if (out->humidity_pct > 100.0f) out->humidity_pct = 100.0f;

    return ESP_OK;
}
