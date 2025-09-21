#include "PIController.h"

PIController::PIController()
{
	err = 0;
    kp = 0;
    ki = 0;
    integMax = 0;
    integMin = 0;
    integ = 0;
    out = 0;
	rcFilter = 1.0f;
}

/***************************************
 * 设置参数
 * ************************************/
void PIController::SetPara(FLOAT32 kpIn, FLOAT32 kiIn, FLOAT32 max, FLOAT32 min,FLOAT32 rcFilterIn)
{
    kp = kpIn;
    ki = kiIn;
    integMax = max;
    integMin = min;
	
	rcFilter = rcFilterIn;
}

void PIController::SetMax(FLOAT32 max)
{
    integMax = max;
}
void PIController::SetMin(FLOAT32 min)
{
    integMin = min;
}
void PIController::SetInteg(FLOAT32 Integ)
{
    integ = Integ;
}
void PIController::SetKi(FLOAT32 kiIn)
{
    ki = kiIn;
}
FLOAT32 PIController::GetInteg( )
{
    return integ ;
}
