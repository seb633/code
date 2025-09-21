#include "arch.h"
#include "variable.h"
#include "isrtime.h"
#include "config.h"
#include "appTaskOs.h"

#define S_H     (35)            //采样保持时间，每个采样时间 （35 + 13）/60 = 0.8us
#define S_H_L     (45)            //采样保持时间，每个采样时间 （35 + 13）/60 = 0.8us

isrtime objIsrTimeStat;


const FLOAT32 tbl[][4] = 
{
    {1.0f, -1.0f/6.0f, 1.0f/120.0f, -1.0f/5040.0f},
    {1.0f, -1.0f/2.0f, 1.0f/24.00f, -1.0f/720.0f},
    {-1.0f, 1.0f/6.0f, -1.0f/120.0f, 1.0f/5040.0f},
    {-1.0f, 1.0f/2.0f, -1.0f/24.00f, 1.0f/720.0f}
};
FLOAT32 tbl2[] = {1.0f, 1.0f, 1.0f/2.0f, 1.0f/6.0f, 1.0f/24.00f, 1.0f/120.0f, 1.0f/720.0f, 1.0f/5040.0f};
/******************
初始化时钟等
********************/
void InitBSP(void)
{
    timer_parameter_struct setTimer;


    rcu_periph_clock_enable(RCU_AF);                                //引脚重映射时钟打开
    nvic_priority_group_set(NVIC_PRIGROUP_PRE2_SUB2);               //设置NVIC优先级分组

//Timer5 ;不知道哪里用到？？？
    rcu_periph_clock_enable(RCU_TIMER5);                                                    //给时钟
    timer_deinit(TIMER5);                                                                   //复位
    setTimer.prescaler = 0U;                                                                //计数时钟不分频
    setTimer.alignedmode = TIMER_COUNTER_EDGE;                                              //边沿对齐
    setTimer.counterdirection = TIMER_COUNTER_UP;                                           //向上计数
    setTimer.clockdivision = TIMER_CKDIV_DIV1;                                              //DTS时钟不分频
    setTimer.period = 0xffff;                                                               //周期值
    setTimer.repetitioncounter = 0;                                                         //重复
    timer_init(TIMER5, &setTimer);                                                          //初始化

    timer_counter_value_config(TIMER5, 0);                                                  //计数值=0
    timer_enable(TIMER5);               //开始

    objIsrTimeStat.SetScale(0.833f);
}

void InitPWM()
{
    timer_parameter_struct setTimer;
    timer_oc_parameter_struct setChannal;
    timer_break_parameter_struct setBreak;

//Timer1；不知道哪里用到了？？
    rcu_periph_clock_enable(RCU_TIMER1);                                                    //给时钟
    timer_deinit(TIMER1);                                                                   //复位
    setTimer.prescaler = 0U;                                                                //计数时钟不分频
    setTimer.alignedmode = TIMER_COUNTER_EDGE;                                              //边沿对齐
    setTimer.counterdirection = TIMER_COUNTER_UP;                                           //向上计数
    setTimer.clockdivision = TIMER_CKDIV_DIV1;                                              //DTS时钟不分频
    setTimer.period = TIMER_FREQ / PWM_FREQ - 1;                                            //周期值
    setTimer.repetitioncounter = 0;                                                         //重复
    timer_init(TIMER1, &setTimer);                                                          //初始化

    timer_update_event_enable(TIMER1);                                                      //使能更新
    timer_update_source_config(TIMER1, TIMER_UPDATE_SRC_REGULAR);                           //溢出时产生更新
    timer_counter_value_config(TIMER1, 0);                                                  //计数值=0
    timer_master_output_trigger_source_select(TIMER1, TIMER_TRI_OUT_SRC_UPDATE);            //更新事件作为TRGO

    setChannal.outputstate = TIMER_CCX_ENABLE;                                              //输出使能
    setChannal.outputnstate = TIMER_CCXN_DISABLE;                                           //互补输出禁止
    setChannal.ocpolarity = TIMER_OC_POLARITY_HIGH;                                         //高电平有效
    setChannal.ocnpolarity = TIMER_OCN_POLARITY_HIGH;                                       //高电平有效
    setChannal.ocidlestate = TIMER_OC_IDLE_STATE_LOW;                                       //空闲低电平
    setChannal.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;                                     //空闲低电平
    timer_channel_output_config(TIMER1, TIMER_CH_1, &setChannal);                           //设置通道0
    timer_channel_output_config(TIMER1, TIMER_CH_2, &setChannal);                           //设置通道0


    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_1, 200);                       //设置通道0比较值
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_2, 200);                       //设置通道0比较值

    timer_channel_output_mode_config(TIMER1, TIMER_CH_1, TIMER_OC_MODE_PWM1);               //输出PWM1模式
    timer_channel_output_mode_config(TIMER1, TIMER_CH_2, TIMER_OC_MODE_PWM1);               //输出PWM1模式



