#ifndef _UART_DRV_H_
#define _UART_DRV_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "gd32f30x_libopt.h"
#include "usertypes.h"

    //发送状态
    enum uartSendState
    {
        eUartSendIdle = 0,              //空闲
        eUartSendBusy                   //发送忙
    };

    //接收状态
    enum uartRcvState
    {
        eUartRcvIdle = 0,               //空闲
        eUartRcvBusy,                   //接收忙
        eUartRcvEnd                     //接收完成
    };

    //硬件相关信息
    struct uartPara
    {
        rcu_periph_enum commPower;                           //开启串口电源
        rcu_periph_enum txPinPower;                           //开启TX电源
        rcu_periph_enum rxPinPower;                           //开启RX电源

        //gao_mpptcomm
        rcu_periph_enum enPinPower;                         //使能

        UINT32 comm;                                //串口
        UINT16 timeout;                             //帧超时，大约3个字节的时间
        UINT32 dataLen;                             //数据长度
        UINT32 first;                                 //LSB/MSB
        UINT32 stopNum;                             //停止位
        UINT32 parity;                              //奇偶校验
        UINT32 baud;                                //波特率
        UINT32 txPort;                              //发送脚端口
        UINT32 txPin;                               //发送脚引脚
        UINT32 rxPort;                              //接收脚端口
        UINT32 rxPin;                               //接收脚引脚

        //gao_mpptcomm
        UINT32 enPort;                              //是能端口
        UINT32 enPin;                               //使能引脚

        UINT32 remap;

        rcu_periph_enum dmaPower;                   //开启DMA电源
        UINT32 dma;                                //DMA
        dma_channel_enum channal;                   //通道号
        UINT32 dataReg;
    };


    class uartDrv
    {
    private:
        uartPara* para;                     //参数

        uartSendState sendState;            //发送状态

        uartRcvState rcvState;              //接收状态
        UINT8 rcvBuf[300];                  //接收缓存
        UINT16 rcvCnt;                       //接收计数
        UINT8 rcvTimeout;                   //接收超时
    public:
        uartDrv(/* args */);
        void init();                        //初始化
        void send(UINT8* buf, UINT16 len);  //发送数据
        void check();                       //查询发送接收状态
        UINT16 rev(UINT8* buf, UINT16 maxRcv);             //接收
        
        //gao_mpptcomm
        //inv通讯用
        UINT8 txBuf[300];
        UINT16 txCnt;
        UINT16 txLen;

        void setPara(const uartPara* p) { para = (uartPara*)p; };
    };






#ifdef __cplusplus
}
#endif
#endif

