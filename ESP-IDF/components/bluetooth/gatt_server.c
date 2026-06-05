/**
 * @file  gatt_server.c
 * @brief GATT attribute table and event handler for BeFine service.
 */

#include "bluetooth.h"
#include "gatt_profile.h"
#include "session.h"
#include "befine_config.h"
#include "esp_log.h"
#include "esp_gatts_api.h"
#include "esp_gap_ble_api.h"
#include "esp_gatt_common_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "GATT_SRV";

extern EventGroupHandle_t g_ble_events;
#define BLE_BIT_CONNECTED  (1 << 0)
#define BLE_BIT_SUBSCRIBED (1 << 1)

/* Active connection handle */
static uint16_t s_conn_id  = 0xFFFF;
static esp_gatt_if_t s_gatts_if = ESP_GATT_IF_NONE;
static uint16_t s_handle_table[GATT_IDX_COUNT];

/* ── UUID helpers ────────────────────────────────────────────────────────── */
static const uint16_t PRIMARY_SERVICE_UUID   = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t CHAR_DECL_UUID         = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t CHAR_CFG_UUID          = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;

static const uint16_t SVC_UUID              = BEFINE_SERVICE_UUID;
static const uint16_t FLOW_UUID             = BEFINE_FLOW_RATE_UUID;
static const uint16_t RESULT_UUID           = BEFINE_RESULT_UUID;
static const uint16_t ENV_UUID              = BEFINE_ENVIRONMENT_UUID;
static const uint16_t CTRL_UUID             = BEFINE_SESSION_CTRL_UUID;
static const uint16_t INFO_UUID             = BEFINE_DEVICE_INFO_UUID;
static const uint16_t ERR_UUID              = BEFINE_ERROR_STATUS_UUID;

static const uint8_t CHAR_PROP_NOTIFY       = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static const uint8_t CHAR_PROP_INDICATE     = ESP_GATT_CHAR_PROP_BIT_INDICATE;
static const uint8_t CHAR_PROP_WRITE        = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t CHAR_PROP_READ         = ESP_GATT_CHAR_PROP_BIT_READ;

static uint16_t s_cccd_flow = 0;
static uint16_t s_cccd_result = 0;
static uint16_t s_cccd_env = 0;

/* Device info — read-only characteristic value */
static const uint8_t s_device_info[DEVICE_INFO_FRAME_LEN] = {
    BEFINE_FW_MAJOR, BEFINE_FW_MINOR, BEFINE_HW_REV, 100 /* battery placeholder */
};