//Timer0;   BUCK-BOOST驱动
    rcu_periph_clock_enable(RCU_TIMER0);
    timer_deinit(TIMER0);
    setTimer.prescaler = 0U;                                                                //计数时钟不分频
    setTimer.alignedmode = TIMER_COUNTER_CENTER_UP;                                         //中心对齐，向上计数匹配时置位
    setTimer.counterdirection = TIMER_COUNTER_UP;                                           //向上计数
    setTimer.clockdivision = TIMER_CKDIV_DIV1;                                              //DTS时钟不分频

    setTimer.period = BUCK_BOOST_PWM_PRD; 
    setTimer.repetitioncounter = 0;                                                         //重复
    timer_init(TIMER0, &setTimer);                                                          //初始化

    timer_counter_value_config(TIMER0, 0);                                                  //计数值=0

    timer_master_slave_mode_config(TIMER0, TIMER_MASTER_SLAVE_MODE_ENABLE);                 //配置为从模式
    timer_slave_mode_select(TIMER0, TIMER_SLAVE_MODE_RESTART);                              //从模式，触发信号复位定时器
    timer_input_trigger_source_select(TIMER0, TIMER_SMCFG_TRGSEL_ITI1);                     //触发信号选择

    setChannal.outputstate = TIMER_CCX_DISABLE;                                             //输出禁止
    setChannal.outputnstate = TIMER_CCXN_DISABLE;                                           //互补输出禁止
    setChannal.ocpolarity = TIMER_OC_POLARITY_HIGH;                                         //高电平有效
    setChannal.ocnpolarity = TIMER_OCN_POLARITY_HIGH;                                       //高电平有效
    setChannal.ocidlestate = TIMER_OC_IDLE_STATE_LOW;                                       //空闲低电平
    setChannal.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;                                     //空闲低电平
    timer_channel_output_config(TIMER0, TIMER_CH_0, &setChannal);                           //设置通道0
    timer_channel_output_config(TIMER0, TIMER_CH_1, &setChannal);                           //设置通道1
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_0, 1000);                       //设置通道0比较值
    timer_channel_output_pulse_value_config(TIMER0, TIMER_CH_1, 1000);                       //设置通道1比较值
    timer_channel_output_fast_config(TIMER0, TIMER_CH_0, TIMER_OC_FAST_ENABLE);             //快速输出模式
    timer_channel_output_fast_config(TIMER0, TIMER_CH_1, TIMER_OC_FAST_ENABLE);             //快速输出模式
    timer_channel_output_mode_config(TIMER0, TIMER_CH_0, TIMER_OC_MODE_PWM1);               //输出PWM0模式
    timer_channel_output_mode_config(TIMER0, TIMER_CH_1, TIMER_OC_MODE_PWM0);               //输出PWM0模式 //ywq_modify:PWM0
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);         //比较值影子寄存器
    timer_channel_output_shadow_config(TIMER0, TIMER_CH_1, TIMER_OC_SHADOW_ENABLE);         //比较值影子寄存器

    setBreak.runoffstate = TIMER_ROS_STATE_ENABLE;
    setBreak.ideloffstate = TIMER_IOS_STATE_DISABLE;                                                   
	setBreak.deadtime = DEADTIME * 12/100; //ywq_modify:MPPT项目配置修改
    setBreak.breakpolarity = TIMER_BREAK_POLARITY_LOW;
    setBreak.outputautostate = TIMER_OUTAUTO_ENABLE;
    setBreak.protectmode = TIMER_CCHP_PROT_OFF;
    setBreak.breakstate = TIMER_BREAK_DISABLE;
    timer_break_config(TIMER0, &setBreak);                                                  //死区

    //引脚
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_8);
    gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_13);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_14);
	
	
	//Timer7;BUCK电路
	rcu_periph_clock_enable(RCU_TIMER7);
    timer_deinit(TIMER7);
    setTimer.prescaler = 0U;                                                                //计数时钟不分频
    setTimer.alignedmode = TIMER_COUNTER_CENTER_UP;                                         //中心对齐，向上计数匹配时置位
    setTimer.counterdirection = TIMER_COUNTER_UP;                                           //向上计数
    setTimer.clockdivision = TIMER_CKDIV_DIV1;                                              //DTS时钟不分频
    setTimer.period = BUCK_BOOST_PWM_PRD; 
    setTimer.repetitioncounter = 0;                                                         //重复
    timer_init(TIMER7, &setTimer);                                                          //初始化

    timer_counter_value_config(TIMER7, 0);                                                  //计数值=0

    timer_master_slave_mode_config(TIMER7, TIMER_MASTER_SLAVE_MODE_ENABLE);                 //配置为从模式
    timer_slave_mode_select(TIMER7, TIMER_SLAVE_MODE_RESTART);                              //从模式，触发信号复位定时器
    timer_input_trigger_source_select(TIMER7, TIMER_SMCFG_TRGSEL_ITI1);                     //触发信号选择

    setChannal.outputstate = TIMER_CCX_DISABLE;                                             //输出禁止
    setChannal.outputnstate = TIMER_CCXN_DISABLE;                                           //互补输出禁止
    setChannal.ocpolarity = TIMER_OC_POLARITY_HIGH;                                         //高电平有效
    setChannal.ocnpolarity = TIMER_OCN_POLARITY_HIGH;                                       //高电平有效
    setChannal.ocidlestate = TIMER_OC_IDLE_STATE_LOW;                                       //空闲低电平
    setChannal.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;                                     //空闲低电平
    //timer_channel_output_config(TIMER7, TIMER_CH_0, &setChannal);                           //设置通道0
    timer_channel_output_config(TIMER7, TIMER_CH_1, &setChannal);                           //设置通道1
    //timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_0, 1000);                       //设置通道0比较值
    timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_1, 1000);                       //设置通道1比较值
    //timer_channel_output_fast_config(TIMER7, TIMER_CH_0, TIMER_OC_FAST_ENABLE);             //快速输出模式
    timer_channel_output_fast_config(TIMER7, TIMER_CH_1, TIMER_OC_FAST_ENABLE);             //快速输出模式
    //timer_channel_output_mode_config(TIMER7, TIMER_CH_0, TIMER_OC_MODE_PWM1);               //输出PWM0模式
    timer_channel_output_mode_config(TIMER7, TIMER_CH_1, TIMER_OC_MODE_PWM0);               //输出PWM0模式 //ywq_modify:PWM0
    //timer_channel_output_shadow_config(TIMER7, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);         //比较值影子寄存器
    timer_channel_output_shadow_config(TIMER7, TIMER_CH_1, TIMER_OC_SHADOW_ENABLE);         //比较值影子寄存器

    setBreak.runoffstate = TIMER_ROS_STATE_ENABLE;
    setBreak.ideloffstate = TIMER_IOS_STATE_DISABLE;                                                   
	setBreak.deadtime = DEADTIME * 12/100; //ywq_modify:MPPT项目配置修改
    setBreak.breakpolarity = TIMER_BREAK_POLARITY_LOW;
    setBreak.outputautostate = TIMER_OUTAUTO_ENABLE;
    setBreak.protectmode = TIMER_CCHP_PROT_OFF;
    setBreak.breakstate = TIMER_BREAK_DISABLE;
    timer_break_config(TIMER7, &setBreak);                                                  //死区

    //引脚
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    gpio_init(GPIOC, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_7);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_MAX, GPIO_PIN_0);


    //Timer3  风扇调速       
    rcu_periph_clock_enable(RCU_TIMER3);                                                    //给时钟
    timer_deinit(TIMER3);                                                                   //复位
    setTimer.prescaler = 0U;                                                                //计数时钟不分频
    setTimer.alignedmode = TIMER_COUNTER_EDGE;                                              //边沿对齐
    setTimer.counterdirection = TIMER_COUNTER_UP;                                           //向上计数
    setTimer.clockdivision = TIMER_CKDIV_DIV1;                                              //DTS时钟不分频
    setTimer.period = FAN_PRD;                                                                 //周期值
    setTimer.repetitioncounter = 0;                                                         //重复
    timer_init(TIMER3, &setTimer);                                                          //初始化

    timer_counter_value_config(TIMER3, 0);                                                  //计数值=0

    setChannal.outputstate = TIMER_CCX_DISABLE;                                              //输出使能
    setChannal.outputnstate = TIMER_CCXN_DISABLE;                                           //互补输出禁止
    setChannal.ocpolarity = TIMER_OC_POLARITY_HIGH;                                         //高电平有效
    setChannal.ocnpolarity = TIMER_OCN_POLARITY_HIGH;                                       //高电平有效
    setChannal.ocidlestate = TIMER_OC_IDLE_STATE_LOW;                                       //空闲低电平
    setChannal.ocnidlestate = TIMER_OCN_IDLE_STATE_LOW;                                     //空闲低电平
    timer_channel_output_config(TIMER3, TIMER_CH_2, &setChannal);                           //设置通道
    timer_channel_output_config(TIMER3, TIMER_CH_3, &setChannal);

    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_2, 0);                         //设置通道0比较值
    timer_channel_output_mode_config(TIMER3, TIMER_CH_2, TIMER_OC_MODE_PWM0);               //输出PWM1模式
    timer_channel_output_shadow_config(TIMER3, TIMER_CH_2, TIMER_OC_SHADOW_ENABLE);         //比较值影子寄存器

    timer_channel_output_pulse_value_config(TIMER3, TIMER_CH_3, 0);
    timer_channel_output_mode_config(TIMER3, TIMER_CH_3, TIMER_OC_MODE_PWM0);
    timer_channel_output_shadow_config(TIMER3, TIMER_CH_3, TIMER_OC_SHADOW_ENABLE);

    //引脚
    rcu_periph_clock_enable(RCU_GPIOB);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_8);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_9);


    timer_enable(TIMER0);               //开始
	timer_enable(TIMER1);               //开始
    timer_enable(TIMER3);               //开始
	timer_enable(TIMER7);               //开始
}
UINT16 start, end, time;
volatile FLOAT32 a = 0, b;
FLOAT32 sinxa, cosxa;
UINT16 intBuf[100];
UINT16 index;
struct AdcDataType
{
    UINT16 adc0;
    UINT16 adc1;
};
AdcDataType g_Adc0Buf[5];

