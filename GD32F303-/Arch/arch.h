#ifndef _ARCH_H_
#define _ARCH_H_

#ifdef __cplusplus
extern "C"
{
#endif


#include "usertypes.h"
#include "gd32f30x_libopt.h"
#include "func.h"
#include "arm_math.h"
#include "config.h"
#include <math.h>

extern const FLOAT32 tbl[][4];
#define ARCH_ISR_TIMER_16
#define LEN (PI / 2.0f)
#define ONE_DIV_LEN	(1.0f / LEN)

//函数声明
extern void InitBSP();
extern void InitPWM();
extern void InitADC();
extern void InitGPIO();
extern void WakeBMUStart();
extern void WakeBMUEnd();
extern void InformBMUStart();
extern void InformBMUEnd();
extern void ARCH_FlashHalfWord(UINT32 addr, UINT16 *src, UINT16 len);

void DMA0_Channel0_IRQHandler(void);
extern void JumpTo(uint32_t Addr);

//ywq_modify:MPPT项目配置修改

INLINE_USER void ARCH_RS485_1_Ctrl(UINT16 onoff)
{
    if(onoff)
    {
        gpio_bit_set(GPIOA, GPIO_PIN_15);
    }
    else
    {
        gpio_bit_reset(GPIOA, GPIO_PIN_15);
    }
}


//PC0用在输出2电流检测
//INLINE_USER void ARCH_FanCtrl(UINT16 onoff)
//{
//    if(onoff)
//    {
//        gpio_bit_set(GPIOC, GPIO_PIN_0);
//    }
//    else
//    {
//        gpio_bit_reset(GPIOC, GPIO_PIN_0);
//    }
//}

INLINE_USER UINT16 ARCH_LedToggle()
{
    static UINT16 ledValue = 0;
    if(ledValue)
    {
        gpio_bit_set(GPIOA, GPIO_PIN_0);
    }
    else
    {
        gpio_bit_reset(GPIOA, GPIO_PIN_0);
    }
    ledValue  = !ledValue ;
    
    return ledValue;
}


INLINE_USER void ARCH_TestToggle()
{
    static UINT16 testValue = 0;
    if(testValue)
    {
        gpio_bit_set(GPIOC, GPIO_PIN_13);
    }
    else
    {
        gpio_bit_reset(GPIOC, GPIO_PIN_13);
    }
    testValue = !testValue;
}

//获取过流信号 电池-PB2; PV1-PB11 ; PV2-PB10
INLINE_USER UINT16 ARCH_GetChargeCurrOv()
{
    // return 0;
    return gpio_input_bit_get(GPIOB, GPIO_PIN_2) | gpio_input_bit_get(GPIOB, GPIO_PIN_10) | gpio_input_bit_get(GPIOB, GPIO_PIN_11);
}
//获取开关机信号
INLINE_USER UINT16 ARCH_GetOnOff()
{
//    return 0;
    return gpio_input_bit_get(GPIOB, GPIO_PIN_5);
}


INLINE_USER UINT16 ARCH_GetCurrTick()
{
    return TIMER_CNT(TIMER5);
}

/*****
 * onoff   0：关闭
 *         1：逆变
 *         2：整流
 * cmp     比较值
*/
//FORCE_INLINE_USER void ARCH_SetInvPWM(UINT16 onoff, INT16 cmp)
//{
//
//	 
//}

FORCE_INLINE_USER void ARCH_SetBuckPWM(UINT16 onoff, INT16 cmp)
{
    
    if(onoff)
    {
        TIMER_CH1CV(TIMER0) = (UINT32)(cmp);
		TIMER_CHCTL2(TIMER0) |= (UINT32)TIMER_CHCTL2_CH1EN;
        TIMER_CHCTL2(TIMER0) |= (UINT32)TIMER_CHCTL2_CH1NEN;  //互补输出
    }
    else
    {
        TIMER_CHCTL2(TIMER0) &= (~(UINT32)TIMER_CHCTL2_CH1EN);
        TIMER_CHCTL2(TIMER0) &= (~(UINT32)TIMER_CHCTL2_CH1NEN);

		TIMER_CH1CV(TIMER0) = (UINT32)(0);
        
    }
}

FORCE_INLINE_USER void ARCH_SetBuck2PWM(UINT16 onoff, INT16 cmp)
{
    
    if(onoff)
    {
        TIMER_CH1CV(TIMER7) = (UINT32)(cmp);
		TIMER_CHCTL2(TIMER7) |= (UINT32)TIMER_CHCTL2_CH1EN;
        TIMER_CHCTL2(TIMER7) |= (UINT32)TIMER_CHCTL2_CH1NEN;  
    }
    else
    {
        TIMER_CHCTL2(TIMER7) &= (~(UINT32)TIMER_CHCTL2_CH1EN);
        TIMER_CHCTL2(TIMER7) &= (~(UINT32)TIMER_CHCTL2_CH1NEN);

		TIMER_CH1CV(TIMER7) = (UINT32)(0);
        
    }
}

FORCE_INLINE_USER void ARCH_SetBoostPWM(UINT16 onoff, INT16 cmp)
{
  
    if(onoff == 1)
    {
		cmp = BUCK_BOOST_PWM_PRD - cmp;
		UpDownLimit(cmp,BUCK_BOOST_PWM_PRD,0);
        TIMER_CH0CV(TIMER0) = (UINT32)cmp;
        TIMER_CHCTL2(TIMER0) |= (UINT32)TIMER_CHCTL2_CH0EN;
        TIMER_CHCTL2(TIMER0) |= (UINT32)TIMER_CHCTL2_CH0NEN;
		
    }
	else if(onoff == 2) //关闭boost反相驱动
    {
		cmp = BUCK_BOOST_PWM_PRD +DEADTIME * 12 / 200 - cmp;
		UpDownLimit(cmp,BUCK_BOOST_PWM_PRD,0);
        TIMER_CH0CV(TIMER0) = (UINT32)cmp;
        TIMER_CHCTL2(TIMER0) |= (UINT32)TIMER_CHCTL2_CH0EN;
        TIMER_CHCTL2(TIMER0) &= (~(UINT32)TIMER_CHCTL2_CH0NEN); 
		
    }
    else
    {
    	//ywq_modify:MPPT项目配置修改
        
       TIMER_CHCTL2(TIMER0) &= (~(UINT32)TIMER_CHCTL2_CH0EN);
       TIMER_CHCTL2(TIMER0) &= (~(UINT32)TIMER_CHCTL2_CH0NEN); //关闭Boost的反相驱动
		
	   TIMER_CH0CV(TIMER0) = (UINT32)0;
		
    }
}

//FORCE_INLINE_USER void ARCH_SetBoost2PWM(UINT16 onoff, INT16 cmp)
//{
//  
//    if(onoff == 1)
//    {
//		cmp = BUCK_BOOST_PWM_PRD - cmp;
//		UpDownLimit(cmp,BUCK_BOOST_PWM_PRD,0);
//        TIMER_CH0CV(TIMER7) = (UINT32)cmp;
//        TIMER_CHCTL2(TIMER7) |= (UINT32)TIMER_CHCTL2_CH0EN;
//        TIMER_CHCTL2(TIMER7) |= (UINT32)TIMER_CHCTL2_CH0NEN;
//		
//    }
//	else if(onoff == 2) //关闭boost反相驱动
//    {
//		cmp = BUCK_BOOST_PWM_PRD + DEADTIME * 12 / 200 - cmp;
//		UpDownLimit(cmp,BUCK_BOOST_PWM_PRD,0);
//        TIMER_CH0CV(TIMER7) = (UINT32)cmp;
//        TIMER_CHCTL2(TIMER7) |= (UINT32)TIMER_CHCTL2_CH0EN;
//        TIMER_CHCTL2(TIMER7) &= (~(UINT32)TIMER_CHCTL2_CH0NEN); 
//		
//    }
//    else
//    {
//    	//ywq_modify:MPPT项目配置修改
//        
//       TIMER_CHCTL2(TIMER7) &= (~(UINT32)TIMER_CHCTL2_CH0EN);
//       TIMER_CHCTL2(TIMER7) &= (~(UINT32)TIMER_CHCTL2_CH0NEN); //关闭Boost的反相驱动
//		
//	   TIMER_CH0CV(TIMER7) = (UINT32)0;
//		
//    }
//}

/*****
 * onoff   0：关闭
 *         1：开电池侧
 *         2：开母线侧
 * prd     周期值
*/

FORCE_INLINE_USER void ARCH_SetFanPWM(UINT16 onoff, INT16 cmp)
{
    if(onoff)
    {
        TIMER_CHCTL2(TIMER3) |= (UINT32)TIMER_CHCTL2_CH3EN;
        TIMER_CH3CV(TIMER3) = (UINT32)cmp;
    }
    else
    {
        TIMER_CHCTL2(TIMER3) &= (~(UINT32)TIMER_CHCTL2_CH3EN);
        TIMER_CH3CV(TIMER3) = (UINT32)0;
    }
}


FORCE_INLINE_USER void ARCH_SetFan2PWM(UINT16 onoff, INT16 cmp)
{
    if (onoff)
    {
        TIMER_CHCTL2(TIMER3) |= (UINT32)TIMER_CHCTL2_CH2EN;
        TIMER_CH2CV(TIMER3) = (UINT32)cmp;
    }
    else
    {
        TIMER_CHCTL2(TIMER3) &= (~(UINT32)TIMER_CHCTL2_CH2EN);
        TIMER_CH2CV(TIMER3) = (UINT32)0;
    }
}


//FORCE_INLINE_USER void ARCH_DisAll()
//{
//
//    ARCH_SetBuckPWM(0, 0);
//    ARCH_SetBoostPWM(0, 0);
// 
//}

FORCE_INLINE_USER FLOAT32 ARCH_Div(FLOAT32 a, FLOAT32 b)
{
    return a / b;
}
FORCE_INLINE_USER FLOAT32 ARCH_Sin(FLOAT32 x)   //0.5us
{
    return arm_sin_f32(x);
    // return sinf(x);
}


FORCE_INLINE_USER FLOAT32 ARCH_Cos(FLOAT32 x)   //0.5us
{
    return arm_cos_f32(x);
}

//tan  
FORCE_INLINE_USER FLOAT32 ARCH_Tan(FLOAT32 angle)   //0.78us
{
    return tanf(angle);
}



FORCE_INLINE_USER FLOAT32 ARCH_Sqrt(FLOAT32 val)    //0.24us
{
    FLOAT32 ret;
    arm_sqrt_f32(val, &ret);
    return ret;
}


FORCE_INLINE_USER FLOAT32 ARCH_Abs(FLOAT32 val)
{
    return fabsf(val);
}

//开关中断
FORCE_INLINE_USER void ARCH_DisInterrupt()
{
    __disable_irq();
}
FORCE_INLINE_USER void ARCH_EnInterrupt()
{
    __enable_irq();
}

#ifdef __cplusplus
}
#endif
#endif


