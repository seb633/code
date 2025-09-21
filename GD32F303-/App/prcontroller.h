#ifndef _PR_CONTROLLER_H_
#define _PR_CONTROLLER_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"
class prcontroller
{
private:
    FLOAT32 b[3];                   //分母系数
    FLOAT32 a[3];                   //分子系数

    FLOAT32 out[3];                 //输出
    FLOAT32 in[3];                  //输入

public:
    prcontroller(/* args */);
    void CalcCoff(FLOAT32 kr, FLOAT32 w0, FLOAT32 fs, FLOAT32 wc);
    void CalcCoff2(FLOAT32 kr, FLOAT32 w0, FLOAT32 fs);
    FLOAT32 run(FLOAT32 err);


    INLINE_USER FLOAT32 GetOutput();
    INLINE_USER void rst();
};

INLINE_USER void prcontroller::rst()
{
    in[0] = 0;
    in[1] = 0;
    in[2] = 0;
    out[0] = 0;
    out[1] = 0;
    out[2] = 0;
}
INLINE_USER FLOAT32 prcontroller::GetOutput() 
{
    return out[0];
}



#ifdef __cplusplus
}
#endif
#endif
