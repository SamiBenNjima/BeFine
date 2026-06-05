/**
 * @file  iir_filter.c
 * @brief 2nd-order Butterworth IIR low-pass filter (Direct Form II Transposed).
 *
 * Coefficients for fc=10 Hz, fs=50 Hz:
 *   python: from scipy.signal import butter, sosfreqz
 *           b, a = butter(2, 10/(50/2))
 *   b = [0.20657208, 0.41314417, 0.20657208]
 *   a = [1.0,       -0.36952738, 0.19581571]
 *
 * The filter attenuates turbulence noise above 10 Hz while preserving
 * the physiological flow signal (respiration: 0.2 – 4 Hz).
 *
 * Thread-safety: each task instance must own its own iir_biquad_t.
 */

#include "iir_filter.h"
#include <string.h>

/* ── Pre-computed coefficients: LP Butterworth 2nd order, fc=10 Hz, fs=50 Hz */
static const iir_biquad_t LP_10HZ_50FS = {
    .b0 =  0.20657208f,
    .b1 =  0.41314417f,
    .b2 =  0.20657208f,
    .a1 = -0.36952738f,   /* stored negated — see Direct Form II Transposed */
    .a2 =  0.19581571f,
    .z1 =  0.0f,
    .z2 =  0.0f,
};

/* ── Public API ─────────────────────────────────────────────────────────── */

void iir_biquad_init(iir_biquad_t *f)
{
    *f = LP_10HZ_50FS;
}

void iir_biquad_reset(iir_biquad_t *f)
{
    f->z1 = 0.0f;
    f->z2 = 0.0f;
}

/**
 * Direct Form II Transposed biquad section.
 *   y[n] = b0*x[n] + z1[n-1]
 *   z1[n] = b1*x[n] - a1*y[n] + z2[n-1]
 *   z2[n] = b2*x[n] - a2*y[n]
 */
float iir_biquad_process(iir_biquad_t *f, float x)
{
    float y  = f->b0 * x + f->z1;
    f->z1    = f->b1 * x - f->a1 * y + f->z2;
    f->z2    = f->b2 * x - f->a2 * y;
    return y;
}
