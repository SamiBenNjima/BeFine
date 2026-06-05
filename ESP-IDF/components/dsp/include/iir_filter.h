#pragma once
#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float b0, b1, b2;   /* numerator coefficients                   */
    float a1, a2;        /* denominator (negated — DF2T convention)  */
    float z1, z2;        /* delay elements (filter state)            */
} iir_biquad_t;

void  iir_biquad_init   (iir_biquad_t *f);
void  iir_biquad_reset  (iir_biquad_t *f);
float iir_biquad_process(iir_biquad_t *f, float x);

#ifdef __cplusplus
}
#endif
