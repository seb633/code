#ifndef _DATA_CALC_H_
#define _DATA_CALC_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"

class dataCalc
{
private:
    //累加器
    
    FLOAT32 dcInCurrAcc;
    FLOAT32 dcInVoltAcc;

    FLOAT32 chargeCurrAcc;
    FLOAT32 battVoltAcc;
    
    FLOAT32 mainCurrAcc;
    FLOAT32 mainVoltAcc;

    FLOAT32 dcInPowerAcc;

    FLOAT32 wAcc;
    UINT16 cnt;                     //数据个数

    //useless
    FLOAT32 dcInCurr2Acc;
    FLOAT32 dcInVolt2Acc;
    FLOAT32 dcInPower2Acc;
    FLOAT32 invPowerAcc;
    FLOAT32 invCurrAvgAcc;
   

    //和
    FLOAT32 dcInCurrSum;
    FLOAT32 dcInVoltSum;
   
    FLOAT32 chargeCurrSum;
    FLOAT32 battVoltSum;

    FLOAT32 mainCurrSum;
    FLOAT32 mainVoltSum;

    FLOAT32 dcInPowerSum;
	
    FLOAT32 wSum;
    UINT16 cntSum;      //数据个数

    //useless
	FLOAT32 chargeCurr2Sum;
    FLOAT32 dcInCurr2Sum;
    FLOAT32 dcInVolt2Sum;
    FLOAT32 battVolt2Sum;
    FLOAT32 dcInPower2Sum;


    //平均值、有效值
    FLOAT32 dcInCurrAvg;
    FLOAT32 dcInVoltAvg;
    
    FLOAT32 chargeCurrAvg;
    FLOAT32 battVoltAvg;

    FLOAT32 mainCurrAvg;
    FLOAT32 mainVoltAvg;

    FLOAT32 dcInPower;

    FLOAT32 wAvg;
    
    //useless
    FLOAT32 dcInCurr2Avg;
    FLOAT32 dcInVolt2Avg;
    FLOAT32 dcIn2Power;



    //滤波值
    FLOAT32 temp1Filt;
    FLOAT32 temp2Filt;
    FLOAT32 dcInVoltFilter;
    FLOAT32 dcInCurrFilter;
    FLOAT32 dcInPowerFilter;
    FLOAT32 battCurrFilt;
public:

    
    dataCalc(/* args */);
    void Add();
    void Save();
    void Calc();

    INLINE_USER FLOAT32 GetDcInCurrAvg() { return dcInCurrAvg; }; //0.8A偏置
    INLINE_USER FLOAT32 GetDcInVoltAvg() { return dcInVoltAvg; };

    INLINE_USER FLOAT32 GetChargeCurrAvg(){return chargeCurrAvg;};
    INLINE_USER FLOAT32 GetBattVoltAvg() { return battVoltAvg; };
    
    INLINE_USER FLOAT32 GetMainCurrAvg() { return mainCurrAvg; };
    INLINE_USER FLOAT32 GetMainVoltAvg() { return mainVoltAvg; };

    INLINE_USER FLOAT32 GetDcInPower() { return dcInPower; };

    //useless
	INLINE_USER FLOAT32 GetDcInCurr2Avg(){return dcInCurr2Avg;};  
    INLINE_USER FLOAT32 GetDcInVolt2Avg(){return dcInVolt2Avg;};
	INLINE_USER FLOAT32 GetDcIn2Power(){return dcIn2Power;};



    
    INLINE_USER FLOAT32 GetTemp1Filt() {return temp1Filt;};
    INLINE_USER FLOAT32 GetTemp2Filt() {return temp2Filt;};
    INLINE_USER FLOAT32 GetDcInVoltFilt() {return dcInVoltFilter;};
    INLINE_USER FLOAT32 GetDcInCurrFilt() {return dcInCurrFilter;};
    INLINE_USER FLOAT32 GetDcInPowerFilt() {return dcInPowerFilter;};

    INLINE_USER FLOAT32 GetBattCurrFilt() {return battCurrFilt;};
    

};





#ifdef __cplusplus
}
#endif
#endif
