#include "faultChk.h"

faultChk::faultChk()
{
    state = 0;
    cnt = 0;
}

void faultChk::Init(UINT16 stateSet, INT16 cntSet)
{
    state = stateSet;
    cnt = cntSet;
}


