#ifndef _FUNC_H_
#define _FUNC_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"

#define PI (3.1415926f)
#define SQRT2 (1.41421356f)
#define SQRT3 (1.73205f)
#define SQRT2_DIV_SQRT3 (0.81649658f)
#define SQRT3_DIV_2     (0.8660254f)
#define FREQ_2_W        (2.0f * PI )                                //频率转角频率
#define W_2_FREQ        (1.0f / FREQ_2_W)                           //角频率转频率
#define PI_x2           (PI * 2.0f)
#define RAD_2_DEGREE    (180.0f / PI)

extern UINT16 CalcCrc(UINT8 *buf, UINT16 len);
extern UINT16 crc_modbus(UINT8 *input_str, UINT16 num_bytes );
extern UINT16 IsEqual(FLOAT32 a, FLOAT32 b, FLOAT32 gap);
//坐标变换
INLINE_USER void Abc_2_AlBe(FLOAT32 *al, FLOAT32 *be, FLOAT32 a, FLOAT32 b, FLOAT32 c)
{
    *al = SQRT2_DIV_SQRT3 * (a - 0.5f * b - 0.5f * c);
    *be = SQRT2_DIV_SQRT3 * (SQRT3_DIV_2 * b - SQRT3_DIV_2 * c);
}
INLINE_USER void AlBe_2_Abc(FLOAT32 *a, FLOAT32 *b, FLOAT32 *c, FLOAT32 al, FLOAT32 be)
{
    *a = al;
    *b = -0.5f * al + SQRT3_DIV_2 * be;
    *c = -0.5f * al - SQRT3_DIV_2 * be;
}
INLINE_USER void Albe_2_Dq(FLOAT32 *d, FLOAT32 *q, FLOAT32 al, FLOAT32 be, FLOAT32 sinx, FLOAT32 cosx)
{
    (*d) = cosx * al + sinx * be;
    (*q) = -sinx * al + cosx * be;
}
INLINE_USER void Dq_2_AlBe(FLOAT32 *al, FLOAT32 *be, FLOAT32 d, FLOAT32 q, FLOAT32 sinx, FLOAT32 cosx)
{
    (*al) = cosx * d - sinx * q;
    (*be) = sinx * d + cosx * q;
}
INLINE_USER FLOAT32 Filter(FLOAT32 input, FLOAT32 k, FLOAT32 preOut)
{
    FLOAT32 ret;
    ret = preOut * (1.0f - k) + input * k;
    return ret;
}


//限幅
#define UpDownLimit(in, max, min) \
do\
{\
    if((in) > (max))   \
    {\
        (in) = (max);\
    }\
    else if((in) < (min))\
    {\
        (in) = (min);\
    }\
} while (0);

#define MAX2(a, b)  (((a) >= (b))? (a) : (b))
#define MIN2(a, b)  (((a) <= (b))? (a) : (b))

#ifdef __cplusplus
}
#endif
#endif

