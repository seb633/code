#ifndef _FAULT_CHK_H_
#define _FAULT_CHK_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"


//回差故障判断参数
struct faultChkRetType
{
    FLOAT32 point;              //故障点
    FLOAT32 retPoint;           //恢复点
    INT16 cntMax;               //计数器最大值
    INT16 inc;                  //大于故障点，增加值
    INT16 dec;                  //小于恢复点，减小值
};

//判断数字量状态
struct faultChkDigitalType
{
    INT32 cntMax;               //计数器最大值
    INT16 inc;                  //大于故障点，增加值
    INT16 dec;                  //小于恢复点，减小值
};


class faultChk
{
private:
    UINT16 state;
    INT32 cnt;
public:
    faultChk();
    void Init(UINT16 stateSet, INT16 cntSet);

    INLINE_USER void RetChkHigh(FLOAT32 in, faultChkRetType *para);
    INLINE_USER void RetChkLow(FLOAT32 in, faultChkRetType *para);
    INLINE_USER void DigitalChkState(UINT16 in, faultChkDigitalType *para);

    INLINE_USER void MultiRateChk(INT16 incDec, INT16 max);

    INLINE_USER UINT16 GetState();
};


/*********************************************************
 * 大于故障判断
 * in：需要判断的模拟量
 * para：回差判断参数
 * *****************************************************/
INLINE_USER void faultChk::RetChkHigh(FLOAT32 in, faultChkRetType *para)
{
    if(in > para->point)
    {
        cnt += para->inc;
        if(cnt >= para->cntMax)
        {
            cnt = para->cntMax;
            state = 1;
        }
    }
    else if(in < para->retPoint)
    {
        cnt -= para->dec;
        if(cnt <= 0)
        {
            cnt = 0;
            state = 0;
        }
    }
}
/*********************************************************
 * 小于故障判断
 * in：需要判断的模拟量
 * para：回差判断参数
 * *****************************************************/
INLINE_USER void faultChk::RetChkLow(FLOAT32 in, faultChkRetType *para)
{
    if(in < para->point)
    {
        cnt += para->inc;
        if(cnt >= para->cntMax)
        {
            cnt = para->cntMax;
            state = 1;
        }
    }
    else if(in > para->retPoint)
    {
        cnt -= para->dec;
        if(cnt <= 0)
        {
            cnt = 0;
            state = 0;
        }
    }
}
/*********************************************************
 * 数字量故障判断
 * in：需要判断的数字量
 * para：参数
 * *****************************************************/
INLINE_USER void faultChk::DigitalChkState(UINT16 in, faultChkDigitalType *para)
{
    if(in)
    {
        cnt += para->inc;
        if(cnt >= para->cntMax)
        {
            cnt = para->cntMax;
            state = 1;
        }
    }
    else
    {
        cnt -= para->dec;
        if(cnt <= 0)
        {
            cnt = 0;
            state = 0;
        }
    }
    
}
/*********************************************************
 * 多速率故障判断
 * incDec：当前模拟量对应的速率
 * max：最大计数
 * *****************************************************/
INLINE_USER void faultChk::MultiRateChk(INT16 incDec, INT16 max)
{
    cnt += incDec;

    if(cnt >= max)
    {
        cnt = max;
        state = 1;
    }
    else if(cnt <= 0)
    {
        cnt = 0;
        state = 0;
    }
}

INLINE_USER UINT16 faultChk::GetState() 
{
    return state;
}


#ifdef __cplusplus
}
#endif
#endif

