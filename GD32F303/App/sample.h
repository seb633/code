#ifndef _SAMPLE_H_
#define _SAMPLE_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "usertypes.h"
struct dataElement
{
    INT16 raw;
    FLOAT32 gain;
    FLOAT32 data;
};
class sample
{
private:
    
    
    dataElement chargeCurr;
	//dataElement charge2Curr;
    dataElement temperature1;
    dataElement dcInputCurr;
	dataElement dcInput2Curr;
    dataElement dcInputVolt;
    dataElement dcInput2Volt;

    dataElement mainCurr;
    dataElement mainVolt;

    dataElement battVolt;
    dataElement temperature2;


    //零偏校准
    UINT16 cnt;                             //校准用计数器
    volatile UINT16 calibrateFlag;          //校准完成标志
    INT32 invCurrOffsetSum;
    INT16 invCurrOffset;

    INT32 chargeCurrOffsetSum;
    INT16 chargeCurrOffset;

    INT32 invVoltOffsetSum;
    INT16 invVoltOffset;

    UINT16 adErr;

    //
public:
    sample(/* args */);

    //设置原始采样数据
    
    //useless
	INLINE_USER void SetDcInput2CurrRaw(UINT16 in) {dcInput2Curr.raw = in;};    
	INLINE_USER void SetDcInput2VoltRaw(UINT16 in) {dcInput2Volt.raw = in;};   


    //设置原始采样数据 MT600
    INLINE_USER void SetDcInputCurrRaw(UINT16 in) {dcInputCurr.raw = in;};
    INLINE_USER void SetDcInputVoltRaw(UINT16 in) {dcInputVolt.raw = in;};
    
    INLINE_USER void SetChargeCurrRaw(UINT16 in) { chargeCurr.raw = in; };
    INLINE_USER void SetBattVoltRaw(UINT16 in) { battVolt.raw = in; };

    INLINE_USER void SetMainCurrRaw(UINT16 in) { mainCurr.raw = in; };
    INLINE_USER void SetMainVoltRaw(UINT16 in) { mainVolt.raw = in; };

    INLINE_USER void SetTemperarute1Raw(UINT16 in) { temperature1.raw = in; };
    INLINE_USER void SetTemperatrue2Raw(UINT16 in) { temperature2.raw = in; };
    

    //设置增益 MT600
    INLINE_USER void SetDcInputCurrGain(UINT16 in) { dcInputCurr.gain = in; };
    INLINE_USER void SetDcInputVoltGain(UINT16 in) { dcInputVolt.gain = in; };

    INLINE_USER void SetChargeCurrGain(UINT16 in) {chargeCurr.gain = in;};
    INLINE_USER void SetBattVoltGain(UINT16 in) { battVolt.gain = in; };
    
    INLINE_USER void SetMainCurrGain(UINT16 in) { mainCurr.gain = in; };
    INLINE_USER void SetMainVoltGain(UINT16 in) { mainVolt.gain = in; };

    INLINE_USER void SetTemperarute1Gain(UINT16 in) { temperature1.gain = in; };
    INLINE_USER void SetTemperatrue2Gain(UINT16 in) { temperature2.gain = in; };

    
    //useless
    INLINE_USER void SetDcInputCurr2Gain(UINT16 in) {dcInput2Curr.gain = in;};
    INLINE_USER void SetDcInputVolt2Gain(UINT16 in) {dcInput2Volt.gain = in;};




    //校准
    void OffsetCalibrate();
    INLINE_USER UINT16 IsCalibrate() {return calibrateFlag;};
    //数据处理
    void DataDeal();
    void resetCalibrate();

    //获取 MT600
    INLINE_USER FLOAT32 GetDcInputCurr() { return dcInputCurr.data; };//电流很小的时候采样不准
    INLINE_USER FLOAT32 GetDcInputVolt() { return dcInputVolt.data; };
    
    INLINE_USER FLOAT32 GetChargeCurr() { return chargeCurr.data; };
    INLINE_USER FLOAT32 GetBattVolt() { return battVolt.data; };

    INLINE_USER FLOAT32 GetMainCurr() { return mainCurr.data; };
    INLINE_USER FLOAT32 GetMainVolt() { return mainVolt.data; };

    INLINE_USER FLOAT32 GetTemperature1() { return temperature1.data; };
    INLINE_USER FLOAT32 GetTemperature2() { return temperature2.data; };
    
    //useless
    INLINE_USER FLOAT32 GetDcInput2Curr() {return dcInput2Curr.data;};
	INLINE_USER FLOAT32 GetDcInput2Volt() {return dcInput2Volt.data;};

    

    void CaliDataLmt(UINT16 *arg);
    
    void CalibrateInDcCurr(UINT16 arg);
    void CalibrateInDcVolt(UINT16 arg);

    void CalibrateBattCurr(UINT16 arg);
    void CalibrateBattVolt(UINT16 arg);

    void CalibrateMainCurr(UINT16 arg);
    void CalibrateMainVolt(UINT16 arg);

    //useless
    void CalibrateInDc2Volt(UINT16 arg);
    void CalibrateInDc2Curr(UINT16 arg);


};


#ifdef __cplusplus
}
#endif
#endif
