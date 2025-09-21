#ifndef _CONFIG_H_
#define _CONFIG_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"

//版本号
#define HwVerLow                (0x00)
#define HwVerMid                (0x01)
#define HwVerHigh               (0x00)
    
#define SwVerLow                (0x00)
#define SwVerMid                (0x00)
#define SwVerHigh               (0x01)
    
#define Suppliers               (0x01)


//芯片时钟配置
#define CPU_FREQ                (120000000UL)                                       //CPU时钟
#define TIMER_FREQ              (CPU_FREQ)                                          //定时器时钟                                          
#define BUCK_BOOST_FREQ         (80000)                                            //buck boost开关频率
#define PWM_FREQ                (20000) 
#define PWM_PRD                 (TIMER_FREQ / PWM_FREQ / 2)                         //PWM周期值
#define BUCK_BOOST_PWM_PRD      (TIMER_FREQ / BUCK_BOOST_FREQ / 2)                         //PWM周期值
#define DEADTIME                (400)              //400ns
#define ONE_DIV_PWM_FREQ        (1.0f / PWM_FREQ)                                   //开关频率倒数，一个开关周期的时间

#define RATE_OP_POWER               (600.0f)                                        //额定功率
#define RATE_OP_VOLT                (1.0f)                                        //额定电压

//采样
#define SAMPLE_COFF                     (3.3f / 4096.0f)
#define SAMPLE_COFF2                    (1.0f / 4096.0f)
#define CHARGE_CURR_GAIN_RECIPROCAL     (1.0f/ 200.0f / 2.71f * 0.0025f)  //充电电流增益的倒数
#define DC_IN_CURR_GAIN_RECIPROCAL      (1.0f/ 100.0f / 2.71f * 0.005f)                             //升降压电流增益的倒数                          
#define DC_IN_VOLT_GAIN_RECIPROCAL      (1.0f/ 10.0f / 400.0f)                             //升降压电压增益的倒数             
#define BATT_VOLT_GAIN_RECIPROCAL       (1.0f/ 10.0f / 500.0f)                   //电池电压增益的倒数
#define BATT_CURR_GAIN_RECIPROCAL       (1.0f/0.08333333333f)					//ywq_modify:MPPT项目配置修改
#define MAIN_CURR_GAIN_RECIPROCAL       (1.0f / 200.0f / 2.71f * 0.005f)
#define MAIN_VOLT_GAIN_RECIPROCAL       (1.0f/ 20.0f / 200.0f)                             //主板供电电压采样


#define NTC_NO_CONNECT                  (3085)
#define NTC_SHORT                       (100)

#define CALIBRATE_CONSTANT              (4096)
#define CALIBRATE_MIN                   (3768)      //3890
#define CALIBRATE_MAX                   (4424)      //4300

//PV配置
#define PV_MAX_POWER                    (600.0f)
#define PV_CURR_OV_POINT                (15.0f)                             //PV过流保护点
#define PV_CHK_VOLT_DROP				(1.0f) //PV可识别电压掉落
#define DC_CHK_VOLT_DROP				(2.0f) //DC防拉死电压掉落
#define PV_DUTY_MAX                     (1.6f)
#define DC_MODE_MIN_CURR                (1.0f)
#define DC_MODE_CURR_STEP               (0.5f)
#define DC_MODE_VOLT_1                  (8.5f) //直流源电压范围
#define DC_MODE_VOLT_2                  (90.5f)
#define DC_IN_VOLT_LOW_RET_POINT        (11.0f)
#define DC_IN_VOLT_LOW_POINT            (9.0f)
#define DC_IN_VOLT_HIGH_RET_POINT       (75.0f)
#define DC_IN_VOLT_HIGH_POINT           (80.0f)
#define DC_IN_IPV_SOFT_VAL				(0.002f)
#define CAR_VOLT_LOW1               	(10.0f)
#define CAR_VOLT_HIGH1					(16.5f)
#define CAR_VOLT_LOW2               	(22.5f)
#define CAR_VOLT_HIGH2					(32.0f)

//电池配置
#define BATT_HIGH_POINT                     (57.5f)
#define BATT_LOW_POINT                      (35.0f)
#define RLOAD_VOLT                          (30.0f)
#define BATT_CHARGE_CURR_LMT                (17.5f)
#define BATT_DISCHARGE_CURR_LMT             (50.0f)
#define CHARGE_VOLT_FAST_OFF                (63.0f)
#define CHARGE_VOLT_MAX                     (58.0f)
#define CHARGE_VOLT_MIN                     (25.0f)
#define PV_MAX_CURR_1                       (12.0f)
#define PV_MAX_CURR_CAR                      (8.0f)
#define PV_MAX_CURR_MPPT_1                  (15.0f)
#define PV_MAX_CURR_MPPT_2                  (10.0f)
#define BATT_CURR_MAX					    (24.0f)
#define BATT_CURR_OV_POINT                  (25.0f)
#define BATT_CURR_OV_RET_POINT              (22.0f)
#define LOW_NOISE_CHARGE_WATT               (100.0f)
#define BATT_CURR_MIN_MOS					(1.5f)

//风扇
#define FAN_FREQ                    (25000)
#define FAN_PRD                     (TIMER_FREQ / FAN_FREQ)

//FLASH
#define APP_INFO                (0x8006000)
#define RAM_SHARE_ADDR          (0x2000BF00)
#define BOOT_ADDR               (0x8000000)
#define CALIBRATE_DATA_ADDR     (0x803F800)


#ifdef __cplusplus
}
#endif

#endif

