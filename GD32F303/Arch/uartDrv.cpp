#include "uartDrv.h"
#include "func.h"
#include "Arch.h"

uartDrv::uartDrv(/* args */)
{
    para = 0;
}
/**********************************************************************
 * 串口初始化
************************************************************************/
void uartDrv::init()
{
    dma_parameter_struct setDma;


    if (para == 0)        //先设置硬件参数，才能初始化
    {
        return;
    }

    //变量初始化
    sendState = eUartSendIdle;
    rcvState = eUartRcvIdle;
    rcvCnt = 0;
    rcvTimeout = 0;

    //硬件初始化
    rcu_periph_clock_enable(para->commPower);
    rcu_periph_clock_enable(para->txPinPower);
    rcu_periph_clock_enable(para->rxPinPower);
    //gao_mpptcomm
    rcu_periph_clock_enable(para->enPinPower);

    rcu_periph_clock_enable(RCU_AF);

    usart_deinit(para->comm);
    usart_baudrate_set(para->comm, para->baud);
    usart_parity_config(para->comm, para->parity);
    usart_word_length_set(para->comm, para->dataLen);
    usart_stop_bit_set(para->comm, para->stopNum);
    usart_data_first_config(para->comm, para->first);

    usart_receive_config(para->comm, USART_RECEIVE_ENABLE);
//	usart_transmit_config(para->comm, USART_TRANSMIT_ENABLE);
    usart_dma_transmit_config(para->comm, USART_DENT_ENABLE);
    
    usart_enable(para->comm);

    gpio_init(para->txPort, GPIO_MODE_AF_PP, GPIO_OSPEED_10MHZ, para->txPin);
    gpio_init(para->rxPort, GPIO_MODE_IPU, GPIO_OSPEED_10MHZ, para->rxPin);

    if (para->remap != 0)//ywq_modify:MPPT项目配置修改
    {
        gpio_pin_remap_config(para->remap, ENABLE);
    }
    //DMA
    rcu_periph_clock_enable(para->dmaPower);
    dma_deinit(para->dma, para->channal);

    setDma.periph_addr = para->dataReg; //ywq_modify:MPPT项目配置修改
    setDma.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;           //数据宽度
    setDma.memory_addr = (UINT32)0;                    //内存地址
    setDma.memory_width = DMA_MEMORY_WIDTH_8BIT;
    setDma.number = 2;                                          //传输个数
    setDma.priority = DMA_PRIORITY_MEDIUM;
    setDma.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    setDma.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    setDma.direction = DMA_MEMORY_TO_PERIPHERAL;
    dma_init(para->dma, para->channal, &setDma);

    rcu_periph_clock_enable(para->enPinPower);//重复
    gpio_init(para->enPort, GPIO_MODE_OUT_PP, GPIO_OSPEED_10MHZ, para->enPin);
    gpio_bit_reset(para->enPort, para->enPin);
}

/**********************************************************************
 * 发送数据
 * buf：发送缓存
 * len：发送长度
************************************************************************/
void uartDrv::send(UINT8* buf, UINT16 len)
{
    if (sendState == eUartSendBusy)              //正在发送，返回
    {
        return;
    }
	gpio_bit_set(para->enPort, para->enPin);
    dma_memory_address_config(para->dma, para->channal, (UINT32)buf);
    dma_transfer_number_config(para->dma, para->channal, len);                      //设置发送长度
    dma_flag_clear(para->dma, para->channal, DMA_FLAG_G);                 //清除标志
    usart_transmit_config(para->comm, USART_TRANSMIT_ENABLE);
    dma_channel_enable(para->dma, para->channal);                       //使能通道


    
    sendState = eUartSendBusy;                  //设置忙

}

/**********************************************************************
 * 取接收的数据
************************************************************************/
UINT16 uartDrv::rev(UINT8* buf, UINT16 maxRcv)
{
    UINT16 ret;
    UINT16 len;

    len = MIN2(rcvCnt, maxRcv);

    if (rcvState == eUartRcvEnd)                 //一帧接收完成
    {
        rcvState = eUartRcvIdle;                //空闲
        for (UINT16 i = 0; i < len; i++)          //复制数据
        {
            buf[i] = rcvBuf[i];
        }
        ret = len;                           //接收数据个数
        rcvCnt = 0;                             //清除
        return ret;
    }
    return 0;
}



/**********************************************************************
 * 检查是否发送完成，是否有接收数据
************************************************************************/
//static char uart_debug_buf[300]; //ywq_test
//static int uart_debug_buf_index;
void uartDrv::check()
{
    UINT16 flag;
    flag = USART_STAT0(para->comm);
    
        //发送
        if ((dma_flag_get(para->dma, para->channal, DMA_FLAG_FTF) == SET) && ((flag & USART_STAT0_TC) != 0))        //DMA标志置位 + UART发送完成
        {
            sendState = eUartSendIdle;                              //空闲
            usart_transmit_config(para->comm, USART_TRANSMIT_DISABLE);
            dma_flag_clear(para->dma, para->channal, DMA_FLAG_G);                 //清除标志
            dma_channel_disable(para->dma, para->channal);
			gpio_bit_reset(para->enPort, para->enPin);
        }
    


    //接收
    if ((flag & USART_STAT0_RBNE) == USART_STAT0_RBNE)           //收到数据
    {
        if (rcvState == eUartRcvEnd)                                 //前一帧数据接收完，没有取数据，丢弃
        {
            rcvCnt = 0;
        }
        rcvState = eUartRcvBusy;                                    //接收忙
        rcvTimeout = 0;                                             //超时计数复位
        rcvBuf[rcvCnt] = usart_data_receive(para->comm);                  //取数据保存
        
      //  uart_debug_buf_index = rcvCnt;    //ywq_test
     //   uart_debug_buf[uart_debug_buf_index] = rcvBuf[rcvCnt];
        rcvCnt++;
    }
    rcvTimeout++;                                                   //超时计数
    if (rcvTimeout >= para->timeout)                                   //接收超时，认为一帧数据接收完成
    {
        rcvTimeout = para->timeout;
        if (rcvCnt > 0)
        {
            rcvState = eUartRcvEnd;
        }
    }

    if (rcvCnt >= 300)                                               //接收缓存满了
    {
        rcvState = eUartRcvEnd;
    }
}
