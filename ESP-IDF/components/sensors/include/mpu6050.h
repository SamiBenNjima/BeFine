#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int16_t accel_x;
    int16_t accel_y;
    int16_t accel_z;
    int16_t gyro_x;
    int16_t gyro_y;
    int16_t gyro_z;
} mpu6050_raw_t;

esp_err_t mpu6050_init(void);
esp_err_t mpu6050_read_accel(mpu6050_raw_t *out);
float     mpu6050_accel_magnitude(const mpu6050_raw_t *raw);
bool      mpu6050_is_shaking(void);

#ifdef __cplusplus
}
#endif
