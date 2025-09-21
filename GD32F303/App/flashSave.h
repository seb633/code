#ifndef _FLASH_SAVE_H_
#define _FLASH_SAVE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"
#include "gd32f30x_libopt.h"
#include "arch.h"

    
    
typedef struct 
{
//    UINT16 invVoltCoff;
//    UINT16 inAcVoltCoff;
//    UINT16 inDcCurrCoff;
//    UINT16 dataCrc;
    
    UINT16 InDcVoltCoff;       //PV0电压 系数
    UINT16 InDc2VoltCoff;       //PV1电压 系数
    UINT16 BattVoltCoff;        //电池电压 系数
    UINT16 InDcCurrCoff;       //PV0电流 系数
    UINT16 InDc2CurrCoff;       //PV1电流 系数
    UINT16 BattCurrCoff;        //电池电流 系数
    UINT16 dataCrc;            //校验
}flashDataStruct;

//typedef union
//{
//    flashDataStruct str;
//    UINT8 byte[2048];
//}flashDataUnion;    

class flashSave
{
private:
    void *ptr;
    flashDataStruct currPara;
public:
    flashSave(/* args */);
    void Init(void *arg);
    UINT16 GetSaveLen();
    
    INLINE_USER UINT16 DataLmt(UINT16 *data, UINT16 arg);
    void WriteToFlash();
    
    //PV0输入电压
    UINT16 SetInDcVoltCoff(UINT16 arg);
    INLINE_USER UINT16 GetInDcVoltCoff() {return currPara.InDcVoltCoff;};
    
    //PV1输入电压
    UINT16 SetInDc2VoltCoff(UINT16 arg);
    INLINE_USER UINT16 GetInDc2VoltCoff() {return currPara.InDc2VoltCoff;};
    
    //电池电压
    UINT16 SetBattVoltCoff(UINT16 arg);
    INLINE_USER UINT16 GetBattVoltCoff() {return currPara.BattVoltCoff;};
    
    //PV0输入电流
    UINT16 SetInDcCurrCoff(UINT16 arg);
    INLINE_USER UINT16 GetInDcCurrCoff() {return currPara.InDcCurrCoff;};
    
    //PV1输入电流
    UINT16 SetInDc2CurrCoff(UINT16 arg);
    INLINE_USER UINT16 GetInDc2CurrCoff() {return currPara.InDc2CurrCoff;};
    
    //电池电流
    UINT16 SetBattCurrCoff(UINT16 arg);
    INLINE_USER UINT16 GetBattCurrCoff() {return currPara.BattCurrCoff;};
    
};

INLINE_USER UINT16 flashSave :: DataLmt(UINT16 *data, UINT16 arg)
{
    UINT16 ret = 0;
    if((arg < CALIBRATE_MIN) || (arg > CALIBRATE_MAX))
    {
        arg = CALIBRATE_CONSTANT;
        ret = 1;
    }
    *data = arg;
    return ret;
}




#ifdef __cplusplus
}
#endif
#endif


