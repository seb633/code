#ifndef _BUCK_BOOST_H_
#define _BUCK_BOOST_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "PIController.h"

enum BuckBoostState
{
    BUCK_BOOST_OFF = 0,
    BUCK_BOOST_ON = 1
};

enum WorkStatus 
 {
 DcUbatCtrl = 0,
 DcIdcCtrl,
 PvUbatCtrl,
 PvUpvCtrl,
 };


class buckboost
{
//private:
    public://ywq_test  
	INT32  SlaveMode;
    BuckBoostState state;
	WorkStatus SourceMode; 
	PIController PVPowerLoop; //ywq_modify:MPPT项目配置修改
    PIController PVvoltLoop;
    PIController PVcurrLoop;

    PIController UbatLoop;

	FLOAT32 vinErrPre;
    FLOAT32 vinRef;

	FLOAT32 IpvErrPre;
	FLOAT32 IpvRef;
	FLOAT32 IpvGoal;
    FLOAT32 voutRef;
    FLOAT32 voutMaxLmt;
    FLOAT32 currRefLmt;//电流限制值
    FLOAT32 DutyStartBoostCnt;
    FLOAT32 DutySet;
    FLOAT32 DutySetOffset;
	FLOAT32 SlaveDutySet;
    FLOAT32 DutyMax;
    INT32   intDuytSet;
    INT32   OverMode;
	FLOAT32 UpvOld;
	 

    UINT16 onoff;
    INT16 boostCmp;
    INT16 buckCmp;
	UINT32 OutMosOnOffCnt;
    FLOAT32 openLoopDuty;
public:
  //  FLOAT32 vinSoftStart;//Ubat电压软启动

    buckboost(/* args */);

    void Regulation(FLOAT32 Upv,FLOAT32 Ipv);
	void IpvSoftStart();

    INLINE_USER void SetState(BuckBoostState arg) {state = arg;};  //SetIpvGoalLimit
    INLINE_USER void SetVinRef(FLOAT32 arg) {vinRef = arg;};
	INLINE_USER void SetIpvGoal(FLOAT32 arg) {IpvGoal = arg;};
	INLINE_USER void SetIpvGoalLimit(FLOAT32 arg) {currRefLmt = arg;};
    INLINE_USER void SetVoRef(FLOAT32 arg) {voutRef = arg;}; //SetIpvGoalLimit
	INLINE_USER void SetSlaveDutySet(FLOAT32 arg) {SlaveDutySet = arg;}; //SetIpvGoalLimit
	INLINE_USER FLOAT32 GetDutySet() {return DutySet;};
    INLINE_USER UINT16 GetOnoff() {return onoff;};
    INLINE_USER INT16 GetBoostCmp() {return boostCmp;};
    INLINE_USER INT16 GetBuckCmp() {return buckCmp;};
    INLINE_USER BuckBoostState GetState() {return state;};
    INLINE_USER void SetOpenLoopDuty(FLOAT32 arg) {openLoopDuty = arg;};
	
    
    friend class rated;
    friend class worklogic;
};



#ifdef __cplusplus
}
#endif

#endif

