#ifndef _ISR_TIME_H_
#define _ISR_TIME_H_
#ifdef __cplusplus
extern "C"
{
#endif


#include "usertypes.h"
#include "arch.h"

class isrtime
{
private:
    FLOAT32 scale;
#ifdef ARCH_ISR_TIMER_16
    UINT16 inTime;              //进入中断时间点
    UINT16 outTime;             //退出中断时间点
    UINT16 runTime;                //中断时间
#else
    UINT32 inTime;              //进入中断时间点
    UINT32 outTime;             //退出中断时间点
    UINT32 runTime;                //中断时间
#endif    
    UINT32 timeSum;             //和
    UINT16 timeCnt;             //累加个数
    UINT32 timeAvg;             //平均值
    UINT32 timeMax;             //最大值
    UINT16 avgUs;
    UINT16 maxUs;
public:
    isrtime(/* args */);

    void calc1();
    void calc2();
    void rst();
    void SetScale(FLOAT32 val) {scale = val;};
    
#ifdef ARCH_ISR_TIMER_16
    INLINE_USER void SetInTime(UINT16 data) {inTime = data;};
    INLINE_USER void SetOutTime(UINT16 data) {outTime = data;};
#else
    INLINE_USER void SetInTime(UINT32 data) {inTime = data;};
    INLINE_USER void SetOutTime(UINT32 data) {outTime = data;};
#endif    


};




#ifdef __cplusplus
}
#endif
#endif
