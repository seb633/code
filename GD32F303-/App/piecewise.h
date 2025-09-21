#ifndef _PIECEWISE_H_
#define _PIECEWISE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"


struct line
{
//直线方程 y=ax + b，gain=a，bais=b
    FLOAT32 gain;
    FLOAT32 bais;
};


class piecewise
{
private:
    UINT16 number;              //分割点个数
    FLOAT32 *pointPtr;          //分割点数组
    line *linePtr;              //number+1 段曲线

public:
    piecewise(/* args */);
    void SetPara(UINT16 n, const FLOAT32 * pp, const line *lp);
    FLOAT32 GetValue(FLOAT32 input);
    FLOAT32 GetValueHalf(FLOAT32 input);
    UINT16 IsSet() { return ( number != 0 ); };             //曲线是否设置
};







#ifdef __cplusplus
}
#endif
#endif
