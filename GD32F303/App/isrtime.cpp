
#include "isrtime.h"

isrtime::isrtime(/* args */)
{
    timeSum = 0;
    timeCnt = 0;
    timeAvg = 0;
    timeMax = 0;
}
/****************************
 * 中断时间统计
 * 向下计数
 * *************************/
void isrtime::calc1()
{
    runTime = (inTime - outTime);                //计算本次中断事件

    timeSum += runTime;
    timeCnt++;
    if(timeCnt >= 16384)
    {
        timeAvg = timeSum >> 14;
        avgUs = (FLOAT32)timeAvg * scale;
        timeSum = 0;
        timeCnt = 0;
    }

    if(runTime > timeMax)
    {
        timeMax = runTime;
        maxUs = (FLOAT32)timeMax * scale;
    }
}
/****************************
 * 中断时间统计
 * 向上计数
 * *************************/
void isrtime::calc2()
{
    runTime = (outTime - inTime);
    
    timeSum += runTime;
    timeCnt++;
    if(timeCnt >= 16384)
    {
        timeAvg = timeSum >> 14;
        avgUs = (FLOAT32)timeAvg * scale;
        timeSum = 0;
        timeCnt = 0;
    }

    if(runTime > timeMax)
    {
        timeMax = runTime;
        maxUs = (FLOAT32)timeMax * scale;
    }
}
/****************************
 * 重新统计最大值
 * *************************/
void isrtime::rst()
{
    timeMax = 0;
}