/* ── Attribute table ─────────────────────────────────────────────────────── */
static const esp_gatts_attr_db_t s_gatt_db[GATT_IDX_COUNT] = {

    /* BeFine Spirometry Service */
    [IDX_SVC] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&PRIMARY_SERVICE_UUID, ESP_GATT_PERM_READ,
         sizeof(SVC_UUID), sizeof(SVC_UUID), (uint8_t *)&SVC_UUID}
    },

    /* Flow Rate — Notify (0xBF01) */
    [IDX_CHAR_FLOW_RATE] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_DECL_UUID, ESP_GATT_PERM_READ,
         1, 1, (uint8_t *)&CHAR_PROP_NOTIFY}
    },
    [IDX_CHAR_VAL_FLOW_RATE] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&FLOW_UUID, ESP_GATT_PERM_READ,
         FLOW_RATE_FRAME_LEN, 0, NULL}
    },
    [IDX_CHAR_CFG_FLOW_RATE] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_CFG_UUID,
         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
         sizeof(s_cccd_flow), sizeof(s_cccd_flow), (uint8_t *)&s_cccd_flow}
    },

    /* FEV1/PEF Result — Indicate (0xBF02) */
    [IDX_CHAR_RESULT] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_DECL_UUID, ESP_GATT_PERM_READ,
         1, 1, (uint8_t *)&CHAR_PROP_INDICATE}
    },
    [IDX_CHAR_VAL_RESULT] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&RESULT_UUID, ESP_GATT_PERM_READ,
         RESULT_FRAME_LEN, 0, NULL}
    },
    [IDX_CHAR_CFG_RESULT] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_CFG_UUID,
         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
         sizeof(s_cccd_result), sizeof(s_cccd_result), (uint8_t *)&s_cccd_result}
    },

    /* Environment — Notify (0xBF03) */
    [IDX_CHAR_ENVIRONMENT] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_DECL_UUID, ESP_GATT_PERM_READ,
         1, 1, (uint8_t *)&CHAR_PROP_NOTIFY}
    },
    [IDX_CHAR_VAL_ENVIRONMENT] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&ENV_UUID, ESP_GATT_PERM_READ,
         ENVIRONMENT_FRAME_LEN, 0, NULL}
    },
    [IDX_CHAR_CFG_ENVIRONMENT] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_CFG_UUID,
         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
         sizeof(s_cccd_env), sizeof(s_cccd_env), (uint8_t *)&s_cccd_env}
    },

    /* Session Control — Write (0xBF04) */
    [IDX_CHAR_SESSION_CTRL] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_DECL_UUID, ESP_GATT_PERM_READ,
         1, 1, (uint8_t *)&CHAR_PROP_WRITE}
    },
    [IDX_CHAR_VAL_SESSION_CTRL] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CTRL_UUID,
         ESP_GATT_PERM_WRITE, SESSION_CTRL_FRAME_LEN, 0, NULL}
    },

    /* Device Info — Read (0xBF05) */
    [IDX_CHAR_DEVICE_INFO] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_DECL_UUID, ESP_GATT_PERM_READ,
         1, 1, (uint8_t *)&CHAR_PROP_READ}
    },
    [IDX_CHAR_VAL_DEVICE_INFO] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&INFO_UUID, ESP_GATT_PERM_READ,
         DEVICE_INFO_FRAME_LEN, DEVICE_INFO_FRAME_LEN, (uint8_t *)s_device_info}
    },

    /* Error Status — Notify (0xBF06) */
    [IDX_CHAR_ERROR_STATUS] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_DECL_UUID, ESP_GATT_PERM_READ,
         1, 1, (uint8_t *)&CHAR_PROP_NOTIFY}
    },
    [IDX_CHAR_VAL_ERROR_STATUS] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&ERR_UUID, ESP_GATT_PERM_READ,
         ERROR_STATUS_FRAME_LEN, 0, NULL}
    },
    [IDX_CHAR_CFG_ERROR_STATUS] = {
        {ESP_GATT_AUTO_RSP},
        {ESP_UUID_LEN_16, (uint8_t *)&CHAR_CFG_UUID,
         ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
         2, 2, NULL}
    },
};

/* ── Session control write handler ─────────────────────────────────────── */
static void handle_session_ctrl_write(const uint8_t *data, uint16_t len)
{
    if (len < 1) return;
    switch (data[0]) {
        case 0x01: session_fsm_post_event(SESSION_EVT_BLE_START);   break;
        case 0x02: session_fsm_post_event(SESSION_EVT_BLE_STOP);    break;
        case 0x03: session_fsm_post_event(SESSION_EVT_RESET);       break;
        case 0xF0: ESP_LOGI(TAG, "Calibration command received");   break;
        default:   ESP_LOGW(TAG, "Unknown ctrl cmd: 0x%02X", data[0]); break;
    }
}

