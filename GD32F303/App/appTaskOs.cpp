#include "appTaskOs.h"
#include "variable.h"
#include "worklogic.h"
#include "arch.h"
#include "SEGGER_RTT.h"

#define LED_SPEED_100MS			(5)
//任务变量
//空闲任务
#define IDLE_STACK_SIZE         (512)                           //堆栈大小
static StaticTask_t idleTaskTcb;                                //任务控制块
static StackType_t  idleTaskStack[IDLE_STACK_SIZE];             //堆栈

//1ms任务
#define TASK_2MS_STACK_SIZE     (512)                           //堆栈大小
#define TASK_2MS_PRIO           (5)                             //优先级
static StaticTask_t Task2msTcb;                                 //任务控制块
static StackType_t  Task2msStack[TASK_2MS_STACK_SIZE];          //堆栈

//10ms任务
#define TASK_10MS_STACK_SIZE     (512)
#define TASK_10MS_PRIO           (3)
static StaticTask_t Task10msTcb;
static StackType_t  Task10msStack[TASK_10MS_STACK_SIZE];

//100ms任务
#define TASK_100MS_STACK_SIZE     (512)
#define TASK_100MS_PRIO           (2)
static StaticTask_t Task100msTcb;
static StackType_t  Task100msStack[TASK_100MS_STACK_SIZE];

//1s任务
#define TASK_1S_STACK_SIZE     (512)
#define TASK_1S_PRIO           (1)
static StaticTask_t Task1sTcb;
static StackType_t  Task1sStack[TASK_1S_STACK_SIZE];


//初始任务
#define TASK_START_STACK_SIZE     (512)
#define TASK_START_PRIO           (4)
static StaticTask_t TaskStartTcb;
static StackType_t  TaskStartStack[TASK_START_STACK_SIZE];
SemaphoreHandle_t g_SemStartHandle;                             //信号量句柄
StaticSemaphore_t g_SemStartMem;                                //信号量

taskPeriod PeriodTask[] = 
{
    {2, 0, 0, NULL},
    {10, 7, 0, NULL},
    {100, 97, 0, NULL},
    {1000, 997, 0, NULL}
};
#define PERIOD_TASK_NUM (sizeof(PeriodTask)/sizeof(taskPeriod))
StaticSemaphore_t g_SignalPool[PERIOD_TASK_NUM];                        //信号量

worklogic objWorkLogic;
UINT32 counter, timeOff = 2;

void IsrDeal()
{
    objSample.DataDeal();
    objBuckBoost.Regulation(objSample.GetDcInputVolt(),objSample.GetDcInputCurr());
	
   //需要写一个类似的
    //objBuck.Regulation()
    
    objWorkLogic.pSd->FastChk();
    objWorkLogic.FastOff();

	
    if(objWorkLogic.hardFault.bit.battOvCurr)
    {
        ARCH_SetBuckPWM(0, 0);
		ARCH_SetBoostPWM(0, 0);

        ARCH_SetBuck2PWM(0, 0);
		
    }
    else
    {
		ARCH_SetBuckPWM(objBuckBoost.GetOnoff(), objBuckBoost.GetBuckCmp());
		ARCH_SetBoostPWM(objBuckBoost.GetOnoff(), objBuckBoost.GetBoostCmp());	
		
        //Buck电路 需要补充
        //ARCH_SetBuck2PWM();
    }
	
	/*发波
	ARCH_SetBuckPWM(1, 	200);
	ARCH_SetBoostPWM(1, 200);	
	ARCH_SetBuck2PWM(1, 200);
	ARCH_SetBoost2PWM(1, 200);
	*/

    objDataCalc.Add();
    objProtocol.comm.check();
 
}

void Task2ms(void * pvParameters)
{
    for(;;)
    {
        if(xSemaphoreTake( PeriodTask[0].signal, portMAX_DELAY ) == pdTRUE)
        {
            objWorkLogic.FaultCollect();
            objWorkLogic.BuckBoostStateShift();
            objWorkLogic.BMUConmunicate();
			objBuckBoost.IpvSoftStart();
			
            //objBuckBoost2.IpvSoftStart();
            objProtocol.deal();
        }
    }
}
void Task10ms(void * pvParameters)
{
	
    for(;;)
    {
        if(xSemaphoreTake( PeriodTask[1].signal, portMAX_DELAY ) == pdTRUE)
        {
            objWorkLogic.pSd->SlowChk();
            objWorkLogic.TemperatureChk();
			
            objDataCalc.Save();
			objDataCalc.Calc();
        }
    }
}
void Task100ms(void * pvParameters)
{
    /*
	MPPT工况扫描测试
	FLOAT32 Uoc = 70.0f;
    FLOAT32 UpvCtrlMax = Uoc*0.95f,UpvCtrlMin = Uoc*0.40f;
    FLOAT32 UpvCtrl = UpvCtrlMax;
    INT16 UpvChangeDir = -1;
    if(UpvCtrlMax> 80.0f)
    {
        UpvCtrlMax = 80.0f;
    }
    if(UpvCtrlMin < 10.0f)
    {
        UpvCtrlMin = 10.0f;
    }*/
	INT32 LedCnt = 0;
    
    for(;;)
    {
        if(xSemaphoreTake( PeriodTask[2].signal, portMAX_DELAY ) == pdTRUE)
        {
			LedCnt++;
			if(LedCnt >= LED_SPEED_100MS)
			{
				LedCnt = 0;
				ARCH_LedToggle();
			}
            objWorkLogic.WakeBmu();
             /*
			MPPT工况扫描测试
            UpvCtrl += 0.05f*(UpvCtrlMax - UpvCtrlMin) * UpvChangeDir;
            
            if(UpvCtrl > UpvCtrlMax)
            {
                UpvChangeDir = -1;
            }
            if(UpvCtrl < UpvCtrlMin)
            {
                UpvChangeDir = 1;
            }
           
             objBuckBoost.SetVinRef(UpvCtrl);
			*/
        }
    }
}
void Task1s(void * pvParameters)
{ 
    for(;;)
    {
        if(xSemaphoreTake( PeriodTask[3].signal, portMAX_DELAY ) == pdTRUE)
        {
            ARCH_TestToggle();
            objWorkLogic.PowerUpTick();
            
            if(objWorkLogic.GetPowerUpTime() == 2)
            {
                objWorkLogic.SetChargeCmd(1);
            }
        }
    }
}

