/**
 * @file  sdp3x.c
 * @brief Driver for the Sensirion SDP31 differential pressure sensor.
 *
 * Communication : I2C, address 0x21
 * Mode          : Continuous measurement with mass-flow temperature compensation
 * Output rate   : Up to 2000 SPS; we read at 50 Hz (BEFINE_SAMPLE_RATE_HZ)
 * CRC           : Sensirion CRC-8, polynomial 0x31, init 0xFF
 */

#include <string.h>
#include "sdp3x.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "befine_config.h"

static const char *TAG = "SDP3X";

/* ── Register / command definitions ────────────────────────────────────── */
#define SDP3X_CMD_START_CONT_MASS  0x3615u  /**< Continuous, mass-flow comp  */
#define SDP3X_CMD_SOFT_RESET       0x0006u  /**< General call soft reset     */
#define SDP3X_SCALE_FACTOR_FLOW    60.0f    /**< LSB per Pa — SDP31 datasheet*/
#define SDP3X_SCALE_FACTOR_TEMP    200.0f   /**< LSB per °C                  */
#define SDP3X_READ_LEN             9u       /**< 3 words × (2 data + 1 CRC)  */

/* ── CRC-8 (Sensirion) ──────────────────────────────────────────────────── */
bool sdp3x_check_crc(const uint8_t *data, uint8_t len, uint8_t crc_byte)
{
    uint8_t crc = 0xFF;
    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (uint8_t bit = 0; bit < 8; bit++) {
            crc = (crc & 0x80) ? ((crc << 1) ^ 0x31) : (crc << 1);
        }
    }
    return (crc == crc_byte);
}

/* ── Initialisation ─────────────────────────────────────────────────────── */
esp_err_t sdp3x_init(void)
{
    uint8_t cmd[2] = {
        (uint8_t)(SDP3X_CMD_START_CONT_MASS >> 8),
        (uint8_t)(SDP3X_CMD_START_CONT_MASS & 0xFF),
    };
    esp_err_t ret = i2c_master_write_to_device(
        BEFINE_I2C_PORT, BEFINE_SDP3X_ADDR,
        cmd, sizeof(cmd),
        pdMS_TO_TICKS(100)
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Init failed: %s", esp_err_to_name(ret));
    } else {
        ESP_LOGI(TAG, "Continuous measurement started");
        vTaskDelay(pdMS_TO_TICKS(25)); /* First result ready after ~20 ms */
    }
    return ret;
}

/* ── Read one measurement ───────────────────────────────────────────────── */
esp_err_t sdp3x_read_measurement(sdp3x_measurement_t *out)
{
    uint8_t raw[SDP3X_READ_LEN] = {0};

    esp_err_t ret = i2c_master_read_from_device(
        BEFINE_I2C_PORT, BEFINE_SDP3X_ADDR,
        raw, sizeof(raw),
        pdMS_TO_TICKS(20)
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Read error: %s", esp_err_to_name(ret));
        return ret;
    }

    /* Verify CRC for differential pressure word and temperature word */
    if (!sdp3x_check_crc(&raw[0], 2, raw[2])) {
        ESP_LOGE(TAG, "CRC fail — pressure word");
        return ESP_ERR_INVALID_CRC;
    }
    if (!sdp3x_check_crc(&raw[3], 2, raw[5])) {
        ESP_LOGE(TAG, "CRC fail — temperature word");
        return ESP_ERR_INVALID_CRC;
    }

    int16_t dp_raw   = (int16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    int16_t temp_raw = (int16_t)(((uint16_t)raw[3] << 8) | raw[4]);

    out->differential_pa = (float)dp_raw   / SDP3X_SCALE_FACTOR_FLOW;
    out->temperature_c   = (float)temp_raw / SDP3X_SCALE_FACTOR_TEMP;

    return ESP_OK;
}
