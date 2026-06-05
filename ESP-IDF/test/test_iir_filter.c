/**
 * @file  test_iir_filter.c
 * @brief Unity tests for the IIR low-pass filter.
 *
 * Tests:
 *  1. DC pass-through (0 Hz signal must pass unattenuated)
 *  2. 5 Hz signal must pass (below fc=10 Hz cutoff)
 *  3. 20 Hz signal must be attenuated > 12 dB
 *  4. Filter state reset works correctly
 */

#include "unity.h"
#include "iir_filter.h"
#include <math.h>

#define FS          50.0f
#define PI          3.14159265f
#define N_SAMPLES   200

/* ── Helpers ─────────────────────────────────────────────────────────────── */

/** Compute RMS of a float array */
static float rms(const float *buf, int n)
{
    float sum = 0.0f;
    for (int i = 0; i < n; i++) sum += buf[i] * buf[i];
    return sqrtf(sum / (float)n);
}

/** Run a sine through the filter and return output RMS (after settling) */
static float filter_rms_at_freq(float freq_hz)
{
    iir_biquad_t f;
    iir_biquad_init(&f);

    float out[N_SAMPLES];
    for (int i = 0; i < N_SAMPLES; i++) {
        float x = sinf(2.0f * PI * freq_hz * (float)i / FS);
        out[i] = iir_biquad_process(&f, x);
    }
    /* Use last 100 samples (settled) */
    return rms(&out[100], 100);
}

/* ── Tests ───────────────────────────────────────────────────────────────── */

void test_dc_passthrough(void)
{
    iir_biquad_t f;
    iir_biquad_init(&f);
    /* Feed 1.0 DC for 50 samples to let filter settle */
    float y = 0.0f;
    for (int i = 0; i < 50; i++) y = iir_biquad_process(&f, 1.0f);
    /* DC gain of LP filter should be 1.0 */
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 1.0f, y);
}

void test_5hz_passes(void)
{
    float rms_in  = 0.707f;   /* RMS of a unit sine */
    float rms_out = filter_rms_at_freq(5.0f);
    /* 5 Hz is well below cutoff — expect > 80% of input amplitude */
    TEST_ASSERT_GREATER_THAN(rms_in * 0.80f, rms_out);
}

void test_20hz_attenuated(void)
{
    float rms_5hz  = filter_rms_at_freq(5.0f);
    float rms_20hz = filter_rms_at_freq(20.0f);
    /* 20 Hz (above fc=10 Hz) must be at least 4× weaker than 5 Hz */
    TEST_ASSERT_GREATER_THAN(rms_20hz * 4.0f, rms_5hz);
}

void test_filter_reset(void)
{
    iir_biquad_t f;
    iir_biquad_init(&f);
    /* Load state */
    for (int i = 0; i < 20; i++) iir_biquad_process(&f, 100.0f);
    iir_biquad_reset(&f);
    /* After reset, first output of zero input should be zero */
    float y = iir_biquad_process(&f, 0.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, y);
}

/* ── Unity main ─────────────────────────────────────────────────────────── */
void app_main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_dc_passthrough);
    RUN_TEST(test_5hz_passes);
    RUN_TEST(test_20hz_attenuated);
    RUN_TEST(test_filter_reset);
    UNITY_END();
}
