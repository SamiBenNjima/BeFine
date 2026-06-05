/**
 * @file  befine_main.c
 * @brief BeFine Smart Inhaler — Firmware entry point.
 *
 * Responsibilities:
 *  1. Initialise shared hardware (NVS, I2C, SPI)
 *  2. Create all inter-task communication primitives (queues, event groups)
 *  3. Launch all FreeRTOS tasks pinned to their respective cores
 *
 * Core 0 (PRO_CPU): BLE stack, display, storage, power management
 * Core 1 (APP_CPU): sensor acquisition, DSP pipeline, session FSM
 */

#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/event_groups.h"

#include "esp_log.h"
#include "esp_err.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"

#include "befine_config.h"
#include "sensors.h"
#include "dsp.h"
#include "session.h"
#include "bluetooth.h"
#include "display.h"
#include "storage.h"
#include "power_mgmt.h"

static const char *TAG = "BEFINE_MAIN";

/* ── Global inter-task handles (extern in component headers) ────────────── */
QueueHandle_t    g_sensor_queue  = NULL;
QueueHandle_t    g_dsp_queue     = NULL;
QueueHandle_t    g_session_queue = NULL;
QueueHandle_t    g_storage_queue = NULL;
EventGroupHandle_t g_ble_events  = NULL;

/** BIT0 = BLE client connected, BIT1 = CCCD subscribed to flow notify */
#define BLE_BIT_CONNECTED   (1 << 0)
#define BLE_BIT_SUBSCRIBED  (1 << 1)

/* ── Forward declarations ───────────────────────────────────────────────── */
static esp_err_t init_nvs(void);
static esp_err_t init_i2c(void);
static void      create_queues(void);
static void      launch_tasks(void);

/* ─────────────────────────────────────────────────────────────────────────
 * app_main — called by ESP-IDF after system init
 * ───────────────────────────────────────────────────────────────────────── */
void app_main(void)
{
    ESP_LOGI(TAG, "BeFine Firmware v%d.%d (HW rev %d) starting...",
             BEFINE_FW_MAJOR, BEFINE_FW_MINOR, BEFINE_HW_REV);

    /* 1. Non-volatile storage — must be first (BLE stack needs NVS) */
    ESP_ERROR_CHECK(init_nvs());

    /* 2. I2C bus — sensors + OLED */
    ESP_ERROR_CHECK(init_i2c());

    /* 3. Initialise each component (hardware probing, one-time setup) */
    ESP_ERROR_CHECK(storage_init());      /* SPIFFS mount + SD card SPI */
    ESP_ERROR_CHECK(sensors_init());      /* SDP3x, SHT4x, MPU6050      */
    ESP_ERROR_CHECK(dsp_init());          /* IIR filter coefficients     */
    ESP_ERROR_CHECK(display_init());      /* SSD1306 OLED                */
    ESP_ERROR_CHECK(session_fsm_init());  /* FSM context reset           */
    ESP_ERROR_CHECK(bluetooth_init());    /* Bluedroid + GATT server     */
    ESP_ERROR_CHECK(power_mgmt_init());   /* esp_pm profiles             */

    /* 4. Create inter-task communication primitives */
    create_queues();

    /* 5. Splash screen while tasks spin up */
    display_show_splash();

    /* 6. Launch all FreeRTOS tasks */
    launch_tasks();

    ESP_LOGI(TAG, "All tasks launched — app_main exiting (idle task takes over)");
    /* app_main may return; the scheduler keeps running. */
}

/* ─────────────────────────────────────────────────────────────────────────
 * Private helpers
 * ───────────────────────────────────────────────────────────────────────── */

static esp_err_t init_nvs(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition truncated — erasing and re-initialising");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

static esp_err_t init_i2c(void)
{
    const i2c_config_t cfg = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = BEFINE_I2C_SDA_GPIO,
        .scl_io_num       = BEFINE_I2C_SCL_GPIO,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = BEFINE_I2C_FREQ_HZ,
    };
    esp_err_t ret = i2c_param_config(BEFINE_I2C_PORT, &cfg);
    if (ret != ESP_OK) return ret;
    return i2c_driver_install(BEFINE_I2C_PORT, cfg.mode, 0, 0, 0);
}

static void create_queues(void)
{
    g_sensor_queue  = xQueueCreate(BEFINE_QUEUE_SENSOR_DEPTH,
                                   sizeof(sensor_data_t));
    g_dsp_queue     = xQueueCreate(BEFINE_QUEUE_DSP_DEPTH,
                                   sizeof(flow_sample_t));
    g_session_queue = xQueueCreate(BEFINE_QUEUE_SESSION_DEPTH,
                                   sizeof(session_event_t));
    g_storage_queue = xQueueCreate(BEFINE_QUEUE_STORAGE_DEPTH,
                                   sizeof(session_result_t));
    g_ble_events    = xEventGroupCreate();

    configASSERT(g_sensor_queue);
    configASSERT(g_dsp_queue);
    configASSERT(g_session_queue);
    configASSERT(g_storage_queue);
    configASSERT(g_ble_events);

    ESP_LOGI(TAG, "Inter-task queues created");
}

static void launch_tasks(void)
{
    /* Core 1 — real-time acquisition & processing */
    xTaskCreatePinnedToCore(sensor_task,      "sensor",   BEFINE_TASK_STACK_SENSOR,
                            NULL, BEFINE_TASK_PRIO_SENSOR,  NULL, 1);
    xTaskCreatePinnedToCore(dsp_task,         "dsp",      BEFINE_TASK_STACK_DSP,
                            NULL, BEFINE_TASK_PRIO_DSP,     NULL, 1);
    xTaskCreatePinnedToCore(session_fsm_task, "fsm",      BEFINE_TASK_STACK_FSM,
                            NULL, BEFINE_TASK_PRIO_FSM,     NULL, 1);

    /* Core 0 — communication, UI, housekeeping */
    xTaskCreatePinnedToCore(ble_notify_task,  "ble_ntfy", BEFINE_TASK_STACK_BLE_NTF,
                            NULL, BEFINE_TASK_PRIO_BLE_NTF, NULL, 0);
    xTaskCreatePinnedToCore(display_task,     "display",  BEFINE_TASK_STACK_DISPLAY,
                            NULL, BEFINE_TASK_PRIO_DISPLAY, NULL, 0);
    xTaskCreatePinnedToCore(storage_task,     "storage",  BEFINE_TASK_STACK_STORAGE,
                            NULL, BEFINE_TASK_PRIO_STORAGE, NULL, 0);
    xTaskCreatePinnedToCore(power_mgmt_task,  "pm",       BEFINE_TASK_STACK_PM,
                            NULL, BEFINE_TASK_PRIO_PM,      NULL, 0);

    ESP_LOGI(TAG, "Tasks: sensor(%d Hz) dsp(%d Hz) fsm ble display storage pm",
             BEFINE_SAMPLE_RATE_HZ, BEFINE_SAMPLE_RATE_HZ);
}
