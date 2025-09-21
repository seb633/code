#include "worklogic.h"
#include "func.h"
#include "variable.h"
#include "protocol.h"
extern protocol objProtocol;

extern UINT16 user_off;
extern UINT16 volt_curr;

#define IDLE_WAIT_TIME              (2*500)                    //空闲状态等待时间  30s
#define CURR_CHANGE_DELTA			(0.5f/500.0f)			//1.0A/s  0.5A/s？


worklogic::worklogic(/* args */)
{
    chargeOnCmd = 0;

    powerUpTime = 0;
    noFaultCnt = 0;
    faultComfirm = 1;
    waitEnd = 0;
    waitCnt = 0;

    country = COUNTRY_CHINA;
    reCalibrate = 0;
    pSd = &objDftStd;

    wakeUpBMU = WAKE_WAIT_1;
    pvState = PV_WORK_FAULT;
	
	//dcModeCurr2LmtSet = dcModeCurrLmtSet = 0;//PV_MAX_CURR_1;
    dcModeCurrLmtSet = 0;

	dcModebmsCurrLmt = 0;//PV_MAX_CURR_1;
	dcModeTempCurrLmt = PV_MAX_CURR_1;

    //dcModeCurr2LmtCnt = dcModeCurrLmtCnt = 0;
    dcModeCurrLmtCnt = 0;

}

#define FAN_LEVEL_NUM   (6)
const FLOAT32 ntcBreakPoint[FAN_LEVEL_NUM - 1] = {70.0f, 80.0f, 90.0f, 95.0f, 100.0f};
const FLOAT32 powerBreakPoint[2] = {0.2f, 0.5f};

const FLOAT32 powerPoint[] = {1.0f, 1.05f, 1.4f, 2.3f};
const line powerLine[] = {{0.0f, -12.0f}, {0.0f, 0.0f}, {0.0f, 120.0f}, {0.0f, 300.0f}, {0.0f, 1200.0f}};

#define CHARGE_POWER_LEVEL_NUM  (5)
const FLOAT32 tempCtrlPowerBreakPoint[CHARGE_POWER_LEVEL_NUM - 1] = {90.0f, 92.0f, 93.0f, 95.0f};
const FLOAT32 chargePowerLevel[CHARGE_POWER_LEVEL_NUM] = {1.0f, 1.0f, 0.84f, 0.50f, 0.25f};
/**********************************************************************
 * 上电后操作
************************************************************************/
void worklogic::init()
{
    objRated.renewParam(0,0);
    objProtocol.SetPara(&comm0Para);
    boostNtc.SetNtcLine(sizeof(g_NtcBreakPoint)/sizeof(FLOAT32), g_NtcBreakPoint, g_NtcLine);
    buckNtc.SetNtcLine(sizeof(g_NtcBreakPoint)/sizeof(FLOAT32), g_NtcBreakPoint, g_NtcLine);
    fanStateNtc.SetPara(FAN_LEVEL_NUM, ntcBreakPoint, 2.0f);
    fanStatePower.SetPara(3, powerBreakPoint, 0.05f);
    powerProtectLine.SetPara(4, powerPoint, powerLine);
    chargeLevelState.SetPara(CHARGE_POWER_LEVEL_NUM, tempCtrlPowerBreakPoint, 2.0f);
    chargePowerLmt = chargePowerLevel[0];
}

/******************************
 * 故障处理
 * ****************************/
void worklogic::FaultCollect()
{
    ARCH_DisInterrupt();                                            //关中断

	hardFault.bit.t1Over = t1OverChk.GetState();
    //hardFault.bit.t2Over = t2OverChk.GetState();
    hardFault.bit.overLoad = powerProtect.GetState();
    hardFault.bit.battOvCurr = battOvCurrChk.GetState();
    hardFault.bit.battVoltLow = battVoltVeryLowChk.GetState();
    
    ARCH_EnInterrupt();                                             //开中断


}

/*****************************************
 * 快速检测的故障，需要快速状态切换
**************************************/
void worklogic::FastOff()
{

}

