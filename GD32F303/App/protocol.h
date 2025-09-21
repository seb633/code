#ifndef _PROTOCOL_H_
#define _PROTOCOL_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "uartDrv.h"

#define FIRST_BYTE      (0x5A)
#define SECOND_BYTE     (0xA5)
#define APP_ADDR        (0xA1)
#define BMS_ADDR        (0xB0)
#define BMU_ADDR        (0xC0)
#define INV_ADDR        (0xD0)
#define MPPT_ADDR       (0xE0)
#define CALIBRATE_NUM   (6)
//故障表，保留
union FaultType
{
    UINT16 all;
    struct
    {
        UINT16 pvInVoltHigh : 1;
        UINT16 pvInVoltLow : 1;
        UINT16 dcChgFault : 1;
        UINT16 fanFault : 1;

        UINT16 mosTempHigh : 1;
		UINT16 mosTempLow : 1;
        UINT16 ntcFault : 1;
		UINT16 pvCurrOver:1;
		UINT16 batChgOver:1;
    }bit;
    
};
union acWorkModeType
{
    UINT16 all;
    struct
    {
        UINT16 res1 : 1;
        UINT16 independent : 1;
        UINT16 parallel : 1;
        UINT16 constP : 1;
        
        UINT16 resv : 12;


    }bit;
};


union ChargeCtrlDef
{
    UINT16 all;
    struct
    {
        UINT16 chargeOff : 1;
        UINT16 chargeOn : 1;
        UINT16 lowSoundOff : 1;
        UINT16 lowSoundOn : 1;
        UINT16 chargeCondition : 1;
        UINT16 resv : 11;
    }bit;
};

class protocol
{
private:
    UINT8 txBuf[256];                   //发送缓存
    UINT8 rxBuf[256];                   //接收缓存
    UINT16 rxNum;                       //实际接收的个数

    UINT16 softVersion;
    UINT16 pvVolt;
    UINT16 pv2Volt;
    UINT16 pvCurr;
	UINT16 pv1Curr;
	UINT16 pv1CurrLmt;
	UINT16 pv2Curr;
	UINT16 pv2CurrLmt;
	UINT16 batCurr;
    UINT16 pvPower;
    UINT16 pvCurrMax;
    FaultType alarm1;
    UINT16 alarm2;
    UINT16 fault1;
    UINT16 fault2;
    UINT16 selfChkTime;
 
    UINT16 pvHighLmt;
    UINT16 pvLowLmt;
    UINT16 opCurrCal;
    UINT16 opVoltCal;

    ChargeCtrlDef chargeCtrl;
    ChargeCtrlDef chargeCtrlReal;

    UINT16 chargePowerMax;
    UINT16 chargeVoltMax;
    UINT16 chargeLowPowerCnt;

    UINT16 bmsTemp;
    UINT16 bmsVolt;
    INT16 bmsCurr;
    UINT16 lowPowerSet;


    UINT8 workState;
	UINT8 MpptworkState;
	INT16 MpptUpvRef;
	UINT8 Mppt2workState;
	INT16 Mppt2UpvRef;
    UINT8 fanReal;
    UINT8 fanSet;               //设置风扇速率
	UINT8 fanSpeedPercentage;  //风扇速率百分比
    UINT8 powerOffCmd;
    UINT8 iapCmd;              //升级标识

    UINT16 commLostCnt;
    UINT16 commLostState;
    
    UINT16 voltSet;
    
    UINT16 inAcCoff;
    UINT16 inAcCoffReal;
    UINT16 invCoff;
    UINT16 invCoffReal;
    UINT16 inDcCCoff;
    UINT16 inDcCCoffReal;

    
    //MPPT_Para
    UINT16 chargeVolt;
    UINT16 chargeCurr;
    UINT16 chargePower;
    INT16 TemperOffset;
    
    UINT16 chargeOff;
    UINT16 chargeOn;
    UINT16 chargeLowNoiseOff;
    UINT16 chargeLowNoiseOn;

    UINT16 MPPTchargePowerSet;
    UINT16 MPPTchargeVoltSet;
    UINT16 MPPTchargeCurrSet;
	
	//校准
    UINT16 calibrateCoff[CALIBRATE_NUM];        //设置的校准系数
    UINT16 calibrateCoffReal[CALIBRATE_NUM];     //校准系数实际值
    UINT16 calibrateValue[CALIBRATE_NUM];        //需要校准的值
    UINT16 e0Flag;    
public:
    uartDrv comm;
    protocol(/* args */);
    void deal();

    void SetPara(const uartPara *para);
    INLINE_USER void ClearChargeCtrlOff() {chargeCtrl.bit.chargeOff = 0;};
    INLINE_USER void ClearChargeCtrlOn() {chargeCtrl.bit.chargeOn = 0;};
    //void inAcCoffInit(UINT16 arg);
    //void invCoffInit(UINT16 arg);
    //void inDcCCoffInit(UINT16 arg);
	void CalibrateCoffInit(UINT16 n, UINT16 arg);
    friend class worklogic;
};





#ifdef __cplusplus
}
#endif
#endif
