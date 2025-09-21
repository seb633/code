#ifndef _APP_TASK_OS_H_
#define _APP_TASK_OS_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"


typedef struct
{
    UINT16 period;
    UINT16 trig;
    UINT16 cnt;
    SemaphoreHandle_t signal; 
}taskPeriod;


extern void TaskCreat();
extern void IsrDeal();

#ifdef __cplusplus
}
#endif
#endif
