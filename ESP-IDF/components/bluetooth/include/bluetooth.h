#pragma once
/**
 * @file  bluetooth.h
 * @brief BLE GATT server public interface.
 */

#include "esp_err.h"
#include "session.h"
#include "dsp.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bluetooth_init(void);

/* Notification helpers — called from ble_notify_task */
void gatt_notify_flow   (float flow_lpm, bool is_inhaling, bool is_exhaling);
void gatt_notify_env    (float temp_c, float humidity_pct);
void gatt_indicate_result(const session_result_t *r);
void gatt_notify_error  (uint8_t error_code);

/* Called by session FSM on state changes */
void ble_notify_state(session_state_t s, const session_context_t *ctx);

/* FreeRTOS task — Core 0 */
void ble_notify_task(void *pvParameters);

/* Advertising interval control (for power management) */
void ble_set_adv_interval(uint16_t min_ms, uint16_t max_ms);

#ifdef __cplusplus
}
#endif
