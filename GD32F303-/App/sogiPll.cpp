#include "sogiPll.h"
#include "config.h"
#include "func.h"
#include <math.h>
#include "arch.h"

sogiPll::sogiPll(/* args */)
{
    FLOAT32 kp, ki;

    mode = 1;
    SetDefaultFreq(50.0f);
    k = 0.8f;
    ts = ONE_DIV_PWM_FREQ;

    theta = 0;
    sinx = 0.0f;
    cosx = 1.0f;
    
    kp = 120;
    ki = kp * kp / 4 / 0.7f / 0.7f;
    ctrl.SetPara(kp, ki * ts, (20.0f * 2 * PI), (-20.0f * 2 * PI));
    rst();
}

void sogiPll::run(UINT16 lockFlag, FLOAT32 in)
{
    FLOAT32 tmpAlpha, tmpBeta;

    if(mode == 0)                                                   //额定频率运行不锁
    {
        rst();
        deltaTheta = 0;                                             //没有相位差
    }
    else if(lockFlag == 0)                                              //电压低不锁，
    {
        rst();
        deltaTheta = 1.57f;                                             //最大相位差
    }
    else
    {
        tmpAlpha = alpha + w * ((in - alpha) * k - beta) * ts;
        tmpBeta = alpha * w * ts + beta;
        alpha = tmpAlpha;
        beta = tmpBeta;

        Albe_2_Dq(&d, &q, alpha, beta, sinx, cosx);

        Abs = ARCH_Sqrt(d * d + q * q);
        UpDownLimit(Abs, 2.0f, 0.1f);
        deltaTheta = ARCH_Div(d, Abs);
        deltaThetaDeg = deltaTheta * RAD_2_DEGREE;

        ctrl.run(deltaTheta);
        w = w0 + ctrl.out;

    }

    theta += w * ts;
    if(theta > PI_x2)
    {
        theta -= PI_x2;
        zeroFlag = 1;
    }


    sinx = ARCH_Sin(theta);
    cosx = ARCH_Cos(theta);
    wF = Filter(w, 0.00157f, wF);                                    //5Hz滤波
    deltaThetaDeg = deltaTheta * RAD_2_DEGREE;         

}
/****************************************************************
 * 是否锁相成功判断
 * **************************************************************/
void sogiPll::lockChk()
{
    const faultChkRetType para = {14.0f, 5.0f, 200, 10, 1};
    FLOAT32 err;
    err = ARCH_Abs(deltaThetaDeg);
    lockState.RetChkHigh(err, (faultChkRetType *)&para);
}