char JS_RTT_UpBuffer[128];
void TaskStart(void * pvParameters)
{ 
    SEGGER_RTT_ConfigUpBuffer(1,"JScope_i2i2i2i2", &JS_RTT_UpBuffer[0], sizeof(JS_RTT_UpBuffer), 
    SEGGER_RTT_MODE_BLOCK_IF_FIFO_FULL);
    
    objWorkLogic.init();
    InitBSP();
    InitGPIO();
    InitADC();
    InitPWM();
    objFlashSave.Init((void *) CALIBRATE_DATA_ADDR);
    
    objSample.CalibrateInDcVolt(objFlashSave.GetInDcVoltCoff());
    objProtocol.CalibrateCoffInit(0, objFlashSave.GetInDcVoltCoff());

    objSample.CalibrateInDc2Volt(objFlashSave.GetInDc2VoltCoff());
    objProtocol.CalibrateCoffInit(1, objFlashSave.GetInDc2VoltCoff());

    objSample.CalibrateBattVolt(objFlashSave.GetBattVoltCoff());
    objProtocol.CalibrateCoffInit(2, objFlashSave.GetBattVoltCoff());
    
    objSample.CalibrateInDcCurr(objFlashSave.GetInDcCurrCoff());
    objProtocol.CalibrateCoffInit(3, objFlashSave.GetInDcCurrCoff());
    
    objSample.CalibrateInDc2Curr(objFlashSave.GetInDc2CurrCoff());
    objProtocol.CalibrateCoffInit(4, objFlashSave.GetInDc2CurrCoff());
    
    objSample.CalibrateBattCurr(objFlashSave.GetBattCurrCoff());
    objProtocol.CalibrateCoffInit(5, objFlashSave.GetBattCurrCoff());

    xTaskCreateStatic(Task2ms,          // Function that implements the task.
                      "2ms task",       // Text name for the task.
                      TASK_2MS_STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * )NULL,         // Parameter passed into the task.
                      TASK_2MS_PRIO,        // Priority at which the task is created.
                      Task2msStack,         // Array to use as the task's stack.
                      &Task2msTcb );        // Variable to hold the task's data structure.

     xTaskCreateStatic(Task10ms,          // Function that implements the task.
                      "10ms task",       // Text name for the task.
                      TASK_10MS_STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * )NULL,         // Parameter passed into the task.
                      TASK_10MS_PRIO,        // Priority at which the task is created.
                      Task10msStack,         // Array to use as the task's stack.
                      &Task10msTcb );        // Variable to hold the task's data structure.

   xTaskCreateStatic(Task100ms,          // Function that implements the task.
                      "100ms task",       // Text name for the task.
                      TASK_100MS_STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * )NULL,         // Parameter passed into the task.
                      TASK_100MS_PRIO,        // Priority at which the task is created.
                      Task100msStack,         // Array to use as the task's stack.
                      &Task100msTcb );        // Variable to hold the task's data structure.

   xTaskCreateStatic(Task1s,          // Function that implements the task.
                      "1s task",       // Text name for the task.
                      TASK_1S_STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * )NULL,         // Parameter passed into the task.
                      TASK_1S_PRIO,        // Priority at which the task is created.
                      Task1sStack,         // Array to use as the task's stack.
                      &Task1sTcb );        // Variable to hold the task's data structure.

    for(;;)
    {
        if(xSemaphoreTake( g_SemStartHandle, portMAX_DELAY ) == pdTRUE)
        {
            
        }

    }
}

void TaskCreat()
{
    xTaskCreateStatic(TaskStart,          // Function that implements the task.
                      "start task",       // Text name for the task.
                      TASK_START_STACK_SIZE,           // Number of indexes in the xStack array.
                      ( void * )NULL,         // Parameter passed into the task.
                      TASK_START_PRIO,        // Priority at which the task is created.
                      TaskStartStack,         // Array to use as the task's stack.
                      &TaskStartTcb );        // Variable to hold the task's data structure.

    for(UINT16 i=0; i<PERIOD_TASK_NUM; i++)
    {
        PeriodTask[i].signal = xSemaphoreCreateBinaryStatic(&g_SignalPool[i]);
    }
    g_SemStartHandle = xSemaphoreCreateBinaryStatic(&g_SemStartMem);


    vTaskStartScheduler();

}
void vApplicationTickHook()
{
    UINT16 i;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;


    for(i=0; i<PERIOD_TASK_NUM; i++)
    {
        PeriodTask[i].cnt++;
        if(PeriodTask[i].cnt >= PeriodTask[i].period)
        {
            PeriodTask[i].cnt = 0;
        }
        if(PeriodTask[i].cnt == PeriodTask[i].trig)
        {
            xSemaphoreGiveFromISR(PeriodTask[i].signal, &xHigherPriorityTaskWoken);
        }
    }

}

//堆栈溢出处理
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    while(1);
}
//传递空闲任务参数
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
    *ppxIdleTaskTCBBuffer = &idleTaskTcb;
    *ppxIdleTaskStackBuffer = idleTaskStack;
    *pulIdleTaskStackSize = IDLE_STACK_SIZE;
}

