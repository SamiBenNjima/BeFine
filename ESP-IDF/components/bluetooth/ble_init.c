/**
 * @file  ble_init.c
 * @brief Bluedroid stack init, GAP advertising configuration.
 */

#include "bluetooth.h"
#include "gatt_profile.h"
#include "befine_config.h"
#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "BLE_INIT";

/* Advertising data */
static uint8_t s_adv_service_uuid128[16] = {
    /* BeFine Spirometry Service — 128-bit UUID derived from 0xBF00 base */
    0xFB, 0x34, 0x9B, 0x5F, 0x80, 0x00, 0x00, 0x80,
    0x00, 0x10, 0x00, 0x00, 0x00, 0xBF, 0x00, 0x00,
};

static esp_ble_adv_data_t s_adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = false,
    .min_interval        = 0x0006,
    .max_interval        = 0x0010,
    .appearance          = 0x0180,   /* Generic Health */
    .manufacturer_len    = 0,
    .p_manufacturer_data = NULL,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(s_adv_service_uuid128),
    .p_service_uuid      = s_adv_service_uuid128,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t s_adv_params = {
    .adv_int_min       = 0x00A0,   /* 100 ms */
    .adv_int_max       = 0x00A0,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

/* External GATT event handler — defined in gatt_server.c */
extern void gatts_event_handler(esp_gatts_cb_event_t event,
                                esp_gatt_if_t gatts_if,
                                esp_ble_gatts_cb_param_t *param);

static void gap_event_handler(esp_gap_ble_cb_event_t event,
                               esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&s_adv_params);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(TAG, "Advertising start failed");
        } else {
            ESP_LOGI(TAG, "Advertising started");
        }
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        ESP_LOGI(TAG, "Advertising stopped");
        break;
    default:
        break;
    }
}

esp_err_t bluetooth_init(void)
{
    esp_err_t ret;

    /* Release classic BT memory — we only use BLE */
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Controller init: %s", esp_err_to_name(ret)); return ret; }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Controller enable: %s", esp_err_to_name(ret)); return ret; }

    ret = esp_bluedroid_init();
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Bluedroid init: %s", esp_err_to_name(ret)); return ret; }

    ret = esp_bluedroid_enable();
    if (ret != ESP_OK) { ESP_LOGE(TAG, "Bluedroid enable: %s", esp_err_to_name(ret)); return ret; }

    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));
    ESP_ERROR_CHECK(esp_ble_gap_set_device_name(BEFINE_BLE_DEVICE_NAME));
    ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&s_adv_data));
    ESP_ERROR_CHECK(esp_ble_gatt_set_local_mtu(BEFINE_BLE_MTU));

    ESP_LOGI(TAG, "BLE stack initialised — advertising as \"%s\"", BEFINE_BLE_DEVICE_NAME);
    return ESP_OK;
}

void ble_set_adv_interval(uint16_t min_ms, uint16_t max_ms)
{
    s_adv_params.adv_int_min = (uint16_t)(min_ms * 1000 / 625);
    s_adv_params.adv_int_max = (uint16_t)(max_ms * 1000 / 625);
    esp_ble_gap_stop_advertising();
    esp_ble_gap_start_advertising(&s_adv_params);
}
