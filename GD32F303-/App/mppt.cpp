#include "mppt.h"
#include "variable.h"
#include "arch.h"

#define RUN_FREQ			(10)
#define PV_POWER_MIN		(2.0f)
#define PV_POWER_MAX		(600.0f)
#define PV_POWER_CONST_CNT	(10.0f*RUN_FREQ)
#define PV_STEP_K			(0.1f)
#define PV_STEP_MIN			(0.2f)
#define PV_STEP_MAX			(1.0f)
#define PV_VOLTAGE_MIN		(10.0f)
#define PV_VOLTAGE_MAX		(80.0f)
#define PV_START_POINT		(0.70f)
#define PV_REF_MIN			(0.4f)
#define PV_REF_MAX			(1.2f)
#define PV_LIMIT_STEP		(1.0f/RUN_FREQ)

mppt::mppt()
{
     rst();
}

void mppt::SetPowerLimit(FLOAT32 vPvPowerLimit)
{
	PVPowerLimit = vPvPowerLimit;
	UpDownLimit(PVPowerLimit,PV_POWER_MAX,PV_POWER_MIN);
}

INT32 mppt::SetUoc(FLOAT32 Upv)
{
    if(WorkStatus == GetOpenVoltage)
    {
        if(Upv >= PV_VOLTAGE_MIN && Upv <= PV_VOLTAGE_MAX)
        {
            Uoc = Upv;
            
            UpvRefMax =  Uoc * PV_REF_MAX;
            UpDownLimit( UpvRefMax, PV_VOLTAGE_MAX, PV_VOLTAGE_MIN);
            UpvRefMin =  Uoc * PV_REF_MIN;
            UpDownLimit( UpvRefMin, PV_VOLTAGE_MAX, PV_VOLTAGE_MIN); 
            UpvRef =  Uoc - PV_STEP_MAX;
             
            UpDownLimit(UpvRef,UpvRefMax,UpvRefMin);
            
            WorkStatus = FirstUpvChange;
            return 1;
        }
    }
    return 0;
}   
 
FLOAT32 mppt::run(FLOAT32 Upv, FLOAT32 vPvPower)
{
    
    
    PVPower = vPvPower;
	
	if( PVPower >  PVPowerLimit)
	{
		 WorkStatus = PowerLimit;	
	}
	
	if( WorkStatus == GetOpenVoltage)
	{		 
       mpptCnt = 0;
	}
	else if( WorkStatus == FirstUpvChange)
	{
		//if(ARCH_Abs(Upv -  UpvRef) < 0.1f) //不需要这个条件,因为有可能永远也达不到
		{
			 PVPowerOld =  0;//PVPower;
			 
             UpvChangeDir = -1;
             PVPowerChange = 0;

             if(mpptCnt == 0)
             {
                UpvRef = Upv - PV_STEP_MAX;
             }
             
             if(mpptCnt >= RUN_FREQ)
             {
                mpptCnt = 0;
                WorkStatus  = UpvChangeLeft;
             }
             mpptCnt++;
             
		}
	}
	else if(	 WorkStatus == UpvChangeLeft 
			|| 	 WorkStatus == UpvChangeRight 
			|| 	 WorkStatus == UpvForceChange)
	{
        
		//更新老的功率值
		 PVPowerChange = ( PVPower -  PVPowerOld);
		if(ARCH_Abs( PVPowerChange) >= PV_POWER_MIN)
		{
			 PVPowerOld =  PVPower;
			 PVdPowerCnt = 0;
		}
		else
		{
			 PVPowerChange = PV_POWER_MIN;
			 PVdPowerCnt++;
		}

		//确定扰动方向
		if( PVPowerChange < 0)
		{
			 UpvChangeDir = - UpvChangeDir;
		}
        
        if(UpvChangeDir > 0)
        {
            WorkStatus = UpvChangeRight;
        }
        else
        {
            WorkStatus = UpvChangeLeft;
        }
        
		if( PVdPowerCnt >= PV_POWER_CONST_CNT)
		{	//按常识，如果一直保持功率不变，可能是由于电压一直卡死在开路电压了
			 PVdPowerCnt = PV_POWER_CONST_CNT;
			 //UpvChangeDir = -UpvChangeDir;
             UpvChangeDir = -1;
			 WorkStatus = UpvForceChange;
		}
		//确定扰动步长
		UpvChangeStep =ARCH_Abs( PVPowerChange) * PV_STEP_K;
		UpDownLimit(UpvChangeStep,PV_STEP_MAX,PV_STEP_MIN);
		
		//确定期望电压
        if(WorkStatus == UpvForceChange)
        {
            //左右(+5V,-5V)区间扫描
			if (Upv <= PV_VOLTAGE_MIN || UpvRef <= PV_VOLTAGE_MIN || UpvRef < (Upv - PV_STEP_MAX * 5))
			{
				UpvChangeDir = 1;
				// UpvRef += UpvChangeStep;    
			}
			if (Upv >= PV_VOLTAGE_MAX || UpvRef >= PV_VOLTAGE_MAX || UpvRef > (Upv + PV_STEP_MAX * 5))
			{
				UpvChangeDir = -1;
				// UpvRef -= UpvChangeStep;
			}
            UpvRef += (PV_STEP_MAX / RUN_FREQ) *  UpvChangeDir;
        }
        else
        {
            UpvRef  =Upv + UpvChangeStep *  UpvChangeDir;	
        }
       
	}
	else if( WorkStatus == PowerLimit)
	{
		if( PVPower <  PVPowerLimit)
		{
			 WorkStatus = FirstUpvChange;
		}
		else
		{
			 UpvRef  = Upv + PV_LIMIT_STEP;
			 PVPowerOld =  PVPower;
		}
	}
	else
	{
		rst();
	}

	//UpDownLimit( UpvRef, UpvRefMax, UpvRefMin);
	UpDownLimit( UpvRef,PV_VOLTAGE_MAX,PV_VOLTAGE_MIN);
	Debug = WorkStatus;
    
	return  UpvRef;
	
	
}
/*****************************************
 * MPPT复位
 * volt：当前pv电压
 * **************************************/
void mppt::rst()
{
     UpvRef = PV_VOLTAGE_MAX;
	 WorkStatus = GetOpenVoltage;
	 PVPowerOld = 0;
	 UpvChangeDir = -1;
	 PVPowerLimit = PV_POWER_MAX;
	 PVdPowerCnt = 0;
	 UpvRefMax = PV_VOLTAGE_MAX;
	 UpvRefMin = PV_VOLTAGE_MIN;
     mpptCnt = 0;
}

