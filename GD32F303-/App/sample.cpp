#include "sample.h"
#include "config.h"
#include "variable.h"
#include "arch.h"

#define ADC_DEBUG
#ifdef ADC_DEBUG
volatile INT16 g_pvVolt, g_pvCurr,  g_pvVolt2, g_pvCurr2,g_battVolt, g_battCurr, g_pvPower,g_pvPower2, g_mainCurr, g_mainVolt;
#endif

#define AD_MAX_1P65 (2200)
#define AD_MIN_1P65 (1900)
sample::sample(/* args */)
{
    calibrateFlag = 0;
    cnt = 0;
    
    //温度没有校准
    temperature1.gain = 1;
    temperature2.gain = 1;

    invCurrOffsetSum = 0;
    chargeCurrOffsetSum = 0;
    invVoltOffsetSum = 0;
    adErr = 0;
    invCurrOffset = 2048;
    chargeCurrOffset = 2048;
    invVoltOffset = 2048;
}

/********************************
 * 直流偏置校准
 * **********************************/
void sample::OffsetCalibrate()
{
    if(calibrateFlag == 1)              //是否完成校准
    {
        return;                         //完成校准，不再执行
    }

    cnt++;
   
    // chargeCurrOffsetSum += chargeCurr.raw;

    if(cnt >= 16384)
    {
        invCurrOffset = invCurrOffsetSum >> 14;
        invVoltOffset = invVoltOffsetSum >> 14;
//        chargeCurrOffset = chargeCurrOffsetSum >> 14;

        if((invCurrOffset > AD_MAX_1P65) || (invCurrOffset < AD_MIN_1P65))
        {
            adErr = 1;
            invCurrOffset = 2048;
        }
        if((invVoltOffset > AD_MAX_1P65) || (invVoltOffset < AD_MIN_1P65))
        {
            adErr = 1;
            invVoltOffset = 2048;
        }
        // if((chargeCurrOffset > AD_MAX_1P65) || (chargeCurrOffset < AD_MIN_1P65))
        // {
        //     adErr = 1;
        // }

        calibrateFlag = 1;                          //校准完成
    }
}
/********************************
 * 原始数据 转 定标值
 * **********************************/

void sample::DataDeal()
{
    static FLOAT32 an = 1;
    an += PI_x2 * 50.0f * ONE_DIV_PWM_FREQ;
    if(an > PI_x2)
    {
        an -= PI_x2;
    }
#if 1
    //useless
	dcInput2Curr.data = dcInput2Curr.raw * dcInput2Curr.gain;
	dcInput2Volt.data = dcInput2Volt.raw * dcInput2Volt.gain;
    
    //MT600
    dcInputCurr.data = dcInputCurr.raw * dcInputCurr.gain;
    dcInputVolt.data = dcInputVolt.raw * dcInputVolt.gain;

    chargeCurr.data = chargeCurr.raw * chargeCurr.gain;
    battVolt.data = battVolt.raw * battVolt.gain;

    mainCurr.data = mainCurr.raw * mainCurr.gain;
    mainVolt.data = mainVolt.raw * mainVolt.gain;

    temperature1.data = temperature1.raw * temperature1.gain;
    temperature2.data = temperature2.raw * temperature2.gain;

#else

    invCurr.data = 0;
    invVolt.data = SQRT2 * 0.0f * ARCH_Sin(an);
    acInputVolt.data = SQRT2 * 0.05f * ARCH_Sin(an);
    chargeCurr.data = 0;
    temperature1.data = 0;
    dcInputCurr.data = 0;
    dcInputVolt.data = 0.0f * objRated.voltReciprocal;
    busVolt.data = 320.0f * objRated.voltReciprocal;
    battVolt.data = 23.0f * objRated.voltReciprocal;
    temperature2.data = 0;
#endif

#ifdef ADC_DEBUG
    
    //MT600
    g_pvCurr = (INT16)(objSample.GetDcInputCurr() * 10.0f);
    g_pvVolt = (INT16)(objSample.GetDcInputVolt() * 10.0f);

    g_battCurr = (INT16)(objSample.GetChargeCurr() * 10.0f);
    g_battVolt = (INT16)(objSample.GetBattVolt() * 10.0f);

    g_mainCurr = (INT16)(objSample.GetMainCurr() * 10.0f);
    g_mainVolt = (INT16)(objSample.GetMainVolt() * 10.0f);

    g_pvPower = (INT16)(objDataCalc.GetDcInPower() * 10.0f);
    
    //useless
    g_pvVolt2 = (INT16)(objSample.GetDcInput2Volt() * 10.0f);
    g_pvCurr2 = (INT16)(objSample.GetDcInput2Curr() * 10.0f);
    g_pvPower2 = (INT16)(objDataCalc.GetDcIn2Power() * 10.0f);

#endif


}
void sample::resetCalibrate()
{
    calibrateFlag = 0;
    cnt = 0;
    invCurrOffsetSum = 0;
    chargeCurrOffsetSum = 0;
    invVoltOffsetSum = 0;
}
void sample::CaliDataLmt(UINT16 *arg)
{
    if((*arg < CALIBRATE_MIN) || (*arg > CALIBRATE_MAX))
    {
        *arg = CALIBRATE_CONSTANT;
    }
}

void sample::CalibrateInDcCurr(UINT16 arg)
{
    CaliDataLmt(&arg);
    dcInputCurr.gain = SAMPLE_COFF * DC_IN_CURR_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;

}
void sample::CalibrateInDcVolt(UINT16 arg)
{
    CaliDataLmt(&arg);
    dcInputVolt.gain = SAMPLE_COFF * DC_IN_VOLT_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;
    
}

void sample::CalibrateBattCurr(UINT16 arg)
{
    CaliDataLmt(&arg);
    chargeCurr.gain = SAMPLE_COFF * CHARGE_CURR_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;

}
void sample::CalibrateBattVolt(UINT16 arg)
{
    CaliDataLmt(&arg);
    battVolt.gain = SAMPLE_COFF * BATT_VOLT_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;

}

void sample::CalibrateMainCurr(UINT16 arg)
{
    CaliDataLmt(&arg);
    mainCurr.gain = SAMPLE_COFF * MAIN_CURR_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;
}


void sample::CalibrateMainVolt(UINT16 arg)
{
    CaliDataLmt(&arg);
    mainVolt.gain = SAMPLE_COFF * MAIN_VOLT_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;
}

//useless
void sample::CalibrateInDc2Volt(UINT16 arg)
{
    CaliDataLmt(&arg);
    dcInput2Volt.gain = SAMPLE_COFF * DC_IN_VOLT_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;
    
}
void sample::CalibrateInDc2Curr(UINT16 arg)
{
    CaliDataLmt(&arg);
    dcInput2Curr.gain = SAMPLE_COFF * DC_IN_CURR_GAIN_RECIPROCAL * SAMPLE_COFF2 * (FLOAT32)arg;
    
}