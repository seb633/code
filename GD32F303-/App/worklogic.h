#ifndef _WORK_LOGIC_H_
#define _WORK_LOGIC_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "piecewise.h"
#include "faultChk.h"
#include "ntc.h"
#include "standard.h"
#include "multistate.h"
#include "PIController.h"


#define FAULT_TRACK_NUM (10)
//整机状态
enum eTotalState
{
    TOTAL_IDLE = 0,                             //空闲等待
    TOTAL_BUS_CHARGE = 1,                       //母线充电
    TOTAL_CHARGE = 2,                           //充电
    TOTAL_BYP_CHARGE = 3,                       //旁路+充电
    TOTAL_BYP  = 4,                             //旁路
    TOTAL_DISCHARGE  = 5,                       //放电
    TOTAL_DISCHARGE_INV  = 6,                    //放电+逆变
};

union acFaultStu
{
    UINT32 word;
    struct
    {
        UINT32 acInputVoltOv : 1;              //电网有效值高压  0
        UINT32 acInputVoltUn : 1;              //电网有效值低压   1
        UINT32 acInputFreqOv : 1;              //电网频率高      2
        UINT32 acInputFreqUn : 1;              //电网频率低      3


        UINT32 invVoltFast : 1;                //逆变电压快检      4
        UINT32 lockFail : 1;                    //锁相异常          5
        UINT32 latch : 1;                        //快速故障锁存      6
        UINT32 rsvd2 : 11;                      //7-31

    }bit;
};
union hardwareFaultStu
{
    UINT32 word;
    struct
    {
        UINT32 t1Over : 1;                      //过温  0
        //UINT32 t2Over : 1;                      //过温   1

        UINT32 powerOff : 1;
        UINT32 update : 1;
        UINT32 overLoad : 1;

        UINT32 battOvCurr : 1;
        UINT32 battVoltLow : 1;                 //电池电压极低，不开机
        UINT32 llcFault : 1;                    //LLC故障      5
        
        UINT32 powerUpDly : 1;
        UINT32 busFastOver : 1;                 //母线过压          6
        UINT32 invOcp : 1;                      //逆变限流          7
        UINT32 invShort : 1;
        
        UINT32 latch :1;                        //快速故障锁存      8
        
        UINT32 rsvd2 : 15;                      //8-31

    }bit;
};
union bypFaultStu
{
    UINT32 word;
    struct
    {
        UINT32 freqOv : 1;
        UINT32 freqUn : 1;

        UINT32 rsvd2 : 30;
    }bit;
};

//国家
enum eCountry
{
    COUNTRY_CHINA  = 0,
    COUNTRY_BRAZIL
};
enum ePvWorkState
{
    PV_WORK_IDLE            = 0,
    PV_WORK_CHK             = 1,
    PV_WORK_NORMAL          = 2,
    PV_WORK_WARN            = 3,
    PV_WORK_FAULT           = 4,
};
enum eWakeBmuStep
{
    WAKE_WAIT_1 = 0,
    WAKE_ONOFF_HIGH,
    WAKE_ONOFF_LOW,
    WAKE_WAIT_2,
    WAKE_TX_LOW,
    WAKE_TX_HIGH,
    WAKE_COMM_INIT,
    WAKE_END
};
class worklogic
{
//private:
public://ywq_test
    UINT16 totalCnt;
    UINT16 chargeOnCmd;                 //充电ON命令
    UINT16 lowSoundCmd;

    faultChk battVoltLowChk;
    faultChk battVoltHighChk;
    faultChk battVoltVeryLowChk;
    faultChk pvHighChk;
	
    //faultChk pv2HighChk;
   
    faultChk pvLowChk;
	//faultChk pv2LowChk;
	faultChk pvOvCurrChk;
	//faultChk pv2OvCurrChk;
    faultChk battOvCurrChk;

    UINT32 powerUpTime;                 //上电计时
    UINT16 noFaultCnt;                  //故障清除计数
    UINT16 faultComfirm;                //故障清除确认
    UINT16 waitCnt;                     //空闲状态 到 自检的等待计数
    UINT16 waitEnd;                     //空闲状态 到 自检，等待完成


    ntc buckNtc;                         
    ntc boostNtc;                        
    FLOAT32 t1;                          
    FLOAT32 t2;                          
    faultChk t1OverChk;                
    //faultChk t2OverChk;             
    multistate fanStateNtc;
    multistate fanStatePower;
    
    multistate chargeLevelState;
    FLOAT32 chargePowerLmt;
    
