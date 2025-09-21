#include "buckboost.h"
#include "config.h"
#include "arch.h"
#include "SEGGER_RTT.h"

#define BUCK_MAX        	(0.90f)
#define BOOST_MIN        	(DEADTIME * 0.00008f)
#define OUT_MOS_ON_OFF_CNT  (5*PWM_FREQ) //5s

#if 1
#include "sample.h"
#include "rated.h"
#include "dataCalc.h"
extern sample objSample;
extern rated objRated;
extern dataCalc objDataCalc;

#else

#endif

buckboost::buckboost(/* args */)
{
    voutMaxLmt = BATT_HIGH_POINT;
    currRefLmt = PV_MAX_CURR_1;

	UpvOld = 0;
    SourceMode = DcUbatCtrl;
    DutySet = 0;
	SlaveMode = 0;
	SlaveDutySet = 0;
}


void buckboost::Regulation(FLOAT32 Upv,FLOAT32 Ipv)
{
     
    FLOAT32 Ibat = 0,UbatErr,IpvErr,UpvErr,Icpv = 0,vout;
    FLOAT32 UpvSafe = Upv;
	FLOAT32 UbatSafe = 0;
    
	UpvOld = Upv;
    vout = objSample.GetBattVolt();
    Ibat = objSample.GetChargeCurr();
    
    UpDownLimit(UpvSafe, DC_IN_VOLT_HIGH_POINT, DC_IN_VOLT_LOW_POINT);	//输入电压
	
	UbatSafe = objDataCalc.GetBattVoltAvg();
	UpDownLimit(UbatSafe, BATT_HIGH_POINT, BATT_LOW_POINT);

	//迅速过压保护(突卸负载保护)
	if (vout > CHARGE_VOLT_FAST_OFF)
	{
		state = BUCK_BOOST_OFF;
	}

	//buck boost发波
    if(state == BUCK_BOOST_OFF)
    {
        OverMode = 0;
        vinRef = 0;
		voutRef = 0;
        DutySet = 0;  
		
		//加入占空比初始偏移
		
		if(UbatSafe > BATT_LOW_POINT && Upv > DC_IN_VOLT_LOW_POINT)	//在正常工作范围内？
		{
			if(UpvSafe > UbatSafe)	//输入电压大于电池电压
			{
				DutySetOffset = UbatSafe / UpvSafe;	//Ki ???
			}
			else
			{
				DutySetOffset =  2.0f - UpvSafe / UbatSafe; //Av = 1/(1-d) ==>   d = 1 - 1/Av
			}
			DutySetOffset -= 0.3f;//保守量。这个条件不会覆盖前面吗
		}
		else
		{
			DutySetOffset = 0;
		}
        UpDownLimit(DutySetOffset,BUCK_MAX + 0.5f,0);
		
        onoff = 0;
        boostCmp = 0;
        buckCmp = 0;
        PVvoltLoop.rst();
        PVcurrLoop.rst();
        UbatLoop.rst();     
        PVcurrLoop.SetInteg(DutySetOffset);
		DutyStartBoostCnt = PWM_FREQ;
    }
    else if(state == BUCK_BOOST_ON)
    {	 
			if(     SourceMode == DcUbatCtrl 
                ||  SourceMode == PvUbatCtrl)
			{
                
                UbatErr = voutRef - vout;//电池侧的电压环路？？

				UpDownLimit(UbatErr, 5.0f, -5.0f);	//决定软启动速度
                if(ARCH_Abs(UbatErr - vinErrPre) > 0.01f)	//钝化误差
                {
                    vinErrPre = UbatErr;
                }
				IpvGoal = UbatLoop.run(vinErrPre);

				UbatLoop.SetMax(currRefLmt);
				UbatLoop.SetMin(-currRefLmt);
			}
            else if(SourceMode == DcIdcCtrl)	//没用？？
            {

            }
            else if(SourceMode == PvUpvCtrl)  
			{
			     UpvErr = Upv - vinRef;	//输入电压误差
                 UpDownLimit(UpvErr, DC_IN_VOLT_HIGH_RET_POINT, - DC_IN_VOLT_HIGH_RET_POINT);
                 
                 IpvRef = PVvoltLoop.run(UpvErr);
                 
                 PVvoltLoop.SetMax(currRefLmt);
				 PVvoltLoop.SetMin(-currRefLmt);
			}
			else
			{
				IpvRef = 0;
                SourceMode = DcUbatCtrl;
			}

			IpvRef = MIN2(IpvRef,currRefLmt);	//限制电流参考值上限？
			UpDownLimit(IpvRef,PV_MAX_CURR_1,-PV_MAX_CURR_1);
            
            //600w功率限制
            //UpDownLimit(Upv,DC_IN_VOLT_HIGH_POINT,DC_IN_VOLT_LOW_POINT);
            IpvRef = MIN2(IpvRef, PV_MAX_POWER / UpvSafe);
			
			
			
			IpvErr = IpvRef - (Ipv - Icpv); //Ipv ref为负数的意思就是要关断BUCKBOOST了；Icpv是干嘛的？？？？
			//IpvErr = IpvRef - Ipv;
			UpDownLimit(IpvErr,PV_MAX_CURR_1,-PV_MAX_CURR_1);

			
            if(ARCH_Abs(IpvErr - IpvErrPre) > 0.01f)//钝化误差;没有赋初值？？？
            {
                IpvErrPre = IpvErr; 
            }

            //防止突卸负载时电压太超 
            FLOAT32 loopDutySet = PVcurrLoop.run(IpvErrPre);	//电流环
            
            UpDownLimit(loopDutySet, BUCK_MAX * 2.0f, - BUCK_MAX * 2.0f);

			if(1 == SlaveMode)
			{
				UpDownLimit(SlaveDutySet, BUCK_MAX * 2.0f, - BUCK_MAX * 2.0f);
				loopDutySet = SlaveDutySet;
			}
            
            DutySet = loopDutySet;
            
            intDuytSet = (INT32)(DutySet * 100.0f);	//没用到？？？
            
			onoff = 2;
			
			if (DutyStartBoostCnt > 0)
			{
				DutyStartBoostCnt--;
			}

            FLOAT32 DutySetTemp = 0;
            DutySetTemp = DutySet;
			
			if(DutySetTemp <= 0.0f)
			{
				buckCmp = 0;
				boostCmp = 0;
				DutyStartBoostCnt = PWM_FREQ;
			}

            
           // 存在电感电流在Boost管附近震荡的问题 Duty = 0.86~0.88变化时
			else if(DutySetTemp < BUCK_MAX)
			{
				buckCmp = DutySetTemp * BUCK_BOOST_PWM_PRD;
				if (DutyStartBoostCnt > 0)
				{
					boostCmp = 0.15f * BUCK_BOOST_PWM_PRD;
				}
				else
				{
					boostCmp = 0;
				}			
			}
			else
			{
				DutyStartBoostCnt = 0;
				DutySetTemp -= BUCK_MAX;
				DutySetTemp += BOOST_MIN; //加入偏移
				UpDownLimit(DutySetTemp, BUCK_MAX, 0);
				buckCmp = BUCK_MAX * BUCK_BOOST_PWM_PRD;
				boostCmp= DutySetTemp * BUCK_BOOST_PWM_PRD;
			}	
            
  	 }

}

void buckboost::IpvSoftStart()
{
   IpvRef = IpvGoal;

}
