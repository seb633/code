#include "llcCtrl.h"
#include "variable.h"
#include "config.h"
#include "arch.h"


llcCtrl::llcCtrl(/* args */)
{
    pwmEn = 0;
    llcMode = LLC_WORK_MODE_OFF;
    prd = 300;
    busGoal = BUS_TARGET_NORMAL * RATE_OP_VOLT_RECIPROCAL;
    battGoal = CHARGE_VOLT_MAX * RATE_OP_VOLT_RECIPROCAL;
}

void llcCtrl::llcReg()
{
    FLOAT32 verr, ierr, iref, temp;

    switch (llcMode)
    {
    case LLC_WORK_MODE_OFF:
        dchgVoltLoop.rst();
        chgVoltLoop.rst();
        dchgCurrLoop.rst();
        chgCurrLoop.rst();
        prd = LLC_BUS_PRD_MIN;
        pwmEn = 0;
        dead = LLC_BUS_DEAD_TIME_MAX;
        busRef = objDataCalc.GetBusVoltAvg();
        battRef = objDataCalc.GetBattVoltAvg();
        dchgFilter = 0.0f;
        break;
    case LLC_WORK_MODE_CHARGE://电流负
        
        verr = battRef - objSample.GetBattVolt();
        UpDownLimit(verr, (2.0f * RATE_OP_VOLT_RECIPROCAL), (-2.0f * RATE_OP_VOLT_RECIPROCAL));
        iref = chgVoltLoop.run(verr);
        ierr = iref + objDataCalc.GetBattCurrFilt();
        UpDownLimit(ierr, (2.0f * RATE_OP_CURR_RECIPROCAL), (-2.0f * RATE_OP_CURR_RECIPROCAL));
        temp = chgCurrLoop.run(ierr);


        if(temp > LLC_BUS_DEAD_TIME_LEN)
        {
            pwmEn = 2;
            prd = LLC_BUS_PRD_MIN + temp - LLC_BUS_DEAD_TIME_LEN;
            dead = LLC_BUS_DEAD_TIME_MIN;
        }
        else if(temp > 0.0f)
        {
            pwmEn = 2;
            prd = LLC_BUS_PRD_MIN;
            dead = LLC_BUS_DEAD_TIME_MAX - temp;
        }
        else
        {
            pwmEn = 0;
            prd = LLC_BUS_PRD_MIN;
            dead = LLC_BUS_DEAD_TIME_MAX;
        }

        dchgVoltLoop.rst();
        dchgCurrLoop.rst();
        busRef = objDataCalc.GetBusVoltAvg();
        break;


    case LLC_WORK_MODE_DISCHARGE://电流正

        verr = busRef - objSample.GetBusVolt();
        UpDownLimit(verr, (50.0f * objRated.voltReciprocal), (-50.0f * objRated.voltReciprocal));
    
#if 0    
        temp = dchgVoltLoop.run(verr);
#else
        dchgVoltLoop.run(verr);
        if(verr > (20.0f * objRated.voltReciprocal))
        {
            dchgFilter = Filter(dchgVoltLoop.out, 1.0f, dchgFilter);
        }
        else
        {
            dchgFilter = Filter(dchgVoltLoop.out, 0.2f, dchgFilter);
        }
        temp = dchgFilter;
#endif

        if(temp > 0.0f)
        {
            pwmEn = 1;
            prd = LLC_BAT_PRD;
            dead = LLC_BAT_DEAD_TIME_MAX - temp;
        }
        else
        {
            pwmEn = 0;
            prd = LLC_BAT_PRD;
            dead = LLC_BAT_DEAD_TIME_MAX;
        }


        chgVoltLoop.rst();
        chgCurrLoop.rst();
        battRef = objDataCalc.GetBattVoltAvg();
        break;
    
    default:
        break;
    }
}
//2ms
void llcCtrl::busSoftStart()
{
    if(busRef < busGoal)
    {
        busRef += BUS_SOFT_VAL * objRated.voltReciprocal;
    }
    else
    {
        busRef -= BUS_SOFT_VAL * objRated.voltReciprocal;
    }

    if(battRef < battGoal)
    {
        battRef += BATT_SOFT_VAL * objRated.voltReciprocal;
    }
    else
    {
        battRef -= BATT_SOFT_VAL * objRated.voltReciprocal;
    }

}



