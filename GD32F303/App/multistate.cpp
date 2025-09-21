#include "multistate.h"


multistate::multistate(/* args */)
{
    state = 0;
    statePre = 0;
}


void multistate::SetPara(UINT16 num, const FLOAT32 *point, FLOAT32 g)
{
    stateNum = num;
    breakPoint = (FLOAT32 *)point;
    gap = g;
}

void multistate::execute(FLOAT32 in)
{
    UINT16 i;

    if((0 == statePre) && (in < breakPoint[0] + gap))
    {
        state = statePre;
    }
    else if(((stateNum-1) == statePre) && (in > breakPoint[stateNum-2] - gap))
    {
        state = statePre;
    }
    else if((in >  breakPoint[statePre-1]-gap) && (in < breakPoint[statePre]+gap))
    {
        state = statePre;
    }
    else
    {
        for(i=0; i<stateNum-1; i++)
        {
            if(in < breakPoint[i] )
            {
                break;
            }
        }
        state = i;

    }
    statePre = state;
}
