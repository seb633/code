#include "dataCalc.h"
#include "sample.h"
#include "variable.h"
#include "arch.h"

dataCalc::dataCalc(/* args */)
{
    dcInCurrAcc = 0.0f;
    dcInVoltAcc = 0.0f;
    
    chargeCurrAcc = 0.0f;
    battVoltAcc = 0.0f;
    
    mainCurrAcc = 0.0f;
    mainVoltAcc = 0.0f;

    dcInPowerAcc = 0.0f;
    
    cnt = 0;
    //useless
    dcInCurr2Acc = 0.0f;
    dcInVolt2Acc = 0.0f;
    dcInPower2Acc = 0.0f;
    
   
    temp1Filt = 0.0f;
    temp2Filt = 0.0f;
    dcInVoltFilter = 0.0f;
    dcInPowerFilter = 0.0f;
    battCurrFilt = 0.0f;
    dcInCurrFilter = 0.0f;
}

/********************************
 * 数据累加，算平均值，有效值，功率
 * **********************************/
void dataCalc::Add()
{
    dcInCurrAcc += objSample.GetDcInputCurr();
    dcInVoltAcc += objSample.GetDcInputVolt();

    chargeCurrAcc += objSample.GetChargeCurr();
    battVoltAcc += objSample.GetBattVolt();
    
    mainCurrAcc += objSample.GetMainCurr();
    mainVoltAcc += objSample.GetMainVolt();

    dcInPowerAcc += objSample.GetDcInputVolt() * objSample.GetDcInputCurr();
	
    //useless
    dcInCurr2Acc += objSample.GetDcInput2Curr();
    dcInVolt2Acc += objSample.GetDcInput2Volt();
	dcInPower2Acc += objSample.GetDcInput2Volt() * objSample.GetDcInput2Curr();
    
    cnt++;

    
    temp1Filt = Filter(objSample.GetTemperature1(), 0.01f, temp1Filt);
    temp2Filt = Filter(objSample.GetTemperature2(), 0.01f, temp2Filt);
    battCurrFilt = Filter(objSample.GetChargeCurr(), 0.01f, battCurrFilt);

}

/********************************
 * 一个周期完成，保存数据，在任务中计算
 * **********************************/
void dataCalc::Save()
{
    dcInCurrSum = dcInCurrAcc;
    dcInVoltSum = dcInVoltAcc;

    chargeCurrSum = chargeCurrAcc;
    battVoltSum = battVoltAcc;
    
    mainCurrSum = mainCurrAcc;
    mainVoltSum = mainVoltAcc;

    dcInPowerSum = dcInPowerAcc;

    //useless
    dcInCurr2Sum = dcInCurr2Acc;
    dcInVolt2Sum = dcInVolt2Acc;
    dcInPower2Sum = dcInPower2Acc;
    
    wSum = wAcc;
    cntSum = cnt;

    dcInCurrAcc = 0.0f;
    dcInVoltAcc = 0.0f;

    chargeCurrAcc = 0.0f;
    battVoltAcc = 0.0f;
    
    mainCurrAcc = 0.0f;
    mainVoltAcc = 0.0f;

    dcInPowerAcc = 0.0f;

    //useless
    dcInCurr2Acc = 0.0f;
    dcInVolt2Acc = 0.0f;
    dcInPower2Acc = 0.0f;
    

    invPowerAcc = 0.0f;

    wAcc = 0.0f;
    invCurrAvgAcc = 0.0f;
   
    cnt = 0;
}

/********************************
 * 计算数据
 * **********************************/
void dataCalc::Calc()
{
    FLOAT32 div = ARCH_Div(1.0, cntSum);        //   1/cntSum

    dcInCurrAvg = dcInCurrSum * div;
    dcInVoltAvg = dcInVoltSum * div;
    
    chargeCurrAvg = chargeCurrSum * div;
    battVoltAvg = battVoltSum * div;
	
    mainCurrAvg = mainCurrSum * div;
    mainVoltAvg = mainVoltSum * div;

    dcInPower = dcInPowerSum * div;
	
    //useless
    dcInCurr2Avg = dcInCurr2Sum * div;
    dcInVolt2Avg = dcInVolt2Sum * div;
    dcIn2Power = dcInPower2Sum * div;
	
    wAvg = wSum * div;


    dcInPowerFilter = Filter(dcInPower, 0.05f, dcInPowerFilter);
    dcInVoltFilter = Filter(dcInVoltAvg, 0.05f, dcInVoltFilter);
    dcInCurrFilter = Filter(dcInCurrAvg, 0.05f, dcInCurrFilter);

    
}
