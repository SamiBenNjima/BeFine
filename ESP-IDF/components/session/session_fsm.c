/**
 * @file  session_fsm.c
 * @brief Deterministic session FSM driven by a FreeRTOS event queue.
 *
 * Transition table pattern:
 *   fsm_table[current_state][event] = handler_function | NULL (invalid)
 * Each handler receives the context, mutates it, and returns the next state.
 * Invalid transitions are logged as warnings but do not crash.
 */

#include "session.h"
#include "dsp.h"
#include "befine_config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/timers.h"

/* Forward-declared dependencies (implemented in bluetooth.c / display.c) */
extern void display_update_state(session_state_t s, const session_context_t *ctx);
extern void ble_notify_state    (session_state_t s, const session_context_t *ctx);
extern void storage_enqueue_result(const session_result_t *r);
extern QueueHandle_t g_session_queue;
extern QueueHandle_t g_storage_queue;

static const char *TAG = "SESSION_FSM";

/* ── Hold-timeout FreeRTOS timer ─────────────────────────────────────────── */
static TimerHandle_t s_hold_timer = NULL;

static void hold_timer_cb(TimerHandle_t xTimer)
{
    (void)xTimer;
    session_fsm_post_event(SESSION_EVT_HOLD_COMPLETE);
}

/* ── Transition handler type ─────────────────────────────────────────────── */
typedef session_state_t (*transition_fn_t)(session_context_t *ctx);

/* ── Transition handlers ─────────────────────────────────────────────────── */

static session_state_t on_idle_shake(session_context_t *ctx)
{
    ctx->session_id     = (uint64_t)(esp_timer_get_time() / 1000ULL);
    ctx->state_entry_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    dsp_reset_session();
    return SESSION_STATE_SHAKING;
}

static session_state_t on_idle_ble_start(session_context_t *ctx)
{
    return on_idle_shake(ctx);   /* Same entry actions */
}

static session_state_t on_shake_insert(session_context_t *ctx)
{
    ctx->state_entry_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    return SESSION_STATE_INSERTING;
}

static session_state_t on_insert_flow(session_context_t *ctx)
{
    ctx->state_entry_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    return SESSION_STATE_INHALING;
}

static session_state_t on_inhale_hold(session_context_t *ctx)
{
    ctx->state_entry_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    /* Start hold-timeout timer */
    xTimerChangePeriod(s_hold_timer,
                       pdMS_TO_TICKS(BEFINE_HOLD_TARGET_SEC * 1000), 0);
    xTimerStart(s_hold_timer, 0);
    return SESSION_STATE_HOLDING;
}

static session_state_t on_hold_resume(session_context_t *ctx)
{
    xTimerStop(s_hold_timer, 0);
    ctx->state_entry_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    return SESSION_STATE_INHALING;
}

static session_state_t on_complete(session_context_t *ctx)
{
    xTimerStop(s_hold_timer, 0);

    float fev1 = 0.0f, pef = 0.0f;
    uint8_t score = 0;
    dsp_get_result(&fev1, &pef, &score);

    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);
    ctx->last_result = (session_result_t){
        .session_id     = ctx->session_id,
        .timestamp_unix = ctx->session_id / 1000ULL,
        .fev1_l         = fev1,
        .pef_lpm        = pef,
        .technique_score = score,
        .duration_ms    = now_ms - ctx->state_entry_ms,
    };

    /* Enqueue for asynchronous storage */
    xQueueSend(g_storage_queue, &ctx->last_result, 0);

    return SESSION_STATE_COMPLETE;
}

static session_state_t on_ble_stop(session_context_t *ctx)
{
    xTimerStop(s_hold_timer, 0);
    dsp_reset_session();
    return SESSION_STATE_IDLE;
}

static session_state_t on_reset(session_context_t *ctx)
{
    return on_ble_stop(ctx);
}

static session_state_t on_error(session_context_t *ctx)
{
    xTimerStop(s_hold_timer, 0);
    return SESSION_STATE_ERROR;
}

/* ── Transition table ────────────────────────────────────────────────────── */
/*    Rows = current state, Cols = event                                      */
/*    NULL = invalid / ignored transition                                      */

