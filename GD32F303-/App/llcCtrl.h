#ifndef _LLC_CTRL_H_
#define _LLC_CTRL_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "PIController.h"
//#include "func.h"

enum eLlcWorkMode
{
    LLC_WORK_MODE_OFF = 0,              //关机
    LLC_WORK_MODE_CHARGE,               //充电
    LLC_WORK_MODE_DISCHARGE             //放电
};
class llcCtrl
{
private:
    eLlcWorkMode llcModePre;
    FLOAT32 busGoal;
    FLOAT32 battGoal;
    FLOAT32 busRef;
    FLOAT32 battRef;
    FLOAT32 chargeCurrLmt;
    FLOAT32 disChargeCurrLmt;

//发驱动
    INT16 prd;
    INT16 dead;
    UINT16 pwmEn;

    PIController dchgVoltLoop;             //放电电压环
    PIController dchgCurrLoop;             //放电电流环
    FLOAT32 dchgFilter;

    PIController chgVoltLoop;             //充电电压环
    PIController chgCurrLoop;             //充电电流环
public:
    eLlcWorkMode llcMode;

    llcCtrl(/* args */);
    void llcReg();
    void busSoftStart();


    INLINE_USER void SetBusGoal(FLOAT32 val) {busGoal = val;};
    INLINE_USER FLOAT32 GetBusGoal() {return busGoal;};
    INLINE_USER void SetBattGoal(FLOAT32 val) {battGoal = val;};
    INLINE_USER FLOAT32 GetBattGoal() {return battGoal;};



    INLINE_USER void SetBattRef(FLOAT32 val) {battRef = val;};
    INLINE_USER void SetLlcMode(eLlcWorkMode arg) {llcMode = arg;};
    INLINE_USER UINT16 GetPwmEn() {return pwmEn;};
    INLINE_USER UINT16 GetPwmPrd() {return prd;};
    INLINE_USER UINT16 GetPwmDead() {return dead;};

    friend class rated;
    friend class worklogic;

};





#ifdef __cplusplus
}
#endif
#endif

