#ifndef _SOGI_PLL_H_
#define _SOGI_PLL_H_

#include "usertypes.h"
#include "PIController.h"
#include "faultChk.h"


class sogiPll
{
private:
    UINT16 mode;                //0:额定频率运行    1：锁相
    FLOAT32 w0;                 //额定角频率
    FLOAT32 k;                  //
    FLOAT32 ts;                 //运行时间间隔

    FLOAT32 alpha;
    FLOAT32 beta;
    FLOAT32 sinx;               //sin（角度）
    FLOAT32 cosx;               //cos（角度）
    FLOAT32 w;
    FLOAT32 theta;
    FLOAT32 d;
    FLOAT32 q;
    FLOAT32 Abs;                //幅值
    PIController ctrl;
    FLOAT32 deltaTheta;         //角度差，单位弧度
    FLOAT32 deltaThetaDeg;      //角度差，单位度
    FLOAT32 wF;                 //w滤波

    UINT16 zeroFlag;            //过零标志
    faultChk lockState;         //锁相标志 0：锁上  1：没锁上
    
public:
    
    sogiPll(/* args */);
    void run(UINT16 lockFlag, FLOAT32 in);
    void lockChk();
    INLINE_USER void rst();

    INLINE_USER void SetMode(UINT16 arg);                                 //设置工作模式
    
    INLINE_USER void SetK(FLOAT32 arg);
    INLINE_USER void SetTs(FLOAT32 arg);
    INLINE_USER void SetDefaultFreq(FLOAT32 freq);
    INLINE_USER FLOAT32 GetSin();                                         //获取正弦值
    INLINE_USER FLOAT32 GetCos();                                         //获取余弦值
    INLINE_USER FLOAT32 GetW();
    INLINE_USER FLOAT32 GetFreq();                                //获取当前频率
    INLINE_USER UINT16 GetLockState();
    INLINE_USER void ClrFlag();                                          //清除过零标志
    INLINE_USER UINT16 GetZeroFlag();                                 //获取过零标志
    INLINE_USER FLOAT32 GetTheta();
};

INLINE_USER void sogiPll::rst()
{
    alpha = 0;
    beta = 0;
    ctrl.rst();
    w = w0;
}
INLINE_USER void sogiPll::SetMode(UINT16 arg) 
{ 
    mode = arg;
}
INLINE_USER void sogiPll::SetK(FLOAT32 arg)
{
    k = arg;
}

INLINE_USER void sogiPll::SetTs(FLOAT32 arg)
{
    ts = arg;
}
INLINE_USER void sogiPll::SetDefaultFreq(FLOAT32 freq) 
{
    w0 = freq * FREQ_2_W;
}
INLINE_USER FLOAT32 sogiPll::GetSin() 
{
    return sinx;
}
INLINE_USER FLOAT32 sogiPll::GetCos() 
{
    return cosx;
}
INLINE_USER FLOAT32 sogiPll::GetW() 
{
    return wF;
}
INLINE_USER FLOAT32 sogiPll::GetFreq() 
{
    return wF * W_2_FREQ;
}
INLINE_USER UINT16 sogiPll::GetLockState() 
{
    return lockState.GetState();
}
INLINE_USER void sogiPll::ClrFlag() 
{
    zeroFlag = 0;
}

INLINE_USER UINT16 sogiPll::GetZeroFlag() 
{
    return zeroFlag;
}
INLINE_USER FLOAT32 sogiPll::GetTheta()
{
    return theta;
}

#endif