void DMA0_Channel0_IRQHandler(void)
{
    if(SET == dma_interrupt_flag_get(DMA0, DMA_CH0, DMA_INT_FLAG_FTF))
    {
        dma_interrupt_flag_clear(DMA0, DMA_CH0, DMA_INT_FLAG_FTF);
        objIsrTimeStat.SetInTime(ARCH_GetCurrTick());

		//objSample.SetDcInputCurrRaw(g_Adc0Buf[4].adc0);	
  //      objSample.SetBattVoltRaw(g_Adc0Buf[1].adc0);
  //      objSample.SetDcInputVoltRaw(g_Adc0Buf[2].adc0);
  //      objSample.SetTemperarute1Raw(g_Adc0Buf[0].adc0);
		//
		//objSample.SetChargeCurrRaw(g_Adc0Buf[0].adc1);
  //      objSample.SetTemperatrue2Raw(g_Adc0Buf[4].adc1);
		
		//第二路BuckBoost
		objSample.SetDcInput2CurrRaw(g_Adc0Buf[3].adc0);
		objSample.SetDcInput2VoltRaw(g_Adc0Buf[3].adc1);
        
        
        //MT600
        objSample.SetDcInputCurrRaw(g_Adc0Buf[4].adc0);
        objSample.SetDcInputVoltRaw(g_Adc0Buf[2].adc0);

        objSample.SetChargeCurrRaw(g_Adc0Buf[0].adc1);
        objSample.SetBattVoltRaw(g_Adc0Buf[1].adc0);

        objSample.SetMainCurrRaw(g_Adc0Buf[3].adc0);
        objSample.SetMainVoltRaw(g_Adc0Buf[3].adc1);

        objSample.SetTemperarute1Raw(g_Adc0Buf[0].adc0);
        objSample.SetTemperatrue2Raw(g_Adc0Buf[4].adc1);





        objSample.OffsetCalibrate();            //没有校准完成，校准，后面不执行

        IsrDeal();

        objIsrTimeStat.SetOutTime(ARCH_GetCurrTick());
        objIsrTimeStat.calc2();


    }

}
void Delay()
{
    UINT32 i;
    for(i=0; i<1000000; i++)
    {
        
    }
}
//ADC_SAMPLETIME_1POINT5
#define ADC_SAMPLE_TIME (ADC_SAMPLETIME_7POINT5)
void InitADC()
{
    dma_parameter_struct setDma;

    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV4);                //ADC时钟
    rcu_periph_clock_enable(RCU_DMA0);                          //给DMA时钟

