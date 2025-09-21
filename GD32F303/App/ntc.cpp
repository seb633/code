#include "ntc.h"

ntc::ntc(/* args */)
{

}
/****************************************************
 * AD采样计算电阻（单位K）
*******************************************************/
FLOAT32 ntc::CalcAdToOhm(FLOAT32 ad)
{
    return ARCH_Div(33.0f * ad, 5.0f * 4096.0f - 3.3f * ad);
}
/****************************************************
 * 电阻换算温度
 * ohm：单位K
*******************************************************/
FLOAT32 ntc::CalcOhmToTemp(FLOAT32 ohm)
{
    if(arc.IsSet() == 0)        //防止空
    {
        return 0;
    }
    else
    {
        return arc.GetValueHalf(ohm);       //从曲线查找
    }
}
/****************************************************
 * AD换算温度
 * ad：采样值
*******************************************************/
FLOAT32 ntc::CalcAdToTemp(FLOAT32 ad)
{
    if(arc.IsSet() == 0)        //防止空
    {
        return 0;
    }
    else
    {
        //ywq_modify:MPPT项目配置修改
        FLOAT32 Vntc = ARCH_Div(3.3f * ad ,4096.0f);
        FLOAT32 den = 3.3f - Vntc;
        UpDownLimit(den, 3.3f, 0.001f);
        FLOAT32 ohm = ARCH_Div(10.0f*Vntc, den);
        
        
        return arc.GetValueHalf(ohm);                                           //从曲线查找
    }
}
/****************************************************
 * 设置曲线
*******************************************************/
void ntc::SetNtcLine(UINT16 n, const FLOAT32 *pp, const line *lp)
{
    arc.SetPara(n, pp, lp);
}
