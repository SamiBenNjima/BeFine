#pragma once
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    POWER_MODE_ACTIVE = 0,   /**< 240 MHz, display full brightness          */
    POWER_MODE_IDLE   = 1,   /**< 80 MHz, light sleep between tasks, dim    */
    POWER_MODE_SLEEP  = 2,   /**< 40 MHz, BLE adv interval extended         */
} power_mode_t;

esp_err_t power_mgmt_init(void);
void      power_mgmt_task(void *pvParameters);
void      power_mgmt_set_mode(power_mode_t mode);
void      power_mgmt_touch(void);   /**< Reset idle timer */

#ifdef __cplusplus
}
#endif
