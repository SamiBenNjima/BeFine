#pragma once
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

void      peak_detector_reset(void);
void      peak_detector_push(float flow_lpm);
float     peak_detector_get_peak(void);
float    *peak_detector_get_buf(void);
uint32_t  peak_detector_get_count(void);

#ifdef __cplusplus
}
#endif
