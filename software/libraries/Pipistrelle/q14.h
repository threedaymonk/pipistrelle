#ifndef __q14_h__
#define __q14_h__

typedef int32_t q14_t;

#define Q14_1    0x4000
#define Q14_1_4  0x1000
#define Q14_1_2  0x2000
#define Q14_3_4  0x3000

#define q14_blend(amt, a, b) (((Q14_1 - (amt)) * (a) + (amt) * (b)) / Q14_1)

q14_t q14_sine(q14_t x);
q14_t q14_triangle(q14_t x);
q14_t q14_square(q14_t x);
q14_t q14_quarter_square(q14_t x);
q14_t q14_saw(q14_t x);
q14_t ftoq14(float f);
float q14tof(q14_t q);

#endif /* __q14_h__ */
