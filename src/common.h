#ifndef AGC_COMMON_H
#define AGC_COMMON_H

#define agc_max(a, b) ((a) > (b) ? (a) : (b))
#define agc_min(a, b) ((a) < (b) ? (a) : (b))

#define agc_countof(arr) (sizeof(arr) / sizeof((arr)[0]))

#define agc_concat2(a, b) a##b
#define agc_concat3(a, b, c) a##b##c
#define agc_paste2(a, b) agc_concat2(a, b)
#define agc_paste3(a, b, c) agc_concat3(a, b, c)

#endif // !AGC_COMMON_H
