#include <stdint.h>
#include <q14.h>

#define WT_ROWS 8
#define WT_COLUMNS 8
#define WT_SAMPLES 256

extern const int16_t wavetable[WT_ROWS][WT_COLUMNS][WT_SAMPLES];

q14_t wt_lookup(int x, int y, int z);
q14_t wt_interpolate(int x, int y, q14_t theta);
