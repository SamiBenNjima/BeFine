#pragma once
/**
 * @file  session.h
 * @brief Session Finite State Machine — public interface.
 *
 * States:  IDLE → SHAKING → INSERTING → INHALING → HOLDING → COMPLETE → ERROR
 * Events:  posted via session_fsm_post_event() from any task (thread-safe).
 * Outputs: display_update_state() + ble_notify_state() called on each transition.
 */

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "dsp.h"   /* session_result_t */

#ifdef __cplusplus
extern "C" {
#endif

/* ── States ─────────────────────────────────────────────────────────────── */
typedef enum {
    SESSION_STATE_IDLE      = 0,
    SESSION_STATE_SHAKING   = 1,
    SESSION_STATE_INSERTING = 2,
    SESSION_STATE_INHALING  = 3,
    SESSION_STATE_HOLDING   = 4,
    SESSION_STATE_COMPLETE  = 5,
    SESSION_STATE_ERROR     = 6,
    SESSION_STATE_COUNT
} session_state_t;

/* ── Events ─────────────────────────────────────────────────────────────── */
typedef enum {
    SESSION_EVT_SHAKE_DETECTED  = 0,
    SESSION_EVT_SPRAY_INSERTED  = 1,
    SESSION_EVT_FLOW_ABOVE_MIN  = 2,
    SESSION_EVT_FLOW_BELOW_MIN  = 3,
    SESSION_EVT_HOLD_TIMEOUT    = 4,
    SESSION_EVT_HOLD_COMPLETE   = 5,
    SESSION_EVT_BLE_START       = 6,
    SESSION_EVT_BLE_STOP        = 7,
    SESSION_EVT_RESET           = 8,
    SESSION_EVT_ERROR           = 9,
    SESSION_EVT_COUNT
} session_event_t;

/* ── Context (runtime state) ────────────────────────────────────────────── */
typedef struct {
    session_state_t  state;
    uint64_t         session_id;
    uint32_t         state_entry_ms;   /* esp_timer tick at last transition */
    float            current_flow_lpm;
    session_result_t last_result;
} session_context_t;

/* ── Public API ─────────────────────────────────────────────────────────── */
esp_err_t session_fsm_init(void);
void      session_fsm_task(void *pvParameters);

/**
 * @brief  Post an event to the session FSM queue (thread-safe).
 *         Safe to call from ISR (uses xQueueSendFromISR if needed).
 */
void session_fsm_post_event(session_event_t evt);

session_state_t session_fsm_get_state(void);
const char     *session_state_name(session_state_t s);

#ifdef __cplusplus
}
#endif