//DMA
    dma_deinit(DMA0, DMA_CH0);                                  //通道重置
    setDma.periph_addr = (UINT32)(&ADC_RDATA(ADC0));            //外设地址
    setDma.periph_width = DMA_PERIPHERAL_WIDTH_32BIT;           //数据宽度
    setDma.memory_addr = (UINT32)&g_Adc0Buf;                    //内存地址
    setDma.memory_width = DMA_MEMORY_WIDTH_32BIT;
    setDma.number = 5;                                          //传输个数
    setDma.priority = DMA_PRIORITY_ULTRA_HIGH;
    setDma.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    setDma.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    setDma.direction = DMA_PERIPHERAL_TO_MEMORY;
    
    dma_init(DMA0, DMA_CH0, &setDma);
    dma_circulation_enable(DMA0, DMA_CH0);                      //循环模式
    dma_interrupt_enable(DMA0, DMA_CH0, DMA_INT_FTF);           //开启中断
    nvic_irq_enable(DMA0_Channel0_IRQn, 0, 0);                  //使能NVIC

//ADC0
    rcu_periph_clock_enable(RCU_ADC0);                          //给ADC时钟
    rcu_periph_clock_enable(RCU_ADC1);
    adc_deinit(ADC0);                                           //重置
    adc_deinit(ADC1);
    adc_mode_config(ADC_DAUL_REGULAL_PARALLEL);                 //并联模式

    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);                                           //扫描模式
    adc_special_function_config(ADC1, ADC_SCAN_MODE, ENABLE);
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T1_CH1);       //触发源
    adc_external_trigger_source_config(ADC1, ADC_REGULAR_CHANNEL, ADC0_1_2_EXTTRIG_REGULAR_NONE);
    adc_resolution_config(ADC0, ADC_RESOLUTION_12B);                                                    //采样分辨率
    adc_resolution_config(ADC1, ADC_RESOLUTION_12B);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);                                               //右对齐
    adc_data_alignment_config(ADC1, ADC_DATAALIGN_RIGHT);  


    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 5);                                            //采样个数
    adc_channel_length_config(ADC1, ADC_REGULAR_CHANNEL, 5);

    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_14, ADC_SAMPLE_TIME);                         //PC4
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_2, ADC_SAMPLE_TIME);                           //PA2
    adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_3, ADC_SAMPLE_TIME);                            //PA3
    adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_10, ADC_SAMPLE_TIME);                           //PC0
    adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_1, ADC_SAMPLE_TIME);                            //PA1

    adc_regular_channel_config(ADC1, 0, ADC_CHANNEL_4, ADC_SAMPLE_TIME);                            //PA4
    adc_regular_channel_config(ADC1, 1, ADC_CHANNEL_13, ADC_SAMPLE_TIME);                           //PC3
    adc_regular_channel_config(ADC1, 2, ADC_CHANNEL_11, ADC_SAMPLE_TIME);                           //PC1
    adc_regular_channel_config(ADC1, 3, ADC_CHANNEL_12, ADC_SAMPLE_TIME);                           //PC2
    adc_regular_channel_config(ADC1, 4, ADC_CHANNEL_15, ADC_SAMPLE_TIME);                           //PC5

    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);                                     //触发使能
    adc_external_trigger_config(ADC1, ADC_REGULAR_CHANNEL, ENABLE);

    //采样
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_1);               //PA1 PV输入电流
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_2);               //PA2 OUT1(TO_BAT)电压
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_3);               //PA3 PV输入电压
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_0);               //PC0 OUT2电流
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_4);               //PC4 温度采样1
    gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_4);               //PA4 OUT1(TO_BAT)电流
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_2);               //PC2 OUT2电压
    gpio_init(GPIOC, GPIO_MODE_AIN, GPIO_OSPEED_MAX, GPIO_PIN_5);               //PC5 温度2


    adc_enable(ADC0);
    Delay();
    adc_calibration_enable(ADC0);
    adc_enable(ADC1);
    Delay();
    adc_calibration_enable(ADC1);

    adc_dma_mode_enable(ADC0);
    dma_channel_enable(DMA0, DMA_CH0);
}

