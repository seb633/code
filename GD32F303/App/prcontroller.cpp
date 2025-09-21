#include "prcontroller.h"
#include "arch.h"
#include "config.h"


prcontroller::prcontroller(/* args */)
{
//    CalcCoff2(1, 50.0f * FREQ_2_W, PWM_FREQ);            //默认参数
    rst();                                                  //清0
}

/****************************************************************
 * 计算系数（准谐振）
 * **************************************************************/
void prcontroller::CalcCoff(FLOAT32 kr, FLOAT32 w0, FLOAT32 fs, FLOAT32 wc)
{
    FLOAT32 temp1, temp2, wo_adjusted;
    FLOAT32 fenmu;

    wo_adjusted = 2.0f * fs * ARCH_Tan(ARCH_Div(w0, 2.0f*fs));              //频率预畸变

    temp1 = 4.0f*fs*fs + wo_adjusted*wo_adjusted + 4.0f*fs*wc;
    fenmu = ARCH_Div(1.0f, temp1);

    temp2 = 4.0f*kr*wc*fs * fenmu;


    ARCH_DisInterrupt();                                                    //关中断

    b[0] = temp2;
    b[1] = 0;
    b[2] = -temp2;
    a[1]= (-8.0f*fs*fs + 2.0f*wo_adjusted*wo_adjusted)  * fenmu;
    a[2] = (temp1 - 8.0f*fs*wc) * fenmu;

    ARCH_EnInterrupt();
}

/****************************************************************
 * 计算系数（谐振）
 * **************************************************************/
void prcontroller::CalcCoff2(FLOAT32 kr, FLOAT32 w0, FLOAT32 fs)
{
    //使用ARCH_Div,计算出来的精度不够，产生频率偏移，效果不好
#if 0
    FLOAT32 temp1, temp2, wo_adjusted;
    FLOAT32 fenmu;

    wo_adjusted = 2.0f * fs * ARCH_Tan(ARCH_Div(w0, 2.0f*fs));              //频率预畸变

    temp1 = 4.0f*fs*fs + wo_adjusted * wo_adjusted;
    fenmu = ARCH_Div(1.0f, temp1);

    temp2 = 4.0f*kr*fs * fenmu;


    ARCH_DisInterrupt();                                                    //关中断

    b[0] = temp2;
    b[1] = 0;
    b[2] = -temp2;
    a[1]= (-8.0f*fs*fs + 2.0f*wo_adjusted*wo_adjusted)  * fenmu;
    a[2] = 1;

    ARCH_EnInterrupt();
#else
    FLOAT32 temp1, temp2, wo_adjusted;
    FLOAT32 fenmu;

    wo_adjusted = 2.0f * fs * ARCH_Tan(w0 / (2.0f*fs));              //频率预畸变

    temp1 = 4.0f*fs*fs + wo_adjusted * wo_adjusted;
    fenmu = 1.0f / temp1;

    temp2 = 4.0f*kr*fs * fenmu;


    ARCH_DisInterrupt();                                                    //关中断

    b[0] = temp2;
    b[1] = 0;
    b[2] = -temp2;
    a[1]= (-8.0f*fs*fs + 2.0f*wo_adjusted*wo_adjusted)  * fenmu;
    a[2] = 1;

    ARCH_EnInterrupt();
#endif
}


/****************************************************************
 * 计算
 * **************************************************************/
FLOAT32 prcontroller::run(FLOAT32 err)
{
    in[0] = err;

    out[0] = b[0] * in[0] + b[1] * in[1] + b[2] * in[2] - a[1] * out[1] - a[2] * out[2];

    out[2] = out[1];
    out[1] = out[0];

    in[2] = in[1];
    in[1] = in[0];

    return out[0];
}

