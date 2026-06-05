#pragma once
/**
 * @file  befine_config.h
 * @brief Compile-time constants for the BeFine Smart Inhaler firmware.
 *
 * All tuneable parameters are centralised here so they can be overridden
 * via idf.py menuconfig or sdkconfig.defaults without touching source files.
 */

#ifdef __cplusplus
extern "C" {
#endif

/* ── Firmware version ────────────────────────────────────────────────────── */
#define BEFINE_FW_MAJOR          1
#define BEFINE_FW_MINOR          0
#define BEFINE_HW_REV            1

/* ── I2C bus ─────────────────────────────────────────────────────────────── */
#define BEFINE_I2C_PORT          I2C_NUM_0
#define BEFINE_I2C_SDA_GPIO      21
#define BEFINE_I2C_SCL_GPIO      22
#define BEFINE_I2C_FREQ_HZ       400000   /**< 400 kHz Fast-mode */

/* ── Sensor I2C addresses ────────────────────────────────────────────────── */
#define BEFINE_SDP3X_ADDR        0x21
#define BEFINE_SHT4X_ADDR        0x44
#define BEFINE_MPU6050_ADDR      0x68
#define BEFINE_SSD1306_ADDR      0x3C

/* ── GPIO assignments ────────────────────────────────────────────────────── */
#define BEFINE_QRE_LEFT_GPIO     34    /**< Spray insert detector — left  */
#define BEFINE_QRE_RIGHT_GPIO    35    /**< Spray insert detector — right */
#define BEFINE_LED_GPIO          48    /**< WS2812B data line (RMT)       */
#define BEFINE_BUZZER_GPIO       38    /**< Passive piezo (LEDC PWM)      */

/* ── SPI (SD card) ───────────────────────────────────────────────────────── */
#define BEFINE_SD_MOSI_GPIO      11
#define BEFINE_SD_MISO_GPIO      13
#define BEFINE_SD_CLK_GPIO       12
#define BEFINE_SD_CS_GPIO        10
#define BEFINE_SD_MOUNT_POINT    "/sdcard"

/* ── SPIFFS ──────────────────────────────────────────────────────────────── */
#define BEFINE_SPIFFS_BASE_PATH  "/spiffs"
#define BEFINE_SPIFFS_MAX_FILES  20

/* ── Sampling & DSP ─────────────────────────────────────────────────────── */
#define BEFINE_SAMPLE_RATE_HZ    50       /**< SDP31 polling + BLE notify rate */
#define BEFINE_SAMPLE_PERIOD_MS  (1000 / BEFINE_SAMPLE_RATE_HZ)   /* 20 ms */
#define BEFINE_IIR_CUTOFF_HZ     10       /**< Low-pass filter cutoff          */

/* ── Session thresholds ─────────────────────────────────────────────────── */
#define BEFINE_SHAKE_THRESHOLD_G   1.5f   /**< MPU6050 shake detection (g)     */
#define BEFINE_FLOW_MIN_LPM        30.0f  /**< Minimum PEF for valid session    */
#define BEFINE_FLOW_ONSET_LPM      0.5f   /**< Flow onset (INSERTING→INHALING)  */
#define BEFINE_FLOW_END_LPM        0.1f   /**< Flow end  (INHALING→HOLDING)     */
#define BEFINE_HOLD_TARGET_SEC     10     /**< Breath-hold duration target (s)  */
#define BEFINE_FEV1_SAMPLE_INDEX   50     /**< Sample at t=1s (50 Hz × 1s)      */

/* ── BLE ─────────────────────────────────────────────────────────────────── */
#define BEFINE_BLE_DEVICE_NAME      "BeFine"
#define BEFINE_BLE_ADV_INTERVAL_MS  100   /**< Advertisement interval (ms)      */
#define BEFINE_BLE_MTU              247   /**< Negotiated MTU (BLE 5 DLE)       */

/* ── Power management ───────────────────────────────────────────────────── */
#define BEFINE_IDLE_SLEEP_MIN      5      /**< Deep sleep after N minutes idle  */
#define BEFINE_SLEEP_WAKEUP_SEC    30     /**< RTC wakeup interval (s)          */

/* ── FreeRTOS task parameters ───────────────────────────────────────────── */
#define BEFINE_TASK_STACK_SENSOR   4096
#define BEFINE_TASK_STACK_DSP      4096
#define BEFINE_TASK_STACK_FSM      4096
#define BEFINE_TASK_STACK_BLE_NTF  4096
#define BEFINE_TASK_STACK_DISPLAY  4096
#define BEFINE_TASK_STACK_STORAGE  8192
#define BEFINE_TASK_STACK_PM       2048

#define BEFINE_TASK_PRIO_SENSOR    10
#define BEFINE_TASK_PRIO_DSP       9
#define BEFINE_TASK_PRIO_FSM       8
#define BEFINE_TASK_PRIO_BLE_NTF   7
#define BEFINE_TASK_PRIO_DISPLAY   5
#define BEFINE_TASK_PRIO_STORAGE   3
#define BEFINE_TASK_PRIO_PM        2

/* ── Inter-task queue depths ────────────────────────────────────────────── */
#define BEFINE_QUEUE_SENSOR_DEPTH   10
#define BEFINE_QUEUE_DSP_DEPTH      10
#define BEFINE_QUEUE_SESSION_DEPTH   5
#define BEFINE_QUEUE_STORAGE_DEPTH  20

/* ── NVS namespace & keys ───────────────────────────────────────────────── */
#define BEFINE_NVS_NAMESPACE     "befine"
#define BEFINE_NVS_KEY_ZERO_OFF  "zero_offset"   /**< float — zero-flow offset (Pa) */
#define BEFINE_NVS_KEY_SPAN      "span_const"    /**< float — orifice constant       */
#define BEFINE_NVS_KEY_PATIENT   "patient_id"    /**< uint32 — patient identifier    */

/* ── Default calibration (overwritten after factory calibration) ─────────── */
#define BEFINE_CALIB_ZERO_DEFAULT  0.0f
#define BEFINE_CALIB_SPAN_DEFAULT  8.5f   /**< L/min per √Pa                   */

#ifdef __cplusplus
}
#endif
