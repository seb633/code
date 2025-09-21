#ifndef _NTC_H_
#define _NTC_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"
#include "piecewise.h"
#include "arch.h"

class ntc
{
private:
    piecewise arc;              //温度曲线
public:
    ntc(/* args */);

    FLOAT32 CalcAdToOhm(FLOAT32 ad);
    FLOAT32 CalcOhmToTemp(FLOAT32 ohm);
    FLOAT32 CalcAdToTemp(FLOAT32 ad);
    void SetNtcLine(UINT16 n, const FLOAT32 *pp, const line *lp);
};





#ifdef __cplusplus
}
#endif
#endif

