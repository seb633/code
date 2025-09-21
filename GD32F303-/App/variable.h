#ifndef _VARIABLE_H_
#define  _VARIABLE_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "sample.h"
#include "rated.h"
#include "dataCalc.h"
#include "standard.h"
#include "mppt.h"
#include "buckboost.h"
#include "uartDrv.h"
#include "protocol.h"
#include "piecewise.h"
#include "ntc.h"
#include "flashSave.h"
//#include "invertorComm.h"
#include "worklogic.h"

typedef UINT8 Info[3][16] ;

//全局变量声明
extern sample objSample;
extern rated objRated;
extern dataCalc objDataCalc;
extern standard objDftStd;
extern mppt objMppt;
extern mppt objMppt2;
extern buckboost objBuckBoost;
extern buckboost objBuckBoost2;
extern protocol objProtocol;
extern flashSave objFlashSave;
extern worklogic objWorkLogic;
extern uartPara comm0Para;
extern uartPara comm1Para;//inv通讯
extern const FLOAT32 g_NtcBreakPoint[146];
extern const line g_NtcLine[];
#ifdef __cplusplus
}
#endif
#endif
