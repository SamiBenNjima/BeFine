#pragma once
/**
 * @file  gatt_profile.h
 * @brief BeFine GATT service and characteristic UUIDs.
 *
 * These constants MUST match befine_gatt_profile.dart in the Flutter SDK.
 * Any change here requires a matching change in the Dart package.
 *
 * Service  : BeFine Spirometry Service — UUID 0xBF00
 * Characteristics:
 *   0xBF01  Flow Rate        Notify   3 bytes  [uint16 flow×100 LE][uint8 flags]
 *   0xBF02  FEV1/PEF Result  Indicate 9 bytes  [float32 fev1][float32 pef][uint8 score]
 *   0xBF03  Environment      Notify   4 bytes  [int16 temp×100 LE][uint16 hum×100 LE]
 *   0xBF04  Session Control  Write    1 byte   [0x01=START 0x02=STOP 0x03=RESET 0xF0=CAL]
 *   0xBF05  Device Info      Read     4 bytes  [fw_major fw_minor hw_rev battery%]
 *   0xBF06  Error Status     Notify   5 bytes  [uint8 code][uint32 timestamp_ms LE]
 */

#pragma once
#include <stdint.h>

/* ── 16-bit UUIDs ───────────────────────────────────────────────────────── */
#define BEFINE_SERVICE_UUID          0xBF00u
#define BEFINE_FLOW_RATE_UUID        0xBF01u
#define BEFINE_RESULT_UUID           0xBF02u
#define BEFINE_ENVIRONMENT_UUID      0xBF03u
#define BEFINE_SESSION_CTRL_UUID     0xBF04u
#define BEFINE_DEVICE_INFO_UUID      0xBF05u
#define BEFINE_ERROR_STATUS_UUID     0xBF06u

/* ── Frame lengths (bytes) ──────────────────────────────────────────────── */
#define FLOW_RATE_FRAME_LEN          3u
#define RESULT_FRAME_LEN             9u
#define ENVIRONMENT_FRAME_LEN        4u
#define SESSION_CTRL_FRAME_LEN       1u
#define DEVICE_INFO_FRAME_LEN        4u
#define ERROR_STATUS_FRAME_LEN       5u

/* ── GATT attribute table indices ────────────────────────────────────────── */
typedef enum {
    IDX_SVC = 0,

    IDX_CHAR_FLOW_RATE,
    IDX_CHAR_VAL_FLOW_RATE,
    IDX_CHAR_CFG_FLOW_RATE,       /* CCCD */

    IDX_CHAR_RESULT,
    IDX_CHAR_VAL_RESULT,
    IDX_CHAR_CFG_RESULT,          /* CCCD */

    IDX_CHAR_ENVIRONMENT,
    IDX_CHAR_VAL_ENVIRONMENT,
    IDX_CHAR_CFG_ENVIRONMENT,     /* CCCD */

    IDX_CHAR_SESSION_CTRL,
    IDX_CHAR_VAL_SESSION_CTRL,

    IDX_CHAR_DEVICE_INFO,
    IDX_CHAR_VAL_DEVICE_INFO,

    IDX_CHAR_ERROR_STATUS,
    IDX_CHAR_VAL_ERROR_STATUS,
    IDX_CHAR_CFG_ERROR_STATUS,    /* CCCD */

    GATT_IDX_COUNT
} gatt_idx_t;
