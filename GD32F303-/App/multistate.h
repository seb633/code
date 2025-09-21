#ifndef _MULTI_STATE_H_
#define _MULTI_STATE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "usertypes.h"

class multistate
{
private:
    UINT16 stateNum;
    FLOAT32 *breakPoint;
    FLOAT32 gap;

    UINT16 state;
    UINT16 statePre;
public:
    multistate(/* args */);
    void SetPara(UINT16 num, const FLOAT32 *point, FLOAT32 g);
    void execute(FLOAT32 in);
    UINT16 GetState() {return state;};
};




#ifdef __cplusplus
}
#endif


#endif
