/**
 * @file  test_session_fsm.c
 * @brief Unity tests for the session FSM transition table.
 *
 * These are host-side logic tests — no FreeRTOS queues used.
 * We directly call the FSM handlers by replaying events.
 */

#include "unity.h"
#include "session.h"

/* Stub implementations for dependencies */
void display_update_state(session_state_t s, const session_context_t *ctx)
{ (void)s; (void)ctx; }

void ble_notify_state(session_state_t s, const session_context_t *ctx)
{ (void)s; (void)ctx; }

void storage_enqueue_result(const session_result_t *r) { (void)r; }

/* ── Happy path ─────────────────────────────────────────────────────────── */

void test_full_happy_path(void)
{
    session_fsm_init();
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_IDLE, session_fsm_get_state());

    session_fsm_post_event(SESSION_EVT_SHAKE_DETECTED);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_SHAKING, session_fsm_get_state());

    session_fsm_post_event(SESSION_EVT_SPRAY_INSERTED);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_INSERTING, session_fsm_get_state());

    session_fsm_post_event(SESSION_EVT_FLOW_ABOVE_MIN);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_INHALING, session_fsm_get_state());

    session_fsm_post_event(SESSION_EVT_FLOW_BELOW_MIN);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_HOLDING, session_fsm_get_state());

    session_fsm_post_event(SESSION_EVT_HOLD_COMPLETE);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_COMPLETE, session_fsm_get_state());

    session_fsm_post_event(SESSION_EVT_RESET);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_IDLE, session_fsm_get_state());
}

void test_ble_stop_resets_from_inhaling(void)
{
    session_fsm_init();
    session_fsm_post_event(SESSION_EVT_BLE_START);
    vTaskDelay(1);
    session_fsm_post_event(SESSION_EVT_SPRAY_INSERTED);
    vTaskDelay(1);
    session_fsm_post_event(SESSION_EVT_FLOW_ABOVE_MIN);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_INHALING, session_fsm_get_state());

    session_fsm_post_event(SESSION_EVT_BLE_STOP);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_IDLE, session_fsm_get_state());
}

void test_invalid_transition_ignored(void)
{
    session_fsm_init();
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_IDLE, session_fsm_get_state());
    /* SPRAY_INSERTED from IDLE is invalid — should stay IDLE */
    session_fsm_post_event(SESSION_EVT_SPRAY_INSERTED);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_IDLE, session_fsm_get_state());
}

void test_error_resets_to_idle(void)
{
    session_fsm_init();
    session_fsm_post_event(SESSION_EVT_BLE_START);
    vTaskDelay(1);
    session_fsm_post_event(SESSION_EVT_ERROR);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_ERROR, session_fsm_get_state());
    session_fsm_post_event(SESSION_EVT_RESET);
    vTaskDelay(1);
    TEST_ASSERT_EQUAL_INT(SESSION_STATE_IDLE, session_fsm_get_state());
}

void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_full_happy_path);
    RUN_TEST(test_ble_stop_resets_from_inhaling);
    RUN_TEST(test_invalid_transition_ignored);
    RUN_TEST(test_error_resets_to_idle);
    UNITY_END();
}