    UINT16 stopFanCnt;
    UINT16 fanEn;
    
    UINT16 fanState;
    UINT16 NtcFault1;
    UINT16 NtcFault2;

    //故障检测
    faultChk rmsHighChk;      //高压
    faultChk rmsLowChk;       //低压
    faultChk rmsLowForAlarm;
    faultChk rmsHighForAlarm;
    faultChk currFastChk;     
    faultChk shortChk;

    UINT16 reCalibrate;


    UINT16 jumpBootCnt;
    
    ePvWorkState pvState;           //PV1 工作状态
	ePvWorkState pv2State;           //PV2 工作状态
    FLOAT32 pvOpenVoltSave;
	FLOAT32 pvOpenVolt2Save;
	FLOAT32 SourceResistor;
	FLOAT32 Source2Resistor;
    UINT16 pvChkCnt;
	UINT16 pv2ChkCnt;
    UINT16 mpptCnt;
    UINT16 mppt2Cnt;
    UINT16 pvSourceMode;            //1:直流源      0：光伏板
    FLOAT32 pvVoltLowPoint;
	
    FLOAT32 bmsNeedPvCurrLmt;
    FLOAT32 bmsNeedPowerLmt;
    faultChk pvVoltLossChk;
	faultChk pv2VoltLossChk;
    FLOAT32 dcModeCurrChk;
	FLOAT32 dcModeCurr2Chk;
    FLOAT32 CurrSoftInc;    
    FLOAT32 Curr2SoftInc; 
    FLOAT32 dcModeCurrLmtSet;
    UINT16  dcModeCurrLmtCnt;
    FLOAT32 dcModeCurr2LmtSet;
    UINT16  dcModeCurr2LmtCnt;
	FLOAT32 dcModebmsCurrLmt;
	FLOAT32 dcModeTempCurrLmt;
	
	
    UINT16 cpFlag;
    faultChk powerProtect;
    faultChk overLoadChk;
    UINT16 cpExitCnt;
    piecewise powerProtectLine;
    faultChk constPowerLmtChk;
    faultChk ratioChk;
    
    eWakeBmuStep wakeUpBMU;
    UINT16 wakeCnt;

public:
    standard *pSd;                      //国家标准
    eCountry country;                   //国家
    hardwareFaultStu hardFault;
    UINT32 pvFault;
	UINT32 pv2Fault;
	UINT32	batFault;
    UINT32  tempFault;
    UINT32  sysFault;
	UINT32  sys2Fault;
    UINT16  sysFaultWait;
    UINT16  sys2FaultWait;

    INT16 faultTraceIndex;
    UINT16 faultTrace[FAULT_TRACK_NUM];
    INT32 faultCode1[FAULT_TRACK_NUM];
    INT32 faultCode2[FAULT_TRACK_NUM];
    INT32 faultCode3[FAULT_TRACK_NUM];
    INT32 faultCode4[FAULT_TRACK_NUM];


    worklogic(/* args */);
    void init();
    void TotalStateShift();
    void TotalStateOutput();
    void BuckBoostStateShift();
    void FaultCollect();
    eCountry GetContry() {return country;};
    void BoostLogicDc();
    void BoostLogicMppt();
    void BoostLogicMpptPara();
    void InvLogicVolt();
    void InvLogicCurr();

    void FastOff();
    void BusTargetSet();
    void InvRmsSet();

    void OutputPowerLmt();
    void OutputPowerLmtDc();
    void SetStandard(eCountry arg);
    void BMUConmunicate();
    void TemperatureChk();

    void AcChargeSet();
    void InvProtect();
    void WakeBmu();
    FLOAT32 FakePowerGen(FLOAT32 Power, FLOAT32 Ipv);

    INLINE_USER void PowerUpTick() {powerUpTime++;};
    INLINE_USER UINT32 GetPowerUpTime() {return powerUpTime;};
    INLINE_USER void SetReCalibrate() {reCalibrate = 1;};
    INLINE_USER void SetChargeCmd(UINT16 arg) {chargeOnCmd = arg;};
    INLINE_USER void FaultTrackWrite(UINT16 code, INT32 data1, INT32 data2, INT32 data3, INT32 data4);
    INLINE_USER FLOAT32 GetPvPowerLmt(){return bmsNeedPowerLmt;};
    bool IsCarChargeMode(FLOAT32 Volt);
   
    friend class standard;
    friend class rated;
};

#ifdef __cplusplus
}
#endif
#endif
