#include "rated.h"
#include "arch.h"
#include "config.h"
#include "worklogic.h"
#include "variable.h"

extern worklogic objWorkLogic;
 
rated::rated(/* args */)
{
}
/***************************************************************************
 * 设置额定功率，额定电压
 * pset：额定功率
 * vset：额定电压
***************************************************************************/
void rated::renewParam(FLOAT32 pset, FLOAT32 vset)
{
    ARCH_DisInterrupt();                                            //关中断

//采样
    gainChargeCurr = SAMPLE_COFF * BATT_CURR_GAIN_RECIPROCAL ;
    gainDcInputCurr = SAMPLE_COFF * DC_IN_CURR_GAIN_RECIPROCAL ;
    gainDcInputVolt = SAMPLE_COFF * DC_IN_VOLT_GAIN_RECIPROCAL ;
	gainDcInput2Curr = SAMPLE_COFF * DC_IN_CURR_GAIN_RECIPROCAL ;
    gainDcInput2Volt = SAMPLE_COFF * DC_IN_VOLT_GAIN_RECIPROCAL ;
    gainBattVolt = SAMPLE_COFF * BATT_VOLT_GAIN_RECIPROCAL ;
//逆变相关参数

    ARCH_EnInterrupt();                                             //开中断
	
	
	FLOAT32 a = 1.0f / (1.0f + 1.0f*PWM_FREQ / (4000.0f*PI_x2));

   	//环路参数表 Kp Ki Max Min
    objBuckBoost.PVvoltLoop.SetPara(	0.204f,		    1.5f/PWM_FREQ,		        	PV_MAX_CURR_1,		-0.1f,          a);
    objBuckBoost.PVcurrLoop.SetPara(	0.002222f,	    0.55555f/PWM_FREQ,			    1.82f,			    -1.82f,			a);
    objBuckBoost.UbatLoop.SetPara(      1.002222f,	    0.04444f/PWM_FREQ,              PV_MAX_CURR_1,		-PV_MAX_CURR_1,	a);

    objBuckBoost2.PVvoltLoop.SetPara(	0.204f,		    1.5f/PWM_FREQ,		        	PV_MAX_CURR_1,		-0.1f,          a);
    objBuckBoost2.PVcurrLoop.SetPara(	0.002222f,	    0.55555f/PWM_FREQ,			    1.82f,			    -1.82f,			a);
    objBuckBoost2.UbatLoop.SetPara(     1.002222f,	    0.0444f/PWM_FREQ,               PV_MAX_CURR_1,		-PV_MAX_CURR_1,	a);

}
void rated::renewFreq(UINT16 arg)
{
	
}




