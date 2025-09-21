#ifndef _STANDARD_H_
#define _STANDARD_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "usertypes.h"

class standard
{
private:

public:
    standard(/* args */);

    virtual void Init();
    virtual void SlowChk();
    virtual void FastChk();

};


#ifdef __cplusplus
}
#endif
#endif




