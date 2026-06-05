# 🌬️ BeFine — ESP32 Firmware (ESP-IDF)

> **Real-Time Smart Inhaler Firmware**
> Onboard measurement · BLE GATT Server · Sensor fusion · Low-power · Open-source

[![ESP-IDF](https://img.shields.io/badge/ESP--IDF-v5.2+-red?logo=espressif)](https://docs.espressif.com/projects/esp-idf/en/latest/)
[![Target](https://img.shields.io/badge/Target-ESP32--S3-blue)](https://www.espressif.com/en/products/socs/esp32-s3)
[![License](https://img.shields.io/badge/License-MIT-green)](LICENSE)
[![MISRA-C](https://img.shields.io/badge/Code%20Style-MISRA--C%20inspired-orange)]()

---

## 📋 Table of Contents

- [System Architecture](#system-architecture)
- [Hardware Bill of Materials](#hardware-bill-of-materials)
- [Project Structure](#project-structure)
- [Component Deep-Dives](#component-deep-dives)
  - [Sensors](#sensors-component)
  - [Flow Measurement & DSP](#flow-measurement--dsp)
  - [BLE GATT Server](#ble-gatt-server)
  - [Session State Machine](#session-state-machine)
  - [Display & UI](#display--ui)
  - [Storage](#storage)
  - [Power Management](#power-management)
- [Real-Time Design (FreeRTOS)](#real-time-design-freertos)
- [GATT Protocol Specification](#gatt-protocol-specification)
- [Calibration Procedure](#calibration-procedure)
- [Building & Flashing](#building--flashing)
- [Menuconfig Reference](#menuconfig-reference)
- [OTA Updates](#ota-updates)
- [Contributing](#contributing)

---

## System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                          ESP32-S3 SoC                               │
│                                                                     │
│  Core 0 (PRO_CPU)                 Core 1 (APP_CPU)                  │
│  ┌─────────────────┐              ┌───────────────────────────┐     │ 
│  │  BLE Stack      │              │  Sensor Task (50 Hz)      │     │
│  │  (Bluedroid)    │              │  ┌─────────────────────┐  │     │
│  │                 │              │  │ SDP3x → Flow rate   │  │     │
│  │  GATT Server    │◄────────────►│  │ SHT4x → Temp/RH     │  │     │
│  │  Notify loop    │  Queue       │  │ MPU6050 → Shake     │  │     │
│  │                 │              │  └─────────────────────┘  │     │
│  └─────────────────┘              │                           │     │
│                                   │  DSP Task (50 Hz)         │     │
│  ┌─────────────────┐              │  ┌─────────────────────┐  │     │
│  │  Display Task   │              │  │ IIR Low-pass filter │  │     │
│  │  (SSD1306 I2C)  │              │  │ Peak detection      │  │     │
│  │                 │              │  │ FEV1 / PEF calc     │  │     │
│  └─────────────────┘              │  └─────────────────────┘  │     │
│                                   │                           │     │
│  ┌─────────────────┐              │  Session FSM Task         │     │
│  │  Storage Task   │◄────────────►│  ┌─────────────────────┐  │     │
│  │  (SPIFFS/SD)    │              │  │ IDLE→SHAKE→INSERT   │  │     │
│  └─────────────────┘              │  │ →INHALE→HOLD→DONE   │  │     │
│                                   │  └─────────────────────┘  │     │
│  ┌──────────────────────────────────────────────────────────┐ │     │
│  │  Power Manager (esp_pm + ULP co-processor)               │ │     │
│  └──────────────────────────────────────────────────────────┘ │     │
└─────────────────────────────────────────────────────────────────────┘
           │ I2C (400 kHz)             │ SPI             │ GPIO
    ┌──────┼──────┐              ┌─────┴───┐        ┌────┴────┐
    │      │      │              │  SD Card│        │  QRE    │
  SDP3x SHT4x MPU6050            └─────────┘        │ Sensors │
  OLED                                              └─────────┘
```

**Inter-task communication** uses **FreeRTOS queues** and **event groups** — no shared mutable state accessed without synchronization.

---

## Hardware Bill of Materials

| Component | Part | Interface | Notes |
|---|---|---|---|
| MCU | ESP32-S3 (8 MB Flash, PSRAM) | — | Dual-core 240 MHz, BLE 5.0 |
| Flow sensor | Sensirion SDP31 | I2C (0x21) | ±500 Pa, 16-bit, 50 Hz |
| Temp/RH | Sensirion SHT41 | I2C (0x44) | ±0.2°C, ±1.8 %rH |
| IMU | InvenSense MPU-6050 | I2C (0x68) | 6-axis, used for shake detection |
| OLED | 0.96" SSD1306 128×64 | I2C (0x3C) | Adafruit-compatible |
| Spray sensor | ROHM QRE1113 × 2 | GPIO ADC | Reflective IR, insert detection |
| RGB LED | WS2812B | GPIO (RMT) | Session status indication |
| Buzzer | Passive piezo | GPIO (LEDC PWM) | Auditory cues |
| Storage | MicroSD (SPI) | SPI (VSPI) | FAT32, session CSV logs |
| Battery | LiPo 3.7V 1200 mAh | I2C (IP5306) | With IP5306 charger/fuel gauge |
| Regulator | AMS1117-3.3 | — | 3.3V rail |

**I2C Bus Map:**

```
I2C Master (GPIO21=SDA, GPIO22=SCL, 400 kHz)
├── 0x21 → SDP31 (flow)
├── 0x44 → SHT41 (temp/RH)
├── 0x3C → SSD1306 (OLED)
└── 0x68 → MPU-6050 (IMU)
```

---

## Project Structure

```
befine-firmware/
├── CMakeLists.txt
├── sdkconfig.defaults               # Pre-configured menuconfig defaults
├── partitions.csv                   # Custom partition table (OTA + SPIFFS + NVS)
├── main/
│   ├── CMakeLists.txt
│   ├── befine_main.c                # Entry point: init + task launch
│   └── include/
│       └── befine_config.h          # Compile-time constants
│
├── components/
│   ├── sensors/                     # SDP3x, SHT4x, MPU6050 drivers
│   │   ├── CMakeLists.txt
│   │   ├── sdp3x.c / sdp3x.h        # Differential pressure driver
│   │   ├── sht4x.c / sht4x.h        # Temp/humidity driver
│   │   ├── mpu6050.c / mpu6050.h    # IMU driver
│   │   ├── sensors.c                # Aggregated sensor task
│   │   └── include/sensors.h
│   │
│   ├── dsp/                         # Signal processing
│   │   ├── CMakeLists.txt
│   │   ├── iir_filter.c / iir_filter.h   # Biquad IIR (low-pass)
│   │   ├── peak_detector.c               # Flow peak & FEV1 integration
│   │   ├── flow_calculator.c             # L/min conversion + calibration
│   │   └── include/dsp.h
│   │
│   ├── session/                     # Session FSM
│   │   ├── CMakeLists.txt
│   │   ├── session_fsm.c
│   │   ├── session_fsm.h
│   │   └── include/session.h
│   │
│   ├── display/                     # OLED driver + UI screens
│   │   ├── CMakeLists.txt
│   │   ├── ssd1306.c / ssd1306.h    # Low-level OLED driver
│   │   ├── display.c                # High-level display API
│   │   ├── screens/
│   │   │   ├── screen_idle.c
│   │   │   ├── screen_guided.c      # Step-by-step procedure
│   │   │   ├── screen_flow.c        # Live flow bargraph
│   │   │   └── screen_summary.c     # Post-session results
│   │   └── include/display.h
│   │
│   ├── bluetooth/                   # BLE GATT server
│   │   ├── CMakeLists.txt
│   │   ├── ble_init.c               # NimBLE / Bluedroid setup
│   │   ├── gatt_server.c            # GATT attribute table
│   │   ├── gatt_profile.h           # UUID definitions (matches Flutter SDK)
│   │   ├── ble_notify.c             # Notification scheduler
│   │   └── include/bluetooth.h
│   │
│   ├── storage/                     # SPIFFS + SD card logging
│   │   ├── CMakeLists.txt
│   │   ├── spiffs_storage.c         # Session metadata (NVS + SPIFFS)
│   │   ├── sd_storage.c             # CSV raw log (SD card)
│   │   ├── session_record.c         # Serialization / deserialization
│   │   └── include/storage.h
│   │
│   └── power_mgmt/                  # Power saving
│       ├── CMakeLists.txt
│       ├── power_mgmt.c
│       └── include/power_mgmt.h
│
├── test/                            # Unity-based unit tests
│   ├── test_iir_filter.c
│   ├── test_frame_encode.c
│   ├── test_session_fsm.c
│   └── CMakeLists.txt
│
└── docs/
    ├── hardware_schematic.pdf
    ├── gatt_profile.md
    └── calibration_procedure.md
```

---

## Component Deep-Dives

### Sensors Component

#### SDP31 — Differential Pressure / Flow

The SDP31 outputs raw differential pressure in Pa. Flow rate is derived via the Bernoulli equation and a calibration lookup table burned into NVS at factory time.

```c
// components/sensors/sdp3x.c

#include "sdp3x.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define SDP3X_ADDR          0x21
#define SDP3X_CMD_START_CONT 0x3615   // Continuous measurement, mass flow
#define SDP3X_SCALE_FACTOR  60.0f     // Pa per LSB (SDP31)

static const char *TAG = "SDP3X";

esp_err_t sdp3x_init(void) {
    uint8_t cmd[2] = {
        (SDP3X_CMD_START_CONT >> 8) & 0xFF,
         SDP3X_CMD_START_CONT       & 0xFF,
    };
    esp_err_t ret = i2c_master_write_to_device(
        I2C_NUM_0, SDP3X_ADDR, cmd, sizeof(cmd),
        pdMS_TO_TICKS(100)
    );
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "SDP3X init failed: %s", esp_err_to_name(ret));
    }
    return ret;
}

esp_err_t sdp3x_read_measurement(sdp3x_measurement_t *out) {
    uint8_t raw[9] = {0};
    esp_err_t ret = i2c_master_read_from_device(
        I2C_NUM_0, SDP3X_ADDR, raw, sizeof(raw),
        pdMS_TO_TICKS(20)
    );
    if (ret != ESP_OK) return ret;

    // Verify CRC for each word (Sensirion CRC-8 polynomial 0x31)
    if (!sdp3x_check_crc(raw, 2, raw[2]) ||
        !sdp3x_check_crc(raw+3, 2, raw[5])) {
        return ESP_ERR_INVALID_CRC;
    }

    int16_t dp_raw  = (int16_t)((raw[0] << 8) | raw[1]);
    int16_t temp_raw = (int16_t)((raw[3] << 8) | raw[4]);

    out->differential_pa = (float)dp_raw / SDP3X_SCALE_FACTOR;
    out->temperature_c   = (float)temp_raw / 200.0f;
    return ESP_OK;
}
```

#### SHT41 — Temperature & Humidity

```c
// components/sensors/sht4x.c

esp_err_t sht4x_read(sht4x_data_t *out) {
    // Send measure command: High repeatability
    uint8_t cmd = 0xFD;
    i2c_master_write_to_device(I2C_NUM_0, 0x44, &cmd, 1, pdMS_TO_TICKS(10));
    vTaskDelay(pdMS_TO_TICKS(9));   // measurement time ≈ 8.3 ms

    uint8_t raw[6];
    i2c_master_read_from_device(I2C_NUM_0, 0x44, raw, 6, pdMS_TO_TICKS(10));

    uint16_t t_raw = (raw[0] << 8) | raw[1];
    uint16_t rh_raw = (raw[3] << 8) | raw[4];

    out->temperature_c = -45.0f + 175.0f * ((float)t_raw / 65535.0f);
    out->humidity_pct  = -6.0f  + 125.0f * ((float)rh_raw / 65535.0f);
    out->humidity_pct  = fmaxf(0.0f, fminf(100.0f, out->humidity_pct));
    return ESP_OK;
}
```

#### MPU6050 — Shake Detection

```c
// components/sensors/mpu6050.c

#define SHAKE_THRESHOLD_G  1.5f   // Configurable via NVS

bool mpu6050_is_shaking(void) {
    mpu6050_raw_t raw;
    mpu6050_read_accel(&raw);

    float ax = raw.accel_x / 16384.0f;  // ±2g range
    float ay = raw.accel_y / 16384.0f;
    float az = raw.accel_z / 16384.0f;

    float magnitude = sqrtf(ax*ax + ay*ay + az*az);
    return (magnitude > SHAKE_THRESHOLD_G);
}
```

---

### Flow Measurement & DSP

Raw differential pressure from the SDP31 is processed through a DSP pipeline running at **50 Hz**:

```
SDP31 raw DP (Pa)
        │
        ▼
┌──────────────────┐
│ IIR Low-pass     │  fc = 10 Hz, prevents turbulence noise
│ (2nd order       │
│ Butterworth)     │
└──────────────────┘
        │
        ▼
┌──────────────────┐
│ Flow Conversion  │  Q = C × sign(ΔP) × √|ΔP|
│ (Bernoulli)      │  C = calibration constant from NVS
└──────────────────┘
        │
        ▼
┌──────────────────┐
│ Volume integrator│  V(t) = ∫ Q dt   (trapezoid rule)
│ (FEV1, FVC calc) │  FEV1 = V at t=1.0s from onset
└──────────────────┘
        │
        ▼
┌──────────────────┐
│ Peak Detector    │  PEF = max(Q) within session window
└──────────────────┘
```

#### IIR Biquad Filter

```c
// components/dsp/iir_filter.c

typedef struct {
    float b0, b1, b2;   // numerator coefficients
    float a1, a2;        // denominator coefficients
    float z1, z2;        // delay elements (state)
} iir_biquad_t;

/**
 * @brief  Process one sample through a Direct Form II Transposed biquad.
 * @note   Thread-safe IF each task uses its own iir_biquad_t instance.
 */
float iir_biquad_process(iir_biquad_t *f, float x) {
    float y = f->b0 * x + f->z1;
    f->z1   = f->b1 * x - f->a1 * y + f->z2;
    f->z2   = f->b2 * x - f->a2 * y;
    return y;
}

/**
 * @brief  Pre-computed 2nd-order Butterworth LP coefficients for fc=10Hz, fs=50Hz.
 *         Generated with: python -c "from scipy.signal import butter; print(butter(2, 10/(50/2)))"
 */
static const iir_biquad_t LP_10HZ_50FS = {
    .b0 =  0.20657208f,
    .b1 =  0.41314417f,
    .b2 =  0.20657208f,
    .a1 = -0.36952738f,
    .a2 =  0.19581571f,
    .z1 =  0.0f,
    .z2 =  0.0f,
};
```

#### Flow Calculator

```c
// components/dsp/flow_calculator.c

#define ORIFICE_CONSTANT_DEFAULT  8.5f   // L/min per √Pa — from NVS calibration

float flow_from_pressure(float dp_pa, float calib_constant) {
    // Bernoulli: Q = C × sign(ΔP) × √|ΔP|
    float sign  = (dp_pa >= 0.0f) ? 1.0f : -1.0f;
    return sign * calib_constant * sqrtf(fabsf(dp_pa));
}

void flow_integrate_fev1(
    const float *flow_lpm,   // Array of flow samples at 50 Hz
    size_t       n_samples,
    float       *fev1_out,
    float       *pef_out
) {
    float volume = 0.0f;
    float peak   = 0.0f;
    const float dt_min = 1.0f / (50.0f * 60.0f);   // 50 Hz → minutes

    for (size_t i = 1; i < n_samples; i++) {
        // Trapezoid rule integration
        float avg = (flow_lpm[i-1] + flow_lpm[i]) * 0.5f;
        volume += avg * dt_min;                          // L

        if (flow_lpm[i] > peak) peak = flow_lpm[i];

        // FEV1: volume at exactly 1 second (sample 50)
        if (i == 50) *fev1_out = volume;
    }
    *pef_out = peak;
}
```

---

### BLE GATT Server

The firmware implements a GATT **peripheral** (server) using Bluedroid (IDF native). Characteristics match exactly the UUIDs consumed by the Flutter SDK.

```c
// components/bluetooth/gatt_profile.h

#define BEFINE_SERVICE_UUID         0xBF00
#define BEFINE_FLOW_RATE_UUID       0xBF01
#define BEFINE_RESULT_UUID          0xBF02
#define BEFINE_ENVIRONMENT_UUID     0xBF03
#define BEFINE_SESSION_CTRL_UUID    0xBF04
#define BEFINE_DEVICE_INFO_UUID     0xBF05
#define BEFINE_ERROR_STATUS_UUID    0xBF06
```

```c
// components/bluetooth/gatt_server.c

static const esp_gatts_attr_db_t befine_gatt_db[] = {

    // ── BeFine Spirometry Service Declaration ──────────────────────────
    [IDX_SVC] = {
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid,
          ESP_GATT_PERM_READ,
          sizeof(uint16_t), sizeof(BEFINE_SERVICE_UUID),
          (uint8_t *)&BEFINE_SERVICE_UUID }
    },

    // ── Flow Rate (Notify, 0xBF01) ────────────────────────────────────
    [IDX_CHAR_FLOW_RATE] = {
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid,
          ESP_GATT_PERM_READ,
          CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE,
          (uint8_t *)&char_prop_notify }
    },
    [IDX_CHAR_VAL_FLOW_RATE] = {
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16, (uint8_t *)&BEFINE_FLOW_RATE_UUID,
          ESP_GATT_PERM_READ,
          FLOW_RATE_FRAME_LEN, 0, NULL }
    },
    [IDX_CHAR_CFG_FLOW_RATE] = {  // CCCD (Client Characteristic Config)
        { ESP_GATT_AUTO_RSP },
        { ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid,
          ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
          sizeof(uint16_t), sizeof(uint16_t), (uint8_t *)&cccd_flow_rate }
    },

    // ── Environment (Notify, 0xBF03) ──────────────────────────────────
    /* ... same pattern ... */

    // ── Session Control (Write, 0xBF04) ───────────────────────────────
    [IDX_CHAR_SESSION_CTRL] = { /* ... char_prop_write ... */ },
    [IDX_CHAR_VAL_SESSION_CTRL] = { /* ... write callback dispatches command ... */ },
};

/**
 * @brief  Encode and notify a FlowMeasurement to all connected clients.
 *         Called from sensor task at 50 Hz.
 */
void gatt_notify_flow(float flow_lpm, bool is_inhaling) {
    uint8_t frame[FLOW_RATE_FRAME_LEN];
    uint16_t raw = (uint16_t)(flow_lpm * 100.0f);   // lpm × 100, matches Flutter parser
    uint8_t  flags = is_inhaling ? 0x01 : 0x00;

    frame[0] = raw & 0xFF;           // Little-endian
    frame[1] = (raw >> 8) & 0xFF;
    frame[2] = flags;

    esp_ble_gatts_send_indicate(
        gatts_if, conn_id,
        handle_table[IDX_CHAR_VAL_FLOW_RATE],
        sizeof(frame), frame,
        false   // false = Notify (no ACK), true = Indicate (ACK)
    );
}
```

#### Session Control Write Handler

```c
// components/bluetooth/gatt_server.c

static void handle_session_ctrl_write(const uint8_t *data, uint16_t len) {
    if (len < 1) return;
    switch (data[0]) {
        case 0x01:  // START
            session_fsm_post_event(SESSION_EVT_BLE_START);
            break;
        case 0x02:  // STOP
            session_fsm_post_event(SESSION_EVT_BLE_STOP);
            break;
        case 0x03:  // RESET
            session_fsm_post_event(SESSION_EVT_RESET);
            break;
        case 0xF0:  // CALIBRATE
            flow_calculator_start_calibration();
            break;
        default:
            ESP_LOGW(TAG, "Unknown session control command: 0x%02X", data[0]);
    }
}
```

---

### Session State Machine

The session lifecycle is managed as a **deterministic FSM** using FreeRTOS event queues:

```c
// components/session/session_fsm.h

typedef enum {
    SESSION_STATE_IDLE        = 0,
    SESSION_STATE_SHAKING     = 1,
    SESSION_STATE_INSERTING   = 2,
    SESSION_STATE_INHALING    = 3,
    SESSION_STATE_HOLDING     = 4,
    SESSION_STATE_COMPLETE    = 5,
    SESSION_STATE_ERROR       = 6,
} session_state_t;

typedef enum {
    SESSION_EVT_SHAKE_DETECTED    = 0,
    SESSION_EVT_SPRAY_INSERTED    = 1,
    SESSION_EVT_FLOW_ABOVE_MIN    = 2,
    SESSION_EVT_FLOW_BELOW_MIN    = 3,
    SESSION_EVT_HOLD_TIMEOUT      = 4,
    SESSION_EVT_HOLD_COMPLETE     = 5,
    SESSION_EVT_BLE_START         = 6,
    SESSION_EVT_BLE_STOP          = 7,
    SESSION_EVT_RESET             = 8,
    SESSION_EVT_ERROR             = 9,
} session_event_t;
```

```c
// components/session/session_fsm.c

typedef session_state_t (*transition_fn_t)(session_context_t *ctx);

/**
 * @brief  Transition table.
 *         Rows = current state, Cols = event.
 *         NULL = invalid transition (logged as warning).
 */
static const transition_fn_t fsm_table[SESSION_STATE_COUNT][SESSION_EVT_COUNT] = {
    //                      SHAKE             INSERTED           FLOW_ABOVE  FLOW_BELOW   HOLD_TIMEOUT  HOLD_COMPLETE  BLE_START         BLE_STOP      RESET         ERROR
    [SESSION_STATE_IDLE]   = { on_idle_shake,  NULL,             NULL,       NULL,        NULL,         NULL,          on_idle_ble_start, NULL,         NULL,         on_error },
    [SESSION_STATE_SHAKING]= { NULL,           on_shake_insert,  NULL,       NULL,        NULL,         NULL,          NULL,              on_ble_stop,  on_reset,     on_error },
    [SESSION_STATE_INSERTING]={ NULL,          NULL,             on_flow,    NULL,        NULL,         NULL,          NULL,              on_ble_stop,  on_reset,     on_error },
    [SESSION_STATE_INHALING]= { NULL,          NULL,             NULL,       on_hold_start,NULL,        NULL,          NULL,              on_ble_stop,  on_reset,     on_error },
    [SESSION_STATE_HOLDING] = { NULL,          NULL,             NULL,       NULL,        on_hold_timeout, on_complete, NULL,             on_ble_stop,  on_reset,     on_error },
    [SESSION_STATE_COMPLETE]= { NULL,          NULL,             NULL,       NULL,        NULL,         NULL,          NULL,              NULL,         on_reset,     NULL     },
};

static void session_fsm_task(void *pvParams) {
    session_context_t ctx = {0};
    session_event_t   evt;

    for (;;) {
        if (xQueueReceive(g_session_queue, &evt, portMAX_DELAY) == pdTRUE) {
            transition_fn_t fn = fsm_table[ctx.state][evt];
            if (fn != NULL) {
                session_state_t next = fn(&ctx);
                ESP_LOGI(TAG, "FSM: %s → %s",
                    session_state_name(ctx.state),
                    session_state_name(next));
                ctx.state = next;
                display_update_state(next, &ctx);
                ble_notify_state(next, &ctx);
            } else {
                ESP_LOGW(TAG, "FSM: Invalid transition state=%d evt=%d",
                    ctx.state, evt);
            }
        }
    }
}
```

**State transitions diagram:**

```
          SHAKE_DETECTED
IDLE ─────────────────────► SHAKING
                                │ SPRAY_INSERTED
                                ▼
                           INSERTING
                                │ FLOW_ABOVE_MIN
                                ▼
                           INHALING  ◄────┐
                                │         │ FLOW_ABOVE_MIN (sustained)
                          FLOW_BELOW_MIN  │
                                │         │
                                ▼         │
                            HOLDING ──────┘
                                │ HOLD_COMPLETE (10s)
                                ▼
                           COMPLETE
                                │ RESET
                                ▼
                              IDLE
```

---

### Display & UI

The 128×64 OLED displays context-specific screens for each FSM state:

```c
// components/display/screens/screen_flow.c

/**
 * @brief  Renders a real-time bargraph of flow rate (0–600 L/min full-scale).
 *         Called from display task at ~10 Hz (no need for 50 Hz screen refresh).
 */
void screen_flow_render(ssd1306_handle_t oled, float flow_lpm, bool is_inhaling) {
    ssd1306_clear(oled);

    // Title
    ssd1306_draw_string(oled, 0, 0, is_inhaling ? "INHALING" : "EXHALING", FONT_6x8);

    // Bargraph — 100px wide, 20px tall at y=16
    uint8_t bar_width = (uint8_t)fminf(flow_lpm / 600.0f * 100.0f, 100.0f);
    ssd1306_fill_rect(oled, 0, 16, bar_width, 20);
    ssd1306_draw_rect(oled, 0, 16, 100, 20);      // Border

    // Numeric value
    char buf[16];
    snprintf(buf, sizeof(buf), "%.1f L/min", flow_lpm);
    ssd1306_draw_string(oled, 0, 42, buf, FONT_8x8);

    ssd1306_flush(oled);
}
```

---

### Storage

Sessions are logged in two locations:

| Storage | Content | Format |
|---|---|---|
| NVS | Calibration constants, patient ID, firmware config | Key-value |
| SPIFFS | Session metadata (FEV1, PEF, score, timestamp) | JSON per session |
| SD Card | Raw flow samples for entire session | CSV (timestamp,flow_lpm) |

```c
// components/storage/session_record.c

esp_err_t session_record_save_metadata(const session_result_t *r) {
    char path[64];
    snprintf(path, sizeof(path), "/spiffs/session_%llu.json", r->session_id);

    FILE *f = fopen(path, "w");
    if (!f) return ESP_ERR_NOT_FOUND;

    fprintf(f,
        "{\"id\":%llu,\"ts\":%llu,\"fev1\":%.3f,\"pef\":%.1f,"
        "\"score\":%u,\"duration_ms\":%lu,\"fw\":\"%d.%d\"}\n",
        r->session_id, r->timestamp_unix,
        r->fev1_l, r->pef_lpm,
        r->technique_score, r->duration_ms,
        CONFIG_BEFINE_FW_MAJOR, CONFIG_BEFINE_FW_MINOR
    );

    fclose(f);
    return ESP_OK;
}
```

---

### Power Management

The device supports three power levels managed by `esp_pm`:

```c
// components/power_mgmt/power_mgmt.c

static const esp_pm_config_t PM_ACTIVE = {
    .max_freq_mhz = 240,
    .min_freq_mhz = 80,
    .light_sleep_enable = false,
};

static const esp_pm_config_t PM_IDLE = {
    .max_freq_mhz = 80,
    .min_freq_mhz = 10,
    .light_sleep_enable = true,   // Auto light-sleep between tasks
};

static const esp_pm_config_t PM_DEEP_SLEEP_PREP = {
    .max_freq_mhz = 40,
    .min_freq_mhz = 10,
    .light_sleep_enable = true,
};

void power_mgmt_set_mode(power_mode_t mode) {
    switch (mode) {
        case POWER_MODE_ACTIVE:
            esp_pm_configure(&PM_ACTIVE);
            display_set_brightness(255);
            break;
        case POWER_MODE_IDLE:
            esp_pm_configure(&PM_IDLE);
            display_set_brightness(64);
            break;
        case POWER_MODE_SLEEP:
            // BLE advertisement at 1285 ms interval to save power
            ble_set_adv_interval(0xA00, 0xA00);
            esp_pm_configure(&PM_DEEP_SLEEP_PREP);
            break;
    }
}

/**
 * @brief  Enter deep sleep if no activity for 5 minutes.
 *         WakeUp source: GPIO (spray insert) + RTC timer.
 */
void power_mgmt_check_idle_timeout(void) {
    if (xTaskGetTickCount() - g_last_activity_tick > pdMS_TO_TICKS(5 * 60 * 1000)) {
        ESP_LOGI(TAG, "Entering deep sleep");
        esp_sleep_enable_gpio_wakeup();
        esp_sleep_enable_timer_wakeup(30ULL * 1000 * 1000);   // 30s RTC wakeup for BLE beacon
        esp_deep_sleep_start();
    }
}
```

---

## Real-Time Design (FreeRTOS)

| Task | Core | Priority | Stack | Period |
|---|---|---|---|---|
| `sensor_task` | APP_CPU (1) | 10 | 4096 | 20 ms (50 Hz) |
| `dsp_task` | APP_CPU (1) | 9 | 4096 | 20 ms (50 Hz) |
| `session_fsm_task` | APP_CPU (1) | 8 | 4096 | Event-driven |
| `ble_notify_task` | PRO_CPU (0) | 7 | 4096 | 20 ms max |
| `display_task` | PRO_CPU (0) | 5 | 4096 | 100 ms (10 Hz) |
| `storage_task` | PRO_CPU (0) | 3 | 8192 | Event-driven |
| `power_mgmt_task` | PRO_CPU (0) | 2 | 2048 | 1000 ms |

**Queue depths:**

```c
// main/befine_main.c

void app_main(void) {
    // Inter-task communication
    g_sensor_queue  = xQueueCreate(10,  sizeof(sensor_data_t));    // SDP3x raw frames
    g_dsp_queue     = xQueueCreate(10,  sizeof(flow_sample_t));     // Processed flow
    g_session_queue = xQueueCreate(5,   sizeof(session_event_t));   // FSM events
    g_storage_queue = xQueueCreate(20,  sizeof(session_result_t));  // Async writes

    // Event groups for synchronization
    g_ble_events    = xEventGroupCreate();   // BIT0=connected, BIT1=subscribed

    // Launch tasks
    xTaskCreatePinnedToCore(sensor_task,      "sensor",   4096, NULL, 10, NULL, 1);
    xTaskCreatePinnedToCore(dsp_task,         "dsp",      4096, NULL, 9,  NULL, 1);
    xTaskCreatePinnedToCore(session_fsm_task, "fsm",      4096, NULL, 8,  NULL, 1);
    xTaskCreatePinnedToCore(ble_notify_task,  "ble_ntfy", 4096, NULL, 7,  NULL, 0);
    xTaskCreatePinnedToCore(display_task,     "display",  4096, NULL, 5,  NULL, 0);
    xTaskCreatePinnedToCore(storage_task,     "storage",  8192, NULL, 3,  NULL, 0);
    xTaskCreatePinnedToCore(power_mgmt_task,  "pm",       2048, NULL, 2,  NULL, 0);
}
```

---

## GATT Protocol Specification

### Frame Formats (all Little-Endian)

#### Flow Rate (0xBF01) — 3 bytes

```
Byte 0-1: uint16  flow_lpm × 100  (e.g., 0x04E2 = 1250 → 12.50 L/min)
Byte 2:   uint8   flags
                  bit 0 = is_inhaling
                  bit 1 = is_exhaling
                  bit 2 = measurement_valid
                  bit 7 = sensor_error
```

#### FEV1/PEF Result (0xBF02) — 9 bytes (Indicated after session end)

```
Byte 0-3: float32  FEV1 in litres
Byte 4-7: float32  PEF in L/min
Byte 8:   uint8    quality score (0=F, 1=D, 2=C, 3=B, 4=A)
```

#### Environment (0xBF03) — 4 bytes

```
Byte 0-1: int16   temperature × 100 (°C)   e.g., 0x0929 = 2345 → 23.45 °C
Byte 2-3: uint16  humidity × 100 (%rH)     e.g., 0x1964 = 6500 → 65.00 %
```

#### Session Control Write (0xBF04) — 1 byte

```
0x01 = START session
0x02 = STOP  session
0x03 = RESET device
0xF0 = CALIBRATE (zero-flow calibration)
```

#### Device Info (0xBF05) — 4 bytes (Read-only)

```
Byte 0: uint8  firmware major version
Byte 1: uint8  firmware minor version
Byte 2: uint8  hardware revision
Byte 3: uint8  battery percentage (0–100)
```

---

## Calibration Procedure

Zero-flow and span calibration must be performed after hardware assembly or sensor replacement.

```bash
# 1. Flash calibration firmware variant
idf.py -DBEFINE_CALIBRATION_MODE=1 build
idf.py -p /dev/ttyUSB0 flash

# 2. Keep inhaler open (no flow) for 5 seconds → zero-point captured to NVS

# 3. Connect calibrated flow meter at inlet, blow at exactly 100 L/min steady-state

# 4. Press MODE button → span calibration captured

# 5. Re-flash production firmware
idf.py build && idf.py -p /dev/ttyUSB0 flash
```

The calibration constants (`zero_offset_pa`, `span_constant`) are stored in NVS under the `befine` namespace and survive firmware updates.

---

## Building & Flashing

### Prerequisites

```bash
# Install ESP-IDF v5.2+
mkdir -p ~/esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
git checkout v5.2.1
./install.sh esp32s3
. ./export.sh
```

### Clone & Build

```bash
git clone https://github.com/befine-dev/befine-firmware.git
cd befine-firmware

# Configure target
idf.py set-target esp32s3

# (Optional) Adjust settings
idf.py menuconfig

# Build
idf.py build

# Flash + monitor
idf.py -p /dev/ttyUSB0 flash monitor
```

### Unit Tests (host-side with Unity)

```bash
cd test
idf.py -C . build
idf.py -C . flash monitor
```

---

## Menuconfig Reference

Navigate to `Component config → BeFine Configuration`:

| Option | Default | Description |
|---|---|---|
| `BEFINE_FLOW_SAMPLE_RATE_HZ` | 50 | SDP3x polling rate |
| `BEFINE_IIR_CUTOFF_HZ` | 10 | Low-pass filter cutoff |
| `BEFINE_HOLD_TARGET_SEC` | 10 | Breath-hold target (seconds) |
| `BEFINE_MIN_PEF_LPM` | 30.0 | Min PEF for valid session (L/min) |
| `BEFINE_BLE_ADV_INTERVAL_MS` | 100 | BLE advertisement interval |
| `BEFINE_IDLE_SLEEP_MIN` | 5 | Deep sleep after N minutes idle |
| `BEFINE_SD_LOGGING_ENABLED` | y | Log raw CSV to SD card |
| `BEFINE_SPIFFS_PARTITION_SIZE` | 1MB | SPIFFS partition |
| `BEFINE_OTA_ENABLED` | y | Enable OTA update support |

---

## OTA Updates

The partition table reserves two OTA slots (app0 / app1) plus a factory partition:

```csv
# partitions.csv
# Name,    Type, SubType,  Offset,   Size,    Flags
nvs,        data, nvs,      0x9000,   0x5000,
otadata,    data, ota,      0xe000,   0x2000,
app0,       app,  ota_0,    0x10000,  0x200000,
app1,       app,  ota_1,    0x210000, 0x200000,
spiffs,     data, spiffs,   0x410000, 0x100000,
```

OTA is triggered via a BLE write to a dedicated OTA control service (not exposed to end-users — only via the BeFine device management tool):

```bash
# CLI trigger (for factory / production line)
python tools/ota_push.py --port /dev/ttyUSB0 --firmware build/befine.bin
```

---

## Contributing

1. Read [`CONTRIBUTING.md`](CONTRIBUTING.md) and sign the CLA.
2. Branch naming: `feat/sensor-fusion`, `fix/ble-reconnect`, `refactor/fsm-cleanup`
3. All PRs must pass:
   - `idf.py build` (zero warnings with `-Werror`)
   - Unity unit tests
   - Static analysis: `idf.py clang-check`
4. Code style: **MISRA-C-inspired** (no dynamic allocation in sensor/DSP paths, no recursion, bounded loops)
5. Commit format: Conventional Commits (`feat(ble): add FEV6 characteristic`)

### Code Review Checklist

- [ ] No `malloc` / `free` in ISR or sensor task
- [ ] All queues have timeout (no `portMAX_DELAY` in non-blocking tasks)
- [ ] CRC verified on all I2C reads
- [ ] NVS writes are atomic (use `nvs_commit`)
- [ ] New characteristic UUIDs added to `gatt_profile.h` AND to the Flutter SDK's `befine_gatt_profile.dart`

---

## License

```
MIT License — Copyright (c) 2025 BeFine Dev
```

See [LICENSE](LICENSE) for full text.

---

*Precision engineering for better respiratory outcomes.*
