#include "standard.h"
#include "variable.h"
#include "worklogic.h"
#include "config.h"

extern worklogic objWorkLogic;


standard::standard(/* args */)
{
}

/********************************
 * 切换国家，重新初始化
 * ******************************/
void standard::Init()
{
     
}


/********************************
 * 电压频率范围 慢检
 * ******************************/
void standard::SlowChk()
{
    faultChkRetType para;


    para.cntMax = 30;
    para.dec = 1;
    para.inc = 10;
    //电池高压
    para.point = CHARGE_VOLT_MAX;
    para.retPoint =BATT_HIGH_POINT;
    objWorkLogic.battVoltHighChk.RetChkHigh(objDataCalc.GetBattVoltAvg(), &para);
	
    para.cntMax = 300;
    para.dec = 1;
    para.inc = 10;
    
    //PV高压
    para.point = DC_IN_VOLT_HIGH_POINT;
    para.retPoint = DC_IN_VOLT_HIGH_RET_POINT ;
    objWorkLogic.pvHighChk.RetChkHigh(objDataCalc.GetDcInVoltAvg(), &para);
	//objWorkLogic.pv2HighChk.RetChkHigh(objDataCalc.GetDcInVolt2Avg(), &para);
	
    //PV低压
    para.point = DC_IN_VOLT_LOW_POINT ;
    para.retPoint = DC_IN_VOLT_LOW_RET_POINT ;
    objWorkLogic.pvLowChk.RetChkLow(objDataCalc.GetDcInVoltAvg(), &para);
    //objWorkLogic.pv2LowChk.RetChkLow(objDataCalc.GetDcInVolt2Avg(), &para);
	
    //PV掉电
    para.point = 5.0f ;
    para.retPoint = 11.0f;
    para.cntMax = 3000;
    para.dec = 100;
    para.inc = 30;
    objWorkLogic.pvVoltLossChk.RetChkLow(objDataCalc.GetDcInVoltAvg(), &para);
	objWorkLogic.pv2VoltLossChk.RetChkLow(objDataCalc.GetDcInVolt2Avg(), &para);

	//PV电流过流
    para.cntMax = 100;
    para.dec = 1;
    para.inc = 3;
    para.point = (PV_MAX_CURR_1 +2.0f);
    para.retPoint = PV_MAX_CURR_1 ;
    objWorkLogic.pvOvCurrChk.RetChkHigh(ARCH_Abs(objDataCalc.GetDcInCurrAvg()), &para);
	//objWorkLogic.pv2OvCurrChk.RetChkHigh(ARCH_Abs(objDataCalc.GetDcInCurr2Avg()), &para);
	
    //电池电流过流
    para.cntMax = 100;
    para.dec = 1;
    para.inc = 3;
    para.point = BATT_CURR_OV_POINT ;
    para.retPoint = BATT_CURR_OV_RET_POINT ;
    objWorkLogic.battOvCurrChk.RetChkHigh(ARCH_Abs(objDataCalc.GetChargeCurrAvg()), &para);

    //电池非常低压
    para.cntMax = 30;
    para.dec = 1;
    para.inc = 3;
    para.point = CHARGE_VOLT_MIN ;
    para.retPoint = BATT_LOW_POINT ;
    objWorkLogic.battVoltVeryLowChk.RetChkLow(objDataCalc.GetBattVoltAvg(), &para);
    
}

/********************************
 * 快检
 * ******************************/
void standard::FastChk()
{
    faultChkDigitalType para;
    UINT16 ov;
    
    para.cntMax = 5;
    para.dec = 1;
    para.inc = 1;

	//电池过流
	ov = ARCH_GetChargeCurrOv();
	objWorkLogic.battOvCurrChk.DigitalChkState(ov,&para);
}

