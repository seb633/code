#ifndef _INV_CLASS_H_
#define _INV_CLASS_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "prcontroller.h"
#include "faultChk.h"
#include "PIController.h"

//逆变器工作模式
enum eInvWorkMode
{
    INV_WORK_MODE_OFF = 0,          //关机
    INV_WORK_MODE_INV,      //电压源
    INV_WORK_MODE_REC       //电流源
};


class invclass
{
private:


public:
//REC
    prcontroller pr1;
    prcontroller pr3;
    PIController busVoltLoop;
    FLOAT32 recCurrKp;
    FLOAT32 invBusGoal;
    FLOAT32 invBusRef;

//INV
    PIController rmsLoop;
    PIController vtLoop;
    FLOAT32 invCurrKp;
    FLOAT32 rmsRef;
    FLOAT32 rmsDrop;
    FLOAT32 rmsGoal;

//快检

//参数
    eInvWorkMode invWorkMode;           //工作模式
    eInvWorkMode invWorkModeReal;           //工作模式



//驱动
    INT16 cmp;
    UINT16 pwmEn;

    invclass(/* args */);

    void InvReg();
    void BusSoftStart();

    INLINE_USER void SetInvMode(eInvWorkMode arg) {invWorkMode = arg;};
    INLINE_USER void SetBusGoal(FLOAT32 arg) {invBusGoal = arg;};
    INLINE_USER void SetInvRms(FLOAT32 arg) {rmsRef = arg;};
    INLINE_USER FLOAT32 GetInvRmsRef() {return rmsRef;};
    INLINE_USER void SetInvRmsDrop(FLOAT32 arg) {rmsDrop = arg;};
    INLINE_USER void SetInvRmsGoal(FLOAT32 arg) {rmsGoal = arg;};




    friend class rated;

};

#ifdef __cplusplus
}
#endif
#endif

