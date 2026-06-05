#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float differential_pa;   /**< Differential pressure in Pascals */
    float temperature_c;     /**< On-chip temperature in °C        */
} sdp3x_measurement_t;

esp_err_t sdp3x_init(void);
esp_err_t sdp3x_read_measurement(sdp3x_measurement_t *out);
bool      sdp3x_check_crc(const uint8_t *data, uint8_t len, uint8_t crc_byte);

#ifdef __cplusplus
}
#endif