void InitGPIO()
{
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    
    rcu_periph_clock_enable(RCU_AF);   //打开复用时钟
	gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP,ENABLE);

    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_0);  //LED
    
    //过流检测
	gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_10MHZ, GPIO_PIN_2);                //输出主板电流检测
    gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_10MHZ, GPIO_PIN_10);               //输出电池电流检测
	gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_10MHZ, GPIO_PIN_11);               //PV输入电流检测
     
	//PA15 RS485-1 EN
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_15); //PA15
	ARCH_RS485_1_Ctrl(0);
}
void WakeBMUStart()
{
    
    if(objDataCalc.GetBattVoltAvg() < BATT_LOW_POINT)
    {
        gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, GPIO_PIN_5);
        gpio_bit_set(GPIOB, GPIO_PIN_5);
    }

}
void WakeBMUEnd()
{
    gpio_bit_reset(GPIOB, GPIO_PIN_5);
    gpio_init(GPIOB, GPIO_MODE_IPD, GPIO_OSPEED_10MHZ, GPIO_PIN_5);
}
void InformBMUStart()
{

}
void InformBMUEnd()
{

}
void JumpTo(uint32_t Addr)
{
    nvic_irq_disable(DMA0_Channel0_IRQn); 

    timer_deinit(TIMER0);
    timer_deinit(TIMER1);
    timer_deinit(TIMER2);
    timer_deinit(TIMER3);
    timer_deinit(TIMER4);
    timer_deinit(TIMER5);
    timer_deinit(TIMER7);
    adc_deinit(ADC0);
    adc_deinit(ADC1);
    dma_deinit(DMA0, DMA_CH0);
    dma_deinit(DMA0, DMA_CH3);
    usart_deinit(USART0);

    rcu_periph_clock_disable(RCU_AF);
    rcu_periph_clock_disable(RCU_GPIOA);
    rcu_periph_clock_disable(RCU_GPIOB);
    rcu_periph_clock_disable(RCU_GPIOC);
    rcu_periph_clock_disable(RCU_TIMER0);
    rcu_periph_clock_disable(RCU_TIMER1);
    rcu_periph_clock_disable(RCU_TIMER2);
    rcu_periph_clock_disable(RCU_TIMER3);
    rcu_periph_clock_disable(RCU_TIMER4);
    rcu_periph_clock_disable(RCU_TIMER5);
    rcu_periph_clock_disable(RCU_TIMER7);
    rcu_periph_clock_disable(RCU_DMA0);
    rcu_periph_clock_disable(RCU_ADC0);
    rcu_periph_clock_disable(RCU_ADC1);
    rcu_periph_clock_disable(RCU_USART0);


#if 1
    NVIC_SystemReset();
#else
    Void_Func_Void JumpToApp;

    if ( ( ( * ( __IO uint32_t * ) Addr ) & 0x2FFE0000 ) == 0x20000000 )
    {
        JumpToApp = (Void_Func_Void) * ( __IO uint32_t *)(Addr + 4); 
        __set_MSP( * ( __IO uint32_t * ) Addr ); 
        __set_PSP( * ( __IO uint32_t * ) Addr );                  
        JumpToApp();                                                
    }
#endif
}

void ARCH_FlashHalfWord(UINT32 addr, UINT16 *src, UINT16 len)
{
    UINT16 i;
    UINT16 *ptr;
    UINT32 currAddr;
    
    if(len > 1024)
    {
        return;
    }
    ptr = src;
    currAddr = addr;
    
    ARCH_DisInterrupt();
    
    fmc_unlock();
    fmc_page_erase(addr);
    for(i=0; i<len; i++)
    {
        fmc_halfword_program(currAddr, *ptr);
        ptr++;
        currAddr += 2;
    }
    
    fmc_lock();
    
    ARCH_EnInterrupt();
}




