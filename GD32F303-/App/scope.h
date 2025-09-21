#ifndef _SCOPE_H_
#define _SCOPE_H_
#ifdef __cplusplus
extern "C"
{
#endif
#include "usertypes.h"

#define CHANNAL_NUM     (4)         //通道数目
#define BUF_NUM         (500)       //每个通道数据长度
struct channal
{
    UINT32 addr;                                            //地址
    void *ptr;                                              //指针
    UINT16 type;        //0:disable, 1:32bit, 2:16bit       //数据类型
    UINT32 buf[BUF_NUM];                                    //缓存
};

class scope
{
private:
    channal ch[CHANNAL_NUM];                        //通道

    UINT16 bufCnt;                                  //已完成采样个数
    UINT16 psc;                                     //分频， 多少个中断采样一次
    UINT16 pscCnt;                                  //分频计数

    UINT32 trigAddr;
    UINT32 trigNum;
    UINT16 trigType;                                //0:disable, 1:32bit, 2:16bit       //数据类型
public:
    UINT16 trig;                                    //触发  0：停止，1：开始，2：采样中
    scope(/* args */);

    void run();

};





#ifdef __cplusplus
}
#endif
#endif