//车充模式判断
bool worklogic::IsCarChargeMode(FLOAT32 Volt)
{
    if ((Volt >= CAR_VOLT_LOW1 && Volt <= CAR_VOLT_HIGH1) || (Volt >= CAR_VOLT_LOW2 && Volt <= CAR_VOLT_HIGH2))
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*要想一个识别DC源和光伏板的方法*/
void worklogic::BuckBoostStateShift()
{ 
    
    //ywq_test
   //   hardFault.bit.powerOff = 0;
    
	//错误检测
	pvFault 	= /* pvLowChk.GetState()  |*/ (pvHighChk.GetState()<<1)     | (pvOvCurrChk.GetState()<<2);
	
    //pv2Fault 	= /* pv2LowChk.GetState() |*/ (pv2HighChk.GetState()<<1)    | (pv2OvCurrChk.GetState()<<2);
	
    batFault 	= 	battVoltHighChk.GetState() | (battOvCurrChk.GetState()<<1);
	//tempFault 	= t1OverChk.GetState() | (t2OverChk.GetState()<<1);
   
    //
    //tempFault = t1OverChk.GetState();
    tempFault = t1OverChk.GetState() | (NtcFault1 << 1);

	sysFault 	=  pvFault || batFault || tempFault;
	
    //sys2Fault 	= pv2Fault || batFault || tempFault;
    
    //---------------------------------PV1+PV2---------------------------------
    /*if(pvState != PV_WORK_NORMAL || pv2State != PV_WORK_NORMAL)
    {
       objBuckBoost2.SlaveMode = objBuckBoost.SlaveMode = 0;
    }
    if((hardFault.bit.powerOff) || ((sysFault != 0)&&(sys2Fault != 0)))
    {
        dcModebmsCurrLmt = 0;
    }*/

    //单路不需要slavemode
    if (pvState != PV_WORK_NORMAL)
    {
        objBuckBoost.SlaveMode = 0;
    }
    if ((hardFault.bit.powerOff) || (sysFault != 0))
    {
        dcModebmsCurrLmt = 0;
    }

	//---------------------------------PV1---------------------------------
    
    if(hardFault.bit.powerOff)
    {
        pvState = PV_WORK_IDLE;
    }
    if( pvLowChk.GetState()) 
    {
        //PV电压低不认为是错误，只认为是空闲态
        pvState = PV_WORK_IDLE;
        pvChkCnt = 0;
    }
    if(sysFault != 0)
	{
		pvState = PV_WORK_FAULT;
        pvChkCnt = 0;
	}

	switch(pvState)
	{
		case PV_WORK_FAULT:
            pvChkCnt = 0;
			objBuckBoost.SetState(BUCK_BOOST_OFF);
			if(sysFault == 0)
			{
				pvState = PV_WORK_IDLE;
			}
			break;
		case PV_WORK_IDLE:
            pvChkCnt++;
            objBuckBoost.SetState(BUCK_BOOST_OFF);
            if(pvChkCnt > IDLE_WAIT_TIME) //2s开机延时
            {   
                 if(hardFault.bit.powerOff == 0)
                {
                    CurrSoftInc = 0.8f;
                    objBuckBoost.SlaveMode = 0; //!
                    dcModeCurrLmtSet = 0.8f;
                    dcModeCurrLmtCnt =0;
                    objMppt.rst();
                
               
                    pvChkCnt = 0;        
                    pvOpenVoltSave = objDataCalc.GetDcInVoltAvg();
                    pvState = PV_WORK_NORMAL;
                    objBuckBoost.SourceMode = DcUbatCtrl;
                }
            }
			break;
		case PV_WORK_NORMAL:
			//识别源的类型
			/*
			 if((pvOpenVoltSave - objDataCalc.GetDcInVoltAvg()) > PV_CHK_VOLT_DROP)
			 {
				pvChkCnt++;
			 }
			 else
			 {
				pvChkCnt = 0;
			 }
			 
			 if(pvChkCnt > 1000) //2s
			 {
				objBuckBoost.SourceMode = PvUpvCtrl;
			 }
		    */  
            if(objBuckBoost.SourceMode == DcUbatCtrl)
            {
                if(		(pvOpenVoltSave >= 16.0f && pvOpenVoltSave <= 23.5f) 
                    || 	(pvOpenVoltSave >= 33.0f && pvOpenVoltSave <= 80.0f))
                {
                    objBuckBoost.SourceMode = PvUpvCtrl;
                }
            }
			 
			//BMS电流设置
			objBuckBoost.currRefLmt =  dcModebmsCurrLmt;

			if(objBuckBoost.SourceMode == DcUbatCtrl) //DC MODE
			{
				//DC源防拉死限制
				if(objDataCalc.GetDcInVoltAvg() <= (pvOpenVoltSave - DC_CHK_VOLT_DROP))
				{
					dcModeCurrLmtSet  = objDataCalc.GetDcInCurrAvg() - 0.2f;
                    dcModeCurrLmtCnt = 5000;//10s
				}
                if(dcModeCurrLmtCnt > 0)
                {
                    dcModeCurrLmtCnt--;
                }
                if(dcModeCurrLmtCnt == 0)
                {
                    dcModeCurrLmtSet += CURR_CHANGE_DELTA;
                }
				
				UpDownLimit(dcModeCurrLmtSet, PV_MAX_CURR_1, 0);
				objBuckBoost.currRefLmt = MIN2(objBuckBoost.currRefLmt, dcModeCurrLmtSet);
			
				//车充充电限制			 
				UpDownLimit(objBuckBoost.currRefLmt, PV_MAX_CURR_CAR, 0);
			}

			else if(objBuckBoost.SourceMode == PvUpvCtrl || objBuckBoost.SourceMode == PvUbatCtrl ) //PV MODE
			{
                objMppt.SetUoc(pvOpenVoltSave);
                
                mpptCnt++;
                if(mpptCnt >= 50) //10Hz
                {
                    mpptCnt = 0;
                    objBuckBoost.SetVinRef(objMppt.run(objDataCalc.GetDcInVoltAvg(), objDataCalc.GetDcInPower()));
				}
				//蓄电池充电限额
				if(objDataCalc.GetBattVoltAvg() > objBuckBoost.voutMaxLmt)
				{
					objMppt.SetPowerLimit(0);
                    objBuckBoost.SourceMode = PvUbatCtrl;
					
					//PV源防拉死
					if(objDataCalc.GetDcInVoltAvg() < (DC_IN_VOLT_LOW_POINT + DC_CHK_VOLT_DROP))
					{
						dcModeCurrLmtSet = objDataCalc.GetDcInCurrAvg() - 0.2f;
					}
				}
				
                //电池电压没到限额时正常充电
                if(objDataCalc.GetBattVoltAvg() < objBuckBoost.voutMaxLmt - 2.0f)
				{
					objMppt.SetPowerLimit(PV_MAX_POWER);
                    objBuckBoost.SourceMode = PvUpvCtrl;
				}
				
				//MPPT限额，两段电压工作区间的电流限额
				if(objDataCalc.GetDcInVoltAvg() >= 16.0f && objDataCalc.GetDcInVoltAvg() <= 23.5f)
				{
					UpDownLimit(objBuckBoost.currRefLmt, PV_MAX_CURR_MPPT_1, 0);
				}					
				else if(objDataCalc.GetDcInVoltAvg() >= 33.0f && objDataCalc.GetDcInVoltAvg() <= 80.0f)
				{
					UpDownLimit(objBuckBoost.currRefLmt, PV_MAX_CURR_MPPT_2, 0);
				}
                
                //缓启动
                dcModeCurrLmtSet += CURR_CHANGE_DELTA;
                UpDownLimit(dcModeCurrLmtSet, PV_MAX_CURR_1, 0);
				objBuckBoost.currRefLmt = MIN2(objBuckBoost.currRefLmt,dcModeCurrLmtSet);
				
			}
			
			//过温限制
			if((chargePowerLmt * PV_MAX_POWER) < objDataCalc.GetDcInPower())
			{
				dcModeTempCurrLmt -= CURR_CHANGE_DELTA;
			}
			else
			{
				dcModeTempCurrLmt += CURR_CHANGE_DELTA;
			}
			UpDownLimit(dcModeTempCurrLmt, PV_MAX_CURR_1, 0);
			objBuckBoost.currRefLmt = MIN2(objBuckBoost.currRefLmt, dcModeTempCurrLmt);
            
            //软启动电流限制
            CurrSoftInc += CURR_CHANGE_DELTA;
            UpDownLimit(CurrSoftInc, objDataCalc.GetDcInCurrAvg() + 1.0f, 0);
            objBuckBoost.currRefLmt = MIN2(objBuckBoost.currRefLmt, CurrSoftInc);
			
			/*
            //防止一路DC源接两路PV
			if(objBuckBoost.SlaveMode == 0 && objBuckBoost2.SlaveMode == 0)
			{
				if( 	(IsEqual(pvOpenVoltSave,pvOpenVolt2Save,0.5f) == 1)
					&& 	(IsEqual(objDataCalc.GetDcInCurrAvg(),objDataCalc.GetDcInCurr2Avg(),1.0f) == 1)
					&&  (IsEqual(objBuckBoost.GetDutySet(),objBuckBoost2.GetDutySet(),0.5f) == 0)
					&&  (objBuckBoost2.GetDutySet() > 0)
				)
				{
					pvChkCnt++;
					if(pvChkCnt > 100) //2s
					{	
						pvChkCnt = 0;
						objBuckBoost.SlaveMode = 1;
					}
							
				}
				else
				{	
					pvChkCnt = 0;
				}
			}
			else
			{
				pvChkCnt = 0;
				if(IsEqual(objDataCalc.GetDcInVoltAvg(),objDataCalc.GetDcInVolt2Avg(),2.0f) != 1)
				{
					objBuckBoost.SlaveMode = 0;
					objBuckBoost.SlaveDutySet = 0;
				}
			}
			
			if(objBuckBoost.SlaveMode == 1)
			{
                if(objBuckBoost.SlaveDutySet < (objBuckBoost2.GetDutySet()-0.01f))
                {   
                    objBuckBoost.SlaveDutySet += CURR_CHANGE_DELTA;
                }
                if(objBuckBoost.SlaveDutySet > (objBuckBoost2.GetDutySet()+0.01f))
                {   
                    objBuckBoost.SlaveDutySet -= CURR_CHANGE_DELTA;
                }
				UpDownLimit(objBuckBoost.SlaveDutySet,objBuckBoost2.GetDutySet()+0.3f,objBuckBoost2.GetDutySet()-0.3f);
			}
            */
			
			//设置输出电压
			UpDownLimit(objBuckBoost.voutMaxLmt, BATT_HIGH_POINT, 0);
			objBuckBoost.SetVoRef(objBuckBoost.voutMaxLmt);
			objBuckBoost.SetState(BUCK_BOOST_ON);
			
		break;

		default:
			pvState = PV_WORK_IDLE;
			objBuckBoost.SetState(BUCK_BOOST_OFF);
			break;
	}

    /*
	//---------------------------------PV2---------------------------------
    if(hardFault.bit.powerOff )
    {
        pv2State = PV_WORK_IDLE;
    }
    if( pv2LowChk.GetState() )
    {
        pv2State = PV_WORK_IDLE;
        pv2ChkCnt = 0;
    }
    if(sys2Fault != 0)
	{
		pv2State = PV_WORK_FAULT;
        pv2ChkCnt = 0;
	}

	switch(pv2State)
	{
		case PV_WORK_FAULT:
            pv2ChkCnt = 0;
			objBuckBoost2.SetState(BUCK_BOOST_OFF);
			if(sys2Fault == 0)
			{
				pv2State = PV_WORK_IDLE;
			}
			break;
		case PV_WORK_IDLE:
            objBuckBoost2.SetState(BUCK_BOOST_OFF);
            pv2ChkCnt++;
            if(pv2ChkCnt > IDLE_WAIT_TIME) //2s开机延时
            {   
                
                if(hardFault.bit.powerOff == 0)
                {
                    Curr2SoftInc = 0.8f;
                    objBuckBoost2.SlaveMode = 0;
                    dcModeCurr2LmtSet = 0.8f;
                    dcModeCurr2LmtCnt =0;
                    objMppt2.rst();
                    
                    pv2ChkCnt = 0;    
                    pvOpenVolt2Save = objDataCalc.GetDcInVolt2Avg();
                    pv2State = PV_WORK_NORMAL;
                    objBuckBoost2.SourceMode = DcUbatCtrl;
                }
            }
			break;
		case PV_WORK_NORMAL:

            if(objBuckBoost2.SourceMode == DcUbatCtrl)
            {
                if(		(pvOpenVolt2Save >= 16.0f && pvOpenVolt2Save <= 23.5f) 
				|| 	(pvOpenVolt2Save >= 33.0f && pvOpenVolt2Save <= 80.0f))
                {
                    objBuckBoost2.SourceMode = PvUpvCtrl;
                }
            
            }
			
			 
			//BMS电流设置
			objBuckBoost2.currRefLmt =  dcModebmsCurrLmt;

			if(objBuckBoost2.SourceMode == DcUbatCtrl) //DC MODE
			{
				//DC源防拉死限制
				if(objDataCalc.GetDcInVolt2Avg() <= (pvOpenVolt2Save - DC_CHK_VOLT_DROP))
				{
					dcModeCurr2LmtSet  = objDataCalc.GetDcInCurr2Avg() - 0.2f;
                    dcModeCurr2LmtCnt = 5000;//10s
				}
                if(dcModeCurr2LmtCnt > 0)
                {
                    dcModeCurr2LmtCnt--;
                }
                if(dcModeCurr2LmtCnt == 0)
                {
                    dcModeCurr2LmtSet += CURR_CHANGE_DELTA;
                }
                
                
				
				UpDownLimit(dcModeCurr2LmtSet, PV_MAX_CURR_1, 0);
				objBuckBoost2.currRefLmt = MIN2(objBuckBoost2.currRefLmt,dcModeCurr2LmtSet);
			
				//车充充电限制			 
				UpDownLimit(objBuckBoost2.currRefLmt,PV_MAX_CURR_CAR,0);
			}
			else if(objBuckBoost2.SourceMode == PvUpvCtrl || objBuckBoost2.SourceMode == PvUbatCtrl ) //PV MODE
			{
                objMppt2.SetUoc(pvOpenVolt2Save);
                
                mppt2Cnt++;
                if(mppt2Cnt >= 50) //10Hz
                {
                    mppt2Cnt = 0;
                    objBuckBoost2.SetVinRef(objMppt2.run(objDataCalc.GetDcInVolt2Avg(), objDataCalc.GetDcIn2Power()));
				}
				//蓄电池充电限额
				if(objDataCalc.GetBattVoltAvg() > objBuckBoost2.voutMaxLmt)
				{
					objMppt2.SetPowerLimit(0);
                    objBuckBoost2.SourceMode = PvUbatCtrl;
					
					//PV源防拉死
					if(objDataCalc.GetDcInVolt2Avg() < (DC_IN_VOLT_LOW_POINT + DC_CHK_VOLT_DROP))
					{
						dcModeCurr2LmtSet = objDataCalc.GetDcInCurr2Avg() - 0.2f;
					}
				}
				
                if(objDataCalc.GetBattVoltAvg() < objBuckBoost2.voutMaxLmt - 2.0f)
				{
					objMppt2.SetPowerLimit(PV_MAX_POWER);
                    objBuckBoost2.SourceMode = PvUpvCtrl;
				}
				
				//MPPT限额
				if(objDataCalc.GetDcInVolt2Avg() >= 16.0f && objDataCalc.GetDcInVolt2Avg() <=23.5f)
				{
					UpDownLimit(objBuckBoost2.currRefLmt,PV_MAX_CURR_MPPT_1,0);
				}					
				else if(objDataCalc.GetDcInVolt2Avg() >= 33.0f && objDataCalc.GetDcInVolt2Avg() <=80.0f)
				{
					UpDownLimit(objBuckBoost2.currRefLmt,PV_MAX_CURR_MPPT_2,0);
				}
                
				//缓启动
                dcModeCurr2LmtSet+=CURR_CHANGE_DELTA;
                UpDownLimit(dcModeCurr2LmtSet, PV_MAX_CURR_1, 0);
				objBuckBoost2.currRefLmt = MIN2(objBuckBoost2.currRefLmt,dcModeCurr2LmtSet);
			}
			
			//过温限制
			if((chargePowerLmt * PV_MAX_POWER) < objDataCalc.GetDcIn2Power())
			{
				dcModeTempCurrLmt -= CURR_CHANGE_DELTA;
			}
			else
			{
				dcModeTempCurrLmt += CURR_CHANGE_DELTA;
			}
			UpDownLimit(dcModeTempCurrLmt,PV_MAX_CURR_1,0);
			objBuckBoost2.currRefLmt = MIN2(objBuckBoost2.currRefLmt, dcModeTempCurrLmt);
            
            //软启动电流限制
            Curr2SoftInc+= CURR_CHANGE_DELTA;
            UpDownLimit(Curr2SoftInc,objDataCalc.GetDcInCurr2Avg() + 1.0f,0);
            objBuckBoost2.currRefLmt = MIN2(objBuckBoost2.currRefLmt, Curr2SoftInc);
			
			//防止一路DC源接两路PV
			if(objBuckBoost.SlaveMode == 0 && objBuckBoost2.SlaveMode == 0)
			{
				if( 	(IsEqual(pvOpenVoltSave,pvOpenVolt2Save,0.5f) == 1)
					&& 	(IsEqual(objDataCalc.GetDcInCurrAvg(),objDataCalc.GetDcInCurr2Avg(),1.0f) == 1)
					&&  (IsEqual(objBuckBoost.GetDutySet(),objBuckBoost2.GetDutySet(),0.5f) == 0)
					&&  (objBuckBoost.GetDutySet() > 0)
				)
				{
					pv2ChkCnt++;
					if(pv2ChkCnt > 100) //2s
					{	
						pv2ChkCnt = 0;
						objBuckBoost2.SlaveMode = 1;
					}
							
				}
				else
				{	
					pv2ChkCnt = 0;
				}
			}
			else
			{
				pv2ChkCnt = 0;
				if(IsEqual(objDataCalc.GetDcInVoltAvg(),objDataCalc.GetDcInVolt2Avg(),2.0f) != 1)
				{
					objBuckBoost2.SlaveMode = 0;
					objBuckBoost2.SlaveDutySet = 0;
				}
			}
			
			if(objBuckBoost2.SlaveMode == 1)
			{
                if(objBuckBoost2.SlaveDutySet < (objBuckBoost.GetDutySet()-0.01f))
                {   
                    objBuckBoost2.SlaveDutySet += CURR_CHANGE_DELTA;
                }
                if(objBuckBoost2.SlaveDutySet > (objBuckBoost.GetDutySet()+0.01f))
                {   
                    objBuckBoost2.SlaveDutySet -= CURR_CHANGE_DELTA;
                }
				UpDownLimit(objBuckBoost2.SlaveDutySet,objBuckBoost.GetDutySet()+0.3f,objBuckBoost.GetDutySet()-0.3f);
			}
			
			//设置输出电压
			UpDownLimit(objBuckBoost2.voutMaxLmt, BATT_HIGH_POINT, 0);
			objBuckBoost2.SetVoRef(objBuckBoost2.voutMaxLmt);
			objBuckBoost2.SetState(BUCK_BOOST_ON);
			
		break;

		default:
			pv2State = PV_WORK_IDLE;
			objBuckBoost2.SetState(BUCK_BOOST_OFF);
			break;


	}
    */
}

void worklogic::BMUConmunicate()
{
    //MPPT协议数据传输
    objProtocol.pvVolt =  (UINT16)(objDataCalc.GetDcInVoltAvg() * 10.0f);
    //objProtocol.pv2Volt =  (UINT16)(objDataCalc.GetDcInVolt2Avg() * 10.0f);
    
	objProtocol.pv1CurrLmt = (UINT16)(objBuckBoost.currRefLmt * 10.0f);
	//objProtocol.pv2CurrLmt = (UINT16)(objBuckBoost2.currRefLmt * 10.0f);
    
    /*
    //总功率、总电流
    if(pvState == PV_WORK_NORMAL || pv2State == PV_WORK_NORMAL)
    {
        objProtocol.pvCurr = (UINT16)((objDataCalc.GetDcInCurrAvg() + objDataCalc.GetDcInCurr2Avg()) * 10.0f);
        objProtocol.pvPower =  (UINT16)((objDataCalc.GetDcInPower() + objDataCalc.GetDcIn2Power())*10.0f);
    }
    */

    if (pvState == PV_WORK_NORMAL)  //pvState是第1路的PV
    {
        objProtocol.pvCurr = (UINT16)(objDataCalc.GetDcInCurrAvg() * 10.0f);
        objProtocol.pvPower = (UINT16)(objDataCalc.GetDcInPower() * 10.0f);
    }

    else
    {
        objProtocol.pvCurr = objProtocol.pvPower = 0;
    }
    if(pvState == PV_WORK_NORMAL)
    {
        objProtocol.pv1Curr = (UINT16)((objDataCalc.GetDcInCurrAvg()) * 10.0f);
    }
    else
    {
        objProtocol.pv1Curr = 0;
    }

    //if(pv2State == PV_WORK_NORMAL)
    //{
    //    objProtocol.pv2Curr = (UINT16)((objDataCalc.GetDcInCurr2Avg()) * 10.0f);
    //}
    //else
    //{
    //    objProtocol.pv2Curr = 0;
    //}

    objProtocol.chargeVolt = (UINT16)(objDataCalc.GetBattVoltAvg() * 10.0f);
    
    //if(pvState == PV_WORK_NORMAL || pv2State == PV_WORK_NORMAL)
    //{
    //    objProtocol.chargeCurr = (UINT16)(objDataCalc.GetChargeCurrAvg() * 10.0f);
    //    objProtocol.chargePower =   (UINT16)((objDataCalc.GetBattVoltAvg() *  objDataCalc.GetChargeCurrAvg())* 10.0f);
    //}
    //else
    //{
    //    objProtocol.chargeCurr = objProtocol.chargePower = 0;
    //}

    if (pvState == PV_WORK_NORMAL)
    {
        objProtocol.chargeCurr = (UINT16)(objDataCalc.GetChargeCurrAvg() * 10.0f);
        objProtocol.chargePower = (UINT16)((objDataCalc.GetBattVoltAvg() * objDataCalc.GetChargeCurrAvg()) * 10.0f);
    }
    else
    {
        objProtocol.chargeCurr = objProtocol.chargePower = 0;
    }

    //objProtocol.TemperOffset = (INT16)(MAX2(objWorkLogic.t1,objWorkLogic.t2)*10.0f + 400);
    objProtocol.TemperOffset = (INT16)((objWorkLogic.t1) * 10.0f + 400);

    //objProtocol.alarm1.bit.pvInVoltHigh = pvHighChk.GetState() || pv2HighChk.GetState();
    objProtocol.alarm1.bit.pvInVoltHigh = pvHighChk.GetState();
    objProtocol.alarm1.bit.pvInVoltLow = 0;
    //objProtocol.alarm1.bit.dcChgFault = (sysFault && sys2Fault);
    objProtocol.alarm1.bit.dcChgFault = sysFault;
    objProtocol.alarm1.bit.fanFault = 0;
    //objProtocol.alarm1.bit.mosTempHigh = t1OverChk.GetState() || t2OverChk.GetState();
   
    objProtocol.alarm1.bit.mosTempHigh = t1OverChk.GetState();
    objProtocol.alarm1.bit.ntcFault = 0;//?NTC故障
	
    //objProtocol.alarm1.bit.pvCurrOver = pvOvCurrChk.GetState()  || pv2OvCurrChk.GetState();
    objProtocol.alarm1.bit.pvCurrOver = pvOvCurrChk.GetState();
	
    objProtocol.alarm1.bit.batChgOver = battOvCurrChk.GetState();

    
	if(objProtocol.chargeCtrl.bit.chargeOn == 1 && objProtocol.chargeCtrl.bit.chargeOff == 0)
    {
        hardFault.bit.powerOff = 0;
		
		objProtocol.chargeCtrl.bit.chargeOff = 0;
		objProtocol.chargeCtrlReal.bit.chargeOn = 1;
		objProtocol.chargeCtrlReal.bit.chargeOff = 0;
    }
	else
	{
		hardFault.bit.powerOff = 1;
		
		objProtocol.chargeCtrl.bit.chargeOn = 0;
		objProtocol.chargeCtrlReal.bit.chargeOn = 0; 
		objProtocol.chargeCtrlReal.bit.chargeOff = 1;
        objProtocol.chargePower = 0;
	 
	}
	
	if(objProtocol.powerOffCmd)
	{
		
	}

	/*设置信息*/
	//ywq_test
	// objProtocol.MPPTchargeCurrSet = 100.0f;//
	if(objProtocol.chargeCurr > objProtocol.MPPTchargeCurrSet)  //设置的充电电流小于当前充电电流，需要减小充电电流
	{
		if((objProtocol.chargeCurr - objProtocol.MPPTchargeCurrSet) > 10.0f) //超过1A的差距
		{
			dcModebmsCurrLmt  -= 5.0f * CURR_CHANGE_DELTA;
		}
		else
		{
			dcModebmsCurrLmt -= CURR_CHANGE_DELTA;
		}
	}
	else
	{
		dcModebmsCurrLmt += CURR_CHANGE_DELTA;
	}
    //dcModebmsCurrLmt = PV_MAX_CURR_1;//ywq_test
	UpDownLimit(dcModebmsCurrLmt, PV_MAX_CURR_1, 0);

	objBuckBoost.voutMaxLmt = objProtocol.MPPTchargeVoltSet * 0.1f;
    UpDownLimit(objBuckBoost.voutMaxLmt,CHARGE_VOLT_MAX,CHARGE_VOLT_MIN);

	//objBuckBoost2.voutMaxLmt = objProtocol.MPPTchargeVoltSet * 0.1f;
	//UpDownLimit(objBuckBoost2.voutMaxLmt,CHARGE_VOLT_MAX,CHARGE_VOLT_MIN);
    
    //objProtocol.workState = MAX2(pvState,pv2State);
    objProtocol.workState = pvState;
	
	objProtocol.MpptworkState = objMppt.Debug;
	//objProtocol.Mppt2workState = objMppt2.Debug;
	
	objProtocol.MpptUpvRef = (INT16)(objMppt.UpvRef * 10.0f);
	//objProtocol.Mppt2UpvRef = (INT16)(objMppt2.UpvRef * 10.0f);
    
    //升级
    if(objProtocol.iapCmd)
    {
        jumpBootCnt++;
        hardFault.bit.update = 1;
        if(jumpBootCnt >= 5)
        {
            RamDateType *flag = (RamDateType *)(RAM_SHARE_ADDR);
            flag->jump_flag = 1;
            flag->update_flag = 1;
            JumpTo(BOOT_ADDR);
        }
    }
    else
    {
        jumpBootCnt = 0;
        hardFault.bit.update = 0;
    }
    
    if(1 == objProtocol.e0Flag)
    {
        objFlashSave.SetInDcVoltCoff(objProtocol.calibrateCoff[0]);
        objSample.CalibrateInDcVolt(objFlashSave.GetInDcVoltCoff());
        
        objFlashSave.SetInDc2VoltCoff(objProtocol.calibrateCoff[1]);
        objSample.CalibrateInDc2Volt(objFlashSave.GetInDc2VoltCoff());
        
        objFlashSave.SetBattVoltCoff(objProtocol.calibrateCoff[2]);
        objSample.CalibrateBattVolt(objFlashSave.GetBattVoltCoff());
        
        objFlashSave.SetInDcCurrCoff(objProtocol.calibrateCoff[3]);
        objSample.CalibrateInDcCurr(objFlashSave.GetInDcCurrCoff());
        
        objFlashSave.SetInDc2CurrCoff(objProtocol.calibrateCoff[4]);
        objSample.CalibrateInDc2Curr(objFlashSave.GetInDc2CurrCoff());
        
        objFlashSave.SetBattCurrCoff(objProtocol.calibrateCoff[5]);
        objSample.CalibrateBattCurr(objFlashSave.GetBattCurrCoff());
       
        objProtocol.e0Flag = 2;

    }
    if((2 == objProtocol.e0Flag) && (pvState == PV_WORK_IDLE))
    {
        objFlashSave.WriteToFlash();
        objProtocol.e0Flag = 0;
    }
    objProtocol.calibrateCoffReal[0] = objFlashSave.GetInDcVoltCoff();
    objProtocol.calibrateCoffReal[1] = objFlashSave.GetInDc2VoltCoff();
    objProtocol.calibrateCoffReal[2] = objFlashSave.GetBattVoltCoff();
    objProtocol.calibrateCoffReal[3] = objFlashSave.GetInDcCurrCoff();
    objProtocol.calibrateCoffReal[4] = objFlashSave.GetInDc2CurrCoff();
    objProtocol.calibrateCoffReal[5] = objFlashSave.GetBattCurrCoff();  
     
    objProtocol.calibrateValue[0] = objDataCalc.GetDcInVoltAvg() * 10.0f + 0.5f;
    objProtocol.calibrateValue[1] = objDataCalc.GetDcInVolt2Avg() * 10.0f + 0.5f;
    objProtocol.calibrateValue[2] = objDataCalc.GetBattVoltAvg() * 10.0f + 0.5f;
    objProtocol.calibrateValue[3] = objDataCalc.GetDcInCurrAvg() * 10.0f + 0.5f;
    objProtocol.calibrateValue[4] = objDataCalc.GetDcInCurr2Avg() * 10.0f + 0.5f;
    objProtocol.calibrateValue[5] = objDataCalc.GetChargeCurrAvg() * 10.0f + 0.5f;

    
    
}

void worklogic::TemperatureChk()
{
    faultChkRetType para;
	
    FLOAT32 tempMax;
    UINT16 fanTemp;

   
    if((objDataCalc.GetTemp1Filt() > NTC_NO_CONNECT)  || (objDataCalc.GetTemp1Filt() < NTC_SHORT))
    {
        NtcFault1 = 1;
        t1 = 25.0f;
    }
    else
    {
        NtcFault1 = 0;
        t1 = buckNtc.CalcAdToTemp(objDataCalc.GetTemp1Filt());
    }
       
    //if((objDataCalc.GetTemp2Filt() > NTC_NO_CONNECT)  || (objDataCalc.GetTemp2Filt() < NTC_SHORT))
    //{
    //    NtcFault2 = 1;
    //    t2 = 25.0f;
    //}
    //else
    //{
    //    NtcFault2 = 0;
    //    t2 = buckNtc.CalcAdToTemp(objDataCalc.GetTemp2Filt());
    //}

    para.point = 100.0f;
    para.retPoint = 75.0f;
    para.cntMax = 600;
    para.inc = 60;
    para.dec = 1;
    t1OverChk.RetChkHigh(t1, &para);

    //para.point = 100.0f;
    //para.retPoint = 75.0f;
    //para.cntMax = 600;
    //para.inc = 60;
    //para.dec = 1;
    //t2OverChk.RetChkHigh(t2, &para);

    //tempMax = MAX2(t1, t2);
    //600W新增的BUCK电路的温度和mppt其余管子是一起的？
    tempMax = t1;
    fanStateNtc.execute(tempMax);
    chargeLevelState.execute(tempMax);
    

    //fanStatePower.execute(0.0f);


  //  if((objBuckBoost.GetState() != BUCK_BOOST_OFF) || (objBuckBoost2.GetState() != BUCK_BOOST_OFF)
		//||(t2OverChk.GetState() == 1) || (t1OverChk.GetState() == 1))
  //  
  //  {
  //      stopFanCnt = 0;
  //      fanEn = 1;
  //  }

    if ((objBuckBoost.GetState() != BUCK_BOOST_OFF) || (t1OverChk.GetState() == 1))
    {
        stopFanCnt = 0;
        fanEn = 1;
    }
    else
    {
        stopFanCnt++;
        if(stopFanCnt >= 6000)
        {
            stopFanCnt = 6000;
            fanEn = 0;
        }
    }
    
    if(fanEn)
    {
        //fanTemp = MAX2(fanStateNtc.GetState(), fanStatePower.GetState());
        fanTemp = fanStateNtc.GetState();
        if(stopFanCnt > 0)
        {
            fanTemp =  MIN2(2, fanTemp);
        }
        fanState = MAX2(fanTemp, objProtocol.fanSet);
         
    }
    else
    {
        fanState = objProtocol.fanSet;
    }
    
    
    
    switch (fanState) //两个FanPwm都放进去？
    {
    case 0:
        objProtocol.fanSpeedPercentage = 0;
        ARCH_SetFanPWM(1, 1.0f * FAN_PRD+1);
//        ARCH_SetFan2PWM(1, 1.0f * FAN_PRD + 1)
        break;
    case 1:
		objProtocol.fanSpeedPercentage = 20;
        ARCH_SetFanPWM(1, 0.54f * FAN_PRD);        //1500
        break;
    case 2:
		objProtocol.fanSpeedPercentage = 30;
        ARCH_SetFanPWM(1, 0.30f * FAN_PRD);          //3000
        break;
    case 3:
		objProtocol.fanSpeedPercentage = 40;
        ARCH_SetFanPWM(1, 0.15f * FAN_PRD);         
        break;
    case 4:
		objProtocol.fanSpeedPercentage = 50;
        ARCH_SetFanPWM(1, 0.1f * FAN_PRD);
        break;
    case 5:
		objProtocol.fanSpeedPercentage = 90;
        ARCH_SetFanPWM(1, 0.0f * FAN_PRD);
        break;
    default:
        break;
    }
 
    //ywq_test
  //  objProtocol.fanSpeedPercentage = 90;

	//ywq_modify:MPPT项目配置修改
	//if(fanState > 0)
	//{
	//	ARCH_FanCtrl(1);
	//}
	//else
	//{
	//	ARCH_FanCtrl(0);
	//}

	
    if(chargeLevelState.GetState() < CHARGE_POWER_LEVEL_NUM)
    {
        chargePowerLmt = chargePowerLevel[chargeLevelState.GetState()];
    }
    else
    {
        chargePowerLmt = chargePowerLevel[CHARGE_POWER_LEVEL_NUM - 1];
    }

}
void worklogic::FaultTrackWrite(UINT16 code, INT32 data1, INT32 data2, INT32 data3, INT32 data4)
{
    INT16 preIndex;
    
    preIndex = faultTraceIndex - 1;
    if(preIndex < 0)
    {
        preIndex += FAULT_TRACK_NUM;
    }

    if(code != faultTrace[preIndex])
    {
        faultTrace[faultTraceIndex] = code;
        faultCode1[faultTraceIndex] = data1;
        faultCode2[faultTraceIndex] = data2;
        faultCode3[faultTraceIndex] = data3;
        faultCode4[faultTraceIndex] = data4;
        faultTraceIndex++;
        if(faultTraceIndex >= FAULT_TRACK_NUM) faultTraceIndex = 0;
    }

}
void worklogic::InvProtect()
{
    
}

void worklogic::AcChargeSet()
{
    
}
void worklogic::WakeBmu()
{  
	switch (wakeUpBMU)
    {
    case WAKE_WAIT_1:
        if((!pvFault || !pv2Fault))
        {
            wakeUpBMU = WAKE_ONOFF_HIGH;
            wakeCnt = 0;
        }
        else if(objDataCalc.GetBattVoltAvg() > (16.0f))
        {
            wakeUpBMU = WAKE_END;
        }
        break;
    case WAKE_ONOFF_HIGH:
        WakeBMUStart();
        wakeUpBMU = WAKE_ONOFF_LOW;
        wakeCnt = 0;
        break;
    case WAKE_ONOFF_LOW:
        WakeBMUEnd();
        wakeUpBMU = WAKE_WAIT_2;
        break;
    
    case WAKE_WAIT_2:
        wakeCnt++;
        if(wakeCnt >= 4)
        {
            wakeUpBMU = WAKE_TX_LOW;
        }
        break;
    case WAKE_TX_LOW:
        InformBMUStart();
        wakeUpBMU = WAKE_TX_HIGH;
        break;
    case WAKE_TX_HIGH:
        InformBMUEnd();
        wakeUpBMU = WAKE_END;
        break;
    case WAKE_END:
        break;
    default:
        break;
    }
}


