#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float temperature_c;
    float humidity_pct;
} sht4x_data_t;

esp_err_t sht4x_init(void);
esp_err_t sht4x_read(sht4x_data_t *out);

#ifdef __cplusplus
}
#endif
