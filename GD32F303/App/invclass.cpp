#include "invclass.h"
#include "variable.h"
#include "func.h"
#include "config.h"
#include "arch.h"


#define INV_CMP_MAX         ((INT16)((INT16)PWM_PRD - 50))              //比较值限幅
#define REC_CMP_MAX         ((INT16)((INT16)PWM_PRD - 50))              //比较值限幅


invclass::invclass(/* args */)
{
    invWorkMode = INV_WORK_MODE_OFF;
    invWorkModeReal = INV_WORK_MODE_OFF;
    pwmEn = 0;
    invBusGoal = BUS_TARGET_NORMAL * RATE_OP_VOLT_RECIPROCAL;
    rmsRef = 1.0f;
    rmsDrop = 0.0f;
}


FLOAT32 iref;
FLOAT32 rmsRegOut, vtRef;
FLOAT32 ctrlOut, currErrOut;
void invclass::InvReg()
{
    FLOAT32 temp;
    FLOAT32 fb;
    FLOAT32 currRef;
    FLOAT32 currErr;
    FLOAT32 verr;
    FLOAT32 currRmsRef;
    FLOAT32 rmsLoopOut;
    FLOAT32 currLoopOut;
    FLOAT32 capCmpn;
    FLOAT32 rmsRefTemp;

    
#if 0
    if(invWorkMode != INV_WORK_MODE_OFF)
    {
        if(objLock2.GetTheta() < 0.17f)
        {
            invWorkModeReal = invWorkMode;
        }
    }
    else
    {
        invWorkModeReal = invWorkMode;
    }
#else
    invWorkModeReal = invWorkMode;
#endif

    switch(invWorkModeReal)
    {
    case INV_WORK_MODE_OFF:         //关机
        cmp = 0; 
        pwmEn = 0;
        pr1.rst();
        pr3.rst();
        busVoltLoop.rst();
        rmsLoop.rst();
        vtLoop.rst();
        invBusRef = objDataCalc.GetBusVoltAvg();
        break;

    case INV_WORK_MODE_INV:                                     //INV

        rmsRefTemp = rmsRef - rmsDrop;
        verr = rmsRefTemp - objDataCalc.GetInvVoltRms();
        UpDownLimit(verr, (10.0f * RATE_OP_VOLT_RECIPROCAL), (-10.0f * RATE_OP_VOLT_RECIPROCAL));
        rmsLoopOut = rmsLoop.run(verr);
        rmsRegOut = rmsRefTemp + rmsLoopOut;
        vtRef = SQRT2 * rmsRegOut * objLock2.GetSin();

        verr = vtRef - objSample.GetInvVolt();
        UpDownLimit(verr, (15.0f * RATE_OP_VOLT_RECIPROCAL), (-15.0f * RATE_OP_VOLT_RECIPROCAL));
        currRef = vtLoop.run(verr);
        currErr = objSample.GetInvCurr() + currRef;
        UpDownLimit(currErr, (0.5f * RATE_OP_CURR_RECIPROCAL), (-0.5f * RATE_OP_CURR_RECIPROCAL));
        currLoopOut = currErr * invCurrKp;
        temp = vtRef + currLoopOut;
//        temp = vtRef;

        cmp = ARCH_Div(temp, objSample.GetBusVolt()) * PWM_PRD;
        UpDownLimit(cmp, INV_CMP_MAX, (-INV_CMP_MAX));
        pwmEn = 1;

        invBusRef = objDataCalc.GetBusVoltAvg();

        break;
        
    case INV_WORK_MODE_REC:                     //REC 电流正
        
    #if 0
        fb = objDataCalc.GetInvVoltRms() * SQRT2 * objLock2.GetSin();                  //前馈
        UpDownLimit(fb, (objSample.GetInvVolt() + 15.0f * objRated.rateVolt), (objSample.GetInvVolt() - 15.0f * objRated.rateVolt));
    #else
        fb = objDataCalc.GetAcInVoltRms() * SQRT2 * objLock2.GetSin() * 0.5f + objSample.GetAcInputVolt() * 0.5f;                  //前馈
        UpDownLimit(fb, (objSample.GetAcInputVolt() + 15.0f * objRated.voltReciprocal), (objSample.GetAcInputVolt() - 15.0f * objRated.voltReciprocal));
    #endif
//        fb = objDataCalc.GetAcInVoltRms() * SQRT2 * objLock2.GetSin();                  //前馈
//        UpDownLimit(fb, (objSample.GetAcInputVolt() + 15.0f * objRated.rateVolt), (objSample.GetAcInputVolt() - 15.0f * objRated.rateVolt));

        capCmpn = objRated.invCapCmpn * objDataCalc.GetWAvg() * objDataCalc.GetAcInVoltRms();
        verr = invBusRef - objSample.GetBusVolt();
        UpDownLimit(verr, (40.0f * RATE_OP_VOLT_RECIPROCAL), (-40.0f * RATE_OP_VOLT_RECIPROCAL));
        currRmsRef = busVoltLoop.run(verr);

        currRef = currRmsRef * objLock2.GetSin() * SQRT2 - capCmpn * objLock2.GetCos();      //电流参考  //
        iref = currRef;
        currErr = currRef - objSample.GetInvCurr();
        UpDownLimit(currErr, objRated.invCurrLoopCurrLmt, (-objRated.invCurrLoopCurrLmt));
        currErrOut = currErr * objRated.rateCurr;
        temp = currErr * recCurrKp;
        UpDownLimit(temp, objRated.invCurrLoopPoutLmt, (-objRated.invCurrLoopPoutLmt));
        ctrlOut = temp * objRated.rateVolt;
        
        FLOAT32 prout1, prout3;
        prout1 = pr1.run(currErr);
        UpDownLimit(prout1, objRated.invPrOutLmt, (-objRated.invPrOutLmt));
        prout3 = pr3.run(currErr);
        UpDownLimit(prout3, objRated.invPrOutLmt, (-objRated.invPrOutLmt));
        

        temp = fb - temp - prout1 - prout3;//
        
        temp = ARCH_Div(temp, objSample.GetBusVolt()) * PWM_PRD;
        UpDownLimit(temp, (REC_CMP_MAX), -(REC_CMP_MAX));
        cmp = temp; 
        pwmEn = 2;
        break;
    
    default:
        break;
    }
}

void invclass::BusSoftStart()
{
    if(invBusRef < invBusGoal)
    {
        invBusRef += BUS_SOFT_VAL2 * objRated.voltReciprocal;
    }
    else
    {
        invBusRef -= BUS_SOFT_VAL2 * objRated.voltReciprocal;
    }

    
    if(rmsRef < rmsGoal)
    {
        rmsRef += 0.2f * objRated.voltReciprocal;
    }
    else
    {
        rmsRef -= 0.2f * objRated.voltReciprocal;
    }

}
