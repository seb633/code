#include "scope.h"
scope::scope(/* args */)
{
    trig = 0;
    bufCnt = 0;
    psc = 1;
    pscCnt = 0;
}

void scope::run()
{
    if(trigAddr != 0)
    {
        if(trigType == 0)
        {
            
        }
        else if(trigType == 1)
        {
            UINT32 *p = (UINT32 *)(trigAddr & 0xfffffffc);
            if(*p == trigNum)
            {
                trig = 1;
            }
        }
        else if(trigType == 2)
        {
            UINT16 *p = (UINT16 *)(trigAddr & 0xfffffffc);
            if(*p == trigNum)
            {
                trig = 1;
            }
        }
            
    }
    
    
    if(trig == 1)
    {
        for (UINT16 i = 0; i < CHANNAL_NUM; i++)
        {
            if(ch[i].type == 1)
            {
                ch[i].ptr = (void *)(ch[i].addr & 0xfffffffc);
            }
            else if(ch[i].type == 2)
            {
                ch[i].ptr = (void *)(ch[i].addr & 0xfffffffe);
            }
            else
            {

            }
        }
        trig = 2;
        bufCnt = 0;
        pscCnt = psc;
    }
    else if(trig == 2)
    {
        if(pscCnt >= psc)
        {
            for (UINT16 i = 0; i < CHANNAL_NUM; i++)
            {
                if(ch[i].type == 1)
                {
                    UINT32 *p = (UINT32 *)ch[i].ptr;
                    ch[i].buf[bufCnt] = *p;
                }
                else if(ch[i].type == 2)
                {
                    UINT16 *p = (UINT16 *)ch[i].ptr;
                    ch[i].buf[bufCnt] = *p;
                }
                else
                {

                }
            }
            bufCnt++;
            if(bufCnt >= BUF_NUM)
            {
                trig = 0;
                bufCnt = 0;
            }
            pscCnt = 1;           
        }
        else
        {
            pscCnt++;
        }

    }
}

