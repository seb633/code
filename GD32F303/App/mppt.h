#ifndef _MPPT_H_
#define _MPPT_H_
#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"
	
enum mppt_status 
{
	GetOpenVoltage = 0,
	FirstUpvChange = 1,
	UpvChangeLeft  = 2,
	UpvChangeRight = 3,
	UpvForceChange = 4,
	PowerLimit     = 5,
};

class mppt
{

private:	
	FLOAT32 	PVPower;
	FLOAT32 	PVPowerLimit;
	FLOAT32 	PVPowerLast;
	FLOAT32 	PVPowerOld;
	FLOAT32 	PVPowerChange;
	INT32 	    PVdPowerCnt;
	
	INT32 	    UpvChangeDir;
	FLOAT32     UpvChangeStep;
	
	FLOAT32 	UpvRefMax;
	FLOAT32 	UpvRefMin;
	FLOAT32 	Uoc;
    UINT16      mpptCnt;
public:	
	enum 	mppt_status	WorkStatus;
	FLOAT32 	Debug;
	FLOAT32 	UpvRef;

public:
	mppt();
	FLOAT32 run(FLOAT32 volt, FLOAT32 power);
	void 	rst();
    INT32   SetUoc(FLOAT32 Upv);
	void 	SetPowerLimit(FLOAT32 PvPowerLimit);
	
	
};




#ifdef __cplusplus
}
#endif
#endif
