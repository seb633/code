#ifndef _RATED_H_
#define _RATED_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"


class rated
{
private:

public:
    FLOAT32 ratePower;              //额定功率
    FLOAT32 rateVolt;               //额定电压
    FLOAT32 rateCurr;               //额定电流


//系数
    //AD到定标值的增益
    FLOAT32 gainChargeCurr;
    FLOAT32 gainDcInputCurr;
    FLOAT32 gainDcInputVolt;
	FLOAT32 gainDcInput2Curr;
    FLOAT32 gainDcInput2Volt;
    FLOAT32 gainBattVolt;

    rated(/* args */);

    void renewParam(FLOAT32 pset, FLOAT32 vset);
    void renewFreq(UINT16 arg);
};





#ifdef __cplusplus
}
#endif
#endif
