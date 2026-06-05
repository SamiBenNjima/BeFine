/**
 * @file  test_flow_calculator.c
 * @brief Unity tests for flow conversion and FEV1 integration.
 */

#include "unity.h"
#include "flow_calculator.h"
#include "befine_config.h"
#include <math.h>

void test_flow_from_pressure_positive(void)
{
    /* C=8.5, DP=100 Pa → Q = 8.5×√100 = 85 L/min */
    /* flow_calculator uses cached NVS span; override default */
    float q = flow_from_pressure(100.0f);
    /* With default span 8.5: 8.5 × 10 = 85.0 */
    TEST_ASSERT_FLOAT_WITHIN(1.0f, 85.0f, q);
}

void test_flow_from_pressure_negative(void)
{
    float q = flow_from_pressure(-100.0f);
    TEST_ASSERT_LESS_THAN(0.0f, q);
    TEST_ASSERT_FLOAT_WITHIN(1.0f, -85.0f, q);
}

void test_flow_from_pressure_zero(void)
{
    float q = flow_from_pressure(0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, q);
}

void test_fev1_integration(void)
{
    /* Constant flow of 60 L/min for 50 samples (1 second at 50 Hz)
     * Expected FEV1 = 60 L/min × 1 min/60 = 1.0 L */
    float samples[60];
    for (int i = 0; i < 60; i++) samples[i] = 60.0f;

    float fev1 = 0.0f, pef = 0.0f;
    flow_compute_results(samples, 60, &fev1, &pef);

    TEST_ASSERT_FLOAT_WITHIN(0.05f, 1.0f, fev1);
}

void test_pef_detection(void)
{
    /* Triangle: rises to 300 L/min at sample 25, falls back to 0 */
    float samples[50];
    for (int i = 0; i < 25; i++) samples[i]      = (float)i * 12.0f;
    for (int i = 25; i < 50; i++) samples[i]     = (float)(50 - i) * 12.0f;

    float fev1 = 0.0f, pef = 0.0f;
    flow_compute_results(samples, 50, &fev1, &pef);
    TEST_ASSERT_FLOAT_WITHIN(5.0f, 300.0f, pef);
}

void test_score_perfect(void)
{
    /* FEV1=2L, PEF=400 L/min, hold=10s → score=4 (A) */
    uint8_t score = flow_compute_score(2.0f, 400.0f, 10000);
    TEST_ASSERT_EQUAL_UINT8(4, score);
}

void test_score_fail(void)
{
    /* FEV1=0.1L, PEF=10 L/min, hold=2s → score=0 (F) */
    uint8_t score = flow_compute_score(0.1f, 10.0f, 2000);
    TEST_ASSERT_EQUAL_UINT8(0, score);
}

void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_flow_from_pressure_positive);
    RUN_TEST(test_flow_from_pressure_negative);
    RUN_TEST(test_flow_from_pressure_zero);
    RUN_TEST(test_fev1_integration);
    RUN_TEST(test_pef_detection);
    RUN_TEST(test_score_perfect);
    RUN_TEST(test_score_fail);
    UNITY_END();
}
