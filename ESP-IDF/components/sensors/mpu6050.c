/**
 * @file  mpu6050.c
 * @brief Driver for InvenSense MPU-6050 6-axis IMU (shake detection only).
 *
 * Communication : I2C, address 0x68
 * Usage         : Reads raw accelerometer X/Y/Z, computes magnitude,
 *                 compares against BEFINE_SHAKE_THRESHOLD_G.
 */

#include "mpu6050.h"
#include "driver/i2c.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "befine_config.h"
#include <math.h>

static const char *TAG = "MPU6050";

/* ── Register map (subset used) ─────────────────────────────────────────── */
#define MPU6050_REG_PWR_MGMT_1   0x6Bu
#define MPU6050_REG_ACCEL_XOUT_H 0x3Bu
#define MPU6050_REG_WHO_AM_I     0x75u
#define MPU6050_WHO_AM_I_VAL     0x68u
#define MPU6050_ACCEL_SCALE_2G   16384.0f   /**< LSB/g for ±2g range */
#define MPU6050_READ_LEN         6u         /**< 3 axes × 2 bytes     */

/* ── Write one register ─────────────────────────────────────────────────── */
static esp_err_t mpu_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = {reg, val};
    return i2c_master_write_to_device(
        BEFINE_I2C_PORT, BEFINE_MPU6050_ADDR,
        buf, sizeof(buf), pdMS_TO_TICKS(20)
    );
}

/* ── Read N bytes starting at reg ──────────────────────────────────────── */
static esp_err_t mpu_read_regs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    return i2c_master_write_read_device(
        BEFINE_I2C_PORT, BEFINE_MPU6050_ADDR,
        &reg, 1, buf, len, pdMS_TO_TICKS(20)
    );
}

/* ── Public API ─────────────────────────────────────────────────────────── */
esp_err_t mpu6050_init(void)
{
    /* Verify device identity */
    uint8_t who_am_i = 0;
    esp_err_t ret = mpu_read_regs(MPU6050_REG_WHO_AM_I, &who_am_i, 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C error: %s", esp_err_to_name(ret));
        return ret;
    }
    if (who_am_i != MPU6050_WHO_AM_I_VAL) {
        ESP_LOGE(TAG, "Wrong WHO_AM_I: 0x%02X (expected 0x68)", who_am_i);
        return ESP_ERR_NOT_FOUND;
    }

    /* Wake from sleep, use internal 8 MHz oscillator */
    ret = mpu_write_reg(MPU6050_REG_PWR_MGMT_1, 0x00);
    if (ret != ESP_OK) return ret;

    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI(TAG, "MPU-6050 initialised");
    return ESP_OK;
}

esp_err_t mpu6050_read_accel(mpu6050_raw_t *out)
{
    uint8_t raw[MPU6050_READ_LEN];
    esp_err_t ret = mpu_read_regs(MPU6050_REG_ACCEL_XOUT_H, raw, sizeof(raw));
    if (ret != ESP_OK) return ret;

    out->accel_x = (int16_t)(((uint16_t)raw[0] << 8) | raw[1]);
    out->accel_y = (int16_t)(((uint16_t)raw[2] << 8) | raw[3]);
    out->accel_z = (int16_t)(((uint16_t)raw[4] << 8) | raw[5]);
    return ESP_OK;
}

float mpu6050_accel_magnitude(const mpu6050_raw_t *raw)
{
    float ax = (float)raw->accel_x / MPU6050_ACCEL_SCALE_2G;
    float ay = (float)raw->accel_y / MPU6050_ACCEL_SCALE_2G;
    float az = (float)raw->accel_z / MPU6050_ACCEL_SCALE_2G;
    return sqrtf(ax * ax + ay * ay + az * az);
}

bool mpu6050_is_shaking(void)
{
    mpu6050_raw_t raw;
    if (mpu6050_read_accel(&raw) != ESP_OK) return false;
    return (mpu6050_accel_magnitude(&raw) > BEFINE_SHAKE_THRESHOLD_G);
}