static const transition_fn_t fsm_table[SESSION_STATE_COUNT][SESSION_EVT_COUNT] = {
/* State          SHAKE           INSERTED        FLOW_ABOVE      FLOW_BELOW    HOLD_TIMEOUT  HOLD_COMPLETE  BLE_START           BLE_STOP     RESET       ERROR     */
[SESSION_STATE_IDLE]     = {on_idle_shake, NULL,           NULL,           NULL,         NULL,         NULL,          on_idle_ble_start,  NULL,        NULL,       on_error },
[SESSION_STATE_SHAKING]  = {NULL,          on_shake_insert,NULL,           NULL,         NULL,         NULL,          NULL,               on_ble_stop, on_reset,   on_error },
[SESSION_STATE_INSERTING]= {NULL,          NULL,           on_insert_flow, NULL,         NULL,         NULL,          NULL,               on_ble_stop, on_reset,   on_error },
[SESSION_STATE_INHALING] = {NULL,          NULL,           NULL,           on_inhale_hold,NULL,        NULL,          NULL,               on_ble_stop, on_reset,   on_error },
[SESSION_STATE_HOLDING]  = {NULL,          NULL,           on_hold_resume, NULL,         on_complete,  on_complete,   NULL,               on_ble_stop, on_reset,   on_error },
[SESSION_STATE_COMPLETE] = {NULL,          NULL,           NULL,           NULL,         NULL,         NULL,          NULL,               NULL,        on_reset,   NULL     },
[SESSION_STATE_ERROR]    = {NULL,          NULL,           NULL,           NULL,         NULL,         NULL,          NULL,               NULL,        on_reset,   NULL     },
};

/* ── State names for logging ─────────────────────────────────────────────── */
static const char *const STATE_NAMES[SESSION_STATE_COUNT] = {
    "IDLE", "SHAKING", "INSERTING", "INHALING",
    "HOLDING", "COMPLETE", "ERROR"
};

const char *session_state_name(session_state_t s)
{
    return (s < SESSION_STATE_COUNT) ? STATE_NAMES[s] : "UNKNOWN";
}

/* ── Shared context ─────────────────────────────────────────────────────── */
static session_context_t s_ctx = {0};

session_state_t session_fsm_get_state(void) { return s_ctx.state; }

/* ── Init ───────────────────────────────────────────────────────────────── */
esp_err_t session_fsm_init(void)
{
    s_ctx.state = SESSION_STATE_IDLE;
    s_hold_timer = xTimerCreate("hold", pdMS_TO_TICKS(10000),
                                pdFALSE, NULL, hold_timer_cb);
    if (!s_hold_timer) return ESP_ERR_NO_MEM;
    ESP_LOGI(TAG, "FSM initialised — state: IDLE");
    return ESP_OK;
}

/* ── Post event (thread-safe) ───────────────────────────────────────────── */
void session_fsm_post_event(session_event_t evt)
{
    if (xQueueSend(g_session_queue, &evt, 0) != pdTRUE) {
        ESP_LOGW(TAG, "session_queue full — event %d dropped", evt);
    }
}

/* ── FSM task ───────────────────────────────────────────────────────────── */
void session_fsm_task(void *pvParameters)
{
    (void)pvParameters;
    session_event_t evt;

    ESP_LOGI(TAG, "FSM task started");

    for (;;) {
        if (xQueueReceive(g_session_queue, &evt, portMAX_DELAY) != pdTRUE) {
            continue;
        }

        if (evt >= SESSION_EVT_COUNT) {
            ESP_LOGE(TAG, "Unknown event %d", evt);
            continue;
        }

        transition_fn_t fn = fsm_table[s_ctx.state][evt];
        if (fn != NULL) {
            session_state_t prev = s_ctx.state;
            session_state_t next = fn(&s_ctx);
            s_ctx.state = next;

            ESP_LOGI(TAG, "  %s -[%d]-> %s",
                     session_state_name(prev), evt, session_state_name(next));

            /* Notify display and BLE of state change */
            display_update_state(next, &s_ctx);
            ble_notify_state(next, &s_ctx);
        } else {
            ESP_LOGW(TAG, "Invalid transition: state=%s evt=%d — ignored",
                     session_state_name(s_ctx.state), evt);
        }
    }
}
