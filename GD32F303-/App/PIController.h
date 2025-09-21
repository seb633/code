#ifndef _PI_CONTROLLER_H_
#define _PI_CONTROLLER_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "func.h"
class PIController
{
public:
	FLOAT32 err;
    FLOAT32 kp;
    FLOAT32 ki;
    FLOAT32 integ;              //积分
    FLOAT32 out;                //输出
    FLOAT32 integMax;           //上限
    FLOAT32 integMin;           //下限
	FLOAT32 rcFilter;

    PIController();
    void SetPara(FLOAT32 kpIn, FLOAT32 kiIn, FLOAT32 max, FLOAT32 min,FLOAT32 rcFilterIn = 1.0f);
    void SetMax(FLOAT32 max);
    void SetMin(FLOAT32 min);
    void SetInteg(FLOAT32 Integ);
    void SetKi(FLOAT32 kiIn);
    FLOAT32 GetInteg();
    INLINE_USER FLOAT32 run(FLOAT32 errin);
    INLINE_USER FLOAT32 runLmt(FLOAT32 err, FLOAT32 lmt);
    void rst();
};

INLINE_USER FLOAT32 PIController::run(FLOAT32 errin)
{
	err = rcFilter*errin + (1.0f - rcFilter) * err;
    integ += err * ki;
    UpDownLimit(integ, integMax, integMin);
    out = integ + kp * err;
    UpDownLimit(out, integMax, integMin);
    return out;
}

INLINE_USER FLOAT32 PIController::runLmt(FLOAT32 err, FLOAT32 lmt)
{
    FLOAT32 tempOut, temp, pout;

    integ += err * ki;                              //积分
    UpDownLimit(integ, integMax, integMin);         
    pout = kp * err;                                //P输出

    tempOut = integ + pout;                         //PI输出

    temp = tempOut - out;                           //PI增量
    UpDownLimit(temp, lmt, -lmt);                   //限幅
    out += temp;                                    //PI输出
    integ = out - pout;                             //积分
    UpDownLimit(out, integMax, integMin);
    return out;
}
INLINE_USER void PIController::rst()
{
    integ = 0;
    out = 0;
}
#ifdef __cplusplus
}
#endif
#endif