/* ── GATTS event handler ─────────────────────────────────────────────────── */
void gatts_event_handler(esp_gatts_cb_event_t event,
                          esp_gatt_if_t gatts_if,
                          esp_ble_gatts_cb_param_t *param)
{
    switch (event) {
    case ESP_GATTS_REG_EVT:
        s_gatts_if = gatts_if;
        esp_ble_gatts_create_attr_tab(s_gatt_db, gatts_if,
                                      GATT_IDX_COUNT, 0);
        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT:
        if (param->add_attr_tab.status != ESP_GATT_OK) {
            ESP_LOGE(TAG, "Attr table creation failed: 0x%x",
                     param->add_attr_tab.status);
        } else {
            memcpy(s_handle_table, param->add_attr_tab.handles,
                   sizeof(s_handle_table));
            esp_ble_gatts_start_service(
                s_handle_table[IDX_SVC]);
            ESP_LOGI(TAG, "GATT service started — %d attributes", GATT_IDX_COUNT);
        }
        break;

    case ESP_GATTS_CONNECT_EVT:
        s_conn_id = param->connect.conn_id;
        xEventGroupSetBits(g_ble_events, BLE_BIT_CONNECTED);
        ESP_LOGI(TAG, "Client connected, conn_id=%d", s_conn_id);
        break;

    case ESP_GATTS_DISCONNECT_EVT:
        s_conn_id = 0xFFFF;
        xEventGroupClearBits(g_ble_events,
                             BLE_BIT_CONNECTED | BLE_BIT_SUBSCRIBED);
        esp_ble_gap_start_advertising(NULL);   /* Restart advertising */
        ESP_LOGI(TAG, "Client disconnected");
        break;

    case ESP_GATTS_WRITE_EVT:
        if (!param->write.is_prep &&
            param->write.handle == s_handle_table[IDX_CHAR_VAL_SESSION_CTRL]) {
            handle_session_ctrl_write(param->write.value, param->write.len);
        }
        /* CCCD writes handled automatically by ESP_GATT_AUTO_RSP */
        break;

    default:
        break;
    }
}

/* ── Notification helpers ────────────────────────────────────────────────── */
void gatt_notify_flow(float flow_lpm, bool is_inhaling, bool is_exhaling)
{
    if (s_conn_id == 0xFFFF) return;
    uint8_t frame[FLOW_RATE_FRAME_LEN];
    uint16_t raw = (uint16_t)(flow_lpm * 100.0f);
    frame[0] = (uint8_t)(raw & 0xFF);
    frame[1] = (uint8_t)(raw >> 8);
    frame[2] = (uint8_t)((is_inhaling ? 0x01 : 0) |
                          (is_exhaling ? 0x02 : 0) | 0x04);  /* bit2=valid */
    esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id,
        s_handle_table[IDX_CHAR_VAL_FLOW_RATE],
        sizeof(frame), frame, false);
}

void gatt_notify_env(float temp_c, float humidity_pct)
{
    if (s_conn_id == 0xFFFF) return;
    uint8_t frame[ENVIRONMENT_FRAME_LEN];
    int16_t  t = (int16_t)(temp_c * 100.0f);
    uint16_t h = (uint16_t)(humidity_pct * 100.0f);
    frame[0] = (uint8_t)(t & 0xFF);
    frame[1] = (uint8_t)(t >> 8);
    frame[2] = (uint8_t)(h & 0xFF);
    frame[3] = (uint8_t)(h >> 8);
    esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id,
        s_handle_table[IDX_CHAR_VAL_ENVIRONMENT],
        sizeof(frame), frame, false);
}

void gatt_indicate_result(const session_result_t *r)
{
    if (s_conn_id == 0xFFFF) return;
    uint8_t frame[RESULT_FRAME_LEN];
    memcpy(&frame[0], &r->fev1_l,  4);
    memcpy(&frame[4], &r->pef_lpm, 4);
    frame[8] = r->technique_score;
    esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id,
        s_handle_table[IDX_CHAR_VAL_RESULT],
        sizeof(frame), frame, true);  /* true = Indicate (ACK required) */
}

void gatt_notify_error(uint8_t error_code)
{
    if (s_conn_id == 0xFFFF) return;
    uint8_t frame[ERROR_STATUS_FRAME_LEN];
    uint32_t ts = (uint32_t)(esp_timer_get_time() / 1000ULL);
    frame[0] = error_code;
    frame[1] = (uint8_t)(ts & 0xFF);
    frame[2] = (uint8_t)(ts >> 8);
    frame[3] = (uint8_t)(ts >> 16);
    frame[4] = (uint8_t)(ts >> 24);
    esp_ble_gatts_send_indicate(s_gatts_if, s_conn_id,
        s_handle_table[IDX_CHAR_VAL_ERROR_STATUS],
        sizeof(frame), frame, false);
}

void ble_notify_state(session_state_t s, const session_context_t *ctx)
{
    if (s == SESSION_STATE_COMPLETE) {
        gatt_indicate_result(&ctx->last_result);
    }
    if (s == SESSION_STATE_ERROR) {
        gatt_notify_error(0xFF);
    }
}
