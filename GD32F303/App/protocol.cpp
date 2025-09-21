#include "protocol.h"
#include "func.h"
#include "config.h"
UINT32 commFaultCnt = 0;
protocol::protocol(/* args */)
{
    rxNum = 0;
    chargeCtrl.all = 0;
   // chargeCtrl.bit.chargeOn = 1;
   // chargeCtrl.bit.chargeOff = 0;
    
    chargePowerMax = RATE_OP_POWER * 0.2f;
    chargeVoltMax = CHARGE_VOLT_MAX * 10;
    lowPowerSet = RATE_OP_POWER;
    
    commLostCnt = 0;
    commLostState = 1;
    pvCurrMax = PV_MAX_CURR_1 * 10;
    
    MPPTchargePowerSet = PV_MAX_POWER * 2.0f;
	MPPTchargeVoltSet = BATT_HIGH_POINT * 10.0f;
	MPPTchargeCurrSet = BATT_CURR_OV_RET_POINT * 10.0f;
	
	fanSpeedPercentage = 0;
}
void protocol::SetPara(const uartPara *para)
{
    comm.setPara(para);
    comm.init();
}
void protocol::deal()
{
    UINT16 temp, crcRx, index = 0, dataLen, ret = 0;
	UINT16 i;
	
    commLostCnt++;
    if(commLostCnt > 25)
    {
        commLostState = 1;
    }
    
    rxNum = comm.rev(rxBuf, 256);            //接收数据
    
    
    if(rxNum <= 6)                      //没收到数据
    {        
        return;
    }
    
    commLostCnt = 0;
    commLostState = 0;
    if((rxBuf[0] != FIRST_BYTE) || (rxBuf[1] != SECOND_BYTE))
    {
        commFaultCnt++;
        return;
    }
   
    if(((rxBuf[2] != BMU_ADDR) && (rxBuf[2] != APP_ADDR)) || (rxBuf[3] != MPPT_ADDR))
    {
        commFaultCnt++;
        return;
    }

    temp = crc_modbus(rxBuf+2, rxNum - 4);
    crcRx = ((UINT16)rxBuf[rxNum-1] << 8) | rxBuf[rxNum-2];

    if(temp != crcRx)                     //CRC校验错误
    {
        commFaultCnt++;
        return;
    }
    
    //数据帧头几段,Byte 0-4
    txBuf[index++] = FIRST_BYTE;
    txBuf[index++] = SECOND_BYTE;
    txBuf[index++] = MPPT_ADDR;
    txBuf[index++] = rxBuf[2];
    txBuf[index++] = rxBuf[4];
    dataLen = ((UINT16)rxBuf[6] << 8) | rxBuf[5];
    
    //根据不同SID来分类处理
    switch (rxBuf[4])
    {
    case 0x00:    //软硬件版本
        txBuf[index++] = 0x06;
        txBuf[index++] = 0x00;
        txBuf[index++] = SwVerLow;
        txBuf[index++] = SwVerMid;
        txBuf[index++] = SwVerHigh;
        txBuf[index++] = HwVerLow;
        txBuf[index++] = HwVerMid;
        txBuf[index++] = HwVerHigh;
        ret = 1;
        break;
    case 0x01:    //状态
        txBuf[index++] = 0x01;
        txBuf[index++] = 0x00;
        
        txBuf[index++] = workState;//5种状态需要在worklogic里面写！！！
        ret = 1;
        break;
    case 0x02:      //MPPT充电数据
    /*    
        txBuf[index++] = 0x08;
        txBuf[index++] = 0x00;
    
        txBuf[index++] = chargeVolt & 0x00ff;
        txBuf[index++] = chargeVolt >> 8;
        txBuf[index++] = chargeCurr & 0x00ff;
        txBuf[index++] = chargeCurr >> 8;
        txBuf[index++] = chargePower & 0x00ff;
        txBuf[index++] = chargePower >> 8;
    */
    
        txBuf[index++] = 0x0A;
        txBuf[index++] = 0x00;
    
        txBuf[index++] = pvVolt & 0x00ff;
        txBuf[index++] = pvVolt >> 8;
        txBuf[index++] = pv2Volt & 0x00ff;
        txBuf[index++] = pv2Volt >> 8;
        txBuf[index++] = pvCurr & 0x00ff;
        txBuf[index++] = pvCurr >> 8;
        txBuf[index++] = pvPower & 0x00ff;
        txBuf[index++] = pvPower >> 8;
    
        txBuf[index++] = TemperOffset & 0x00ff;
        txBuf[index++] = TemperOffset >> 8;
		
		/**/
    
        ret = 1;
        break;

    case 0x03:      //告警信息
        txBuf[index++] = 0x02;
        txBuf[index++] = 0x00;
        txBuf[index++] = alarm1.all & 0x00ff;   //具体告警定义需要写
        txBuf[index++] = alarm1.all >> 8;
        ret = 1;
        break;

    case 0x04:      //故障数据
        txBuf[index++] = 0x02;
        txBuf[index++] = 0x00;
        txBuf[index++] = fault1 & 0x00ff;
        txBuf[index++] = fault1 >> 8;
        ret = 1;
        break;

    case 0x05:      //风扇状态
        txBuf[index++] = 0x01;
        txBuf[index++] = 0x00;
        txBuf[index++] = fanSpeedPercentage;
        ret = 1;
        break;
    case 0x06:      //充电控制数据
        if (dataLen)
        {
            chargeCtrl.all = rxBuf[7];//协议漏写了数据位？？？
        }
        txBuf[index++] = 0x01;
        txBuf[index++] = 0x00;
        txBuf[index++] = chargeCtrlReal.all & 0x00ff;//具体数据需要定义！！！
        ret = 1;
        break;

    case 0x07:      //MPPT充电设置数据帧
        if (dataLen)
        {
            MPPTchargeVoltSet = ((UINT16)rxBuf[8] << 8) | rxBuf[7];
            MPPTchargeCurrSet = ((UINT16)rxBuf[10] << 8) | rxBuf[9];

        }
        txBuf[index++] = 0x04;
        txBuf[index++] = 0x00;
        txBuf[index++] = MPPTchargeVoltSet & 0x00ff;
        txBuf[index++] = MPPTchargeVoltSet >> 8;
        txBuf[index++] = MPPTchargeCurrSet & 0x00ff;
        txBuf[index++] = MPPTchargeCurrSet >> 8;
        ret = 1;
        break;
    
    case 0x08:      //充电功率设置帧
        if (dataLen)
        {
            MPPTchargePowerSet = ((UINT16)rxBuf[8] << 8) | rxBuf[7];
        }
        txBuf[index++] = 0x02;
        txBuf[index++] = 0x00;
        txBuf[index++] = MPPTchargePowerSet & 0x00ff;
        txBuf[index++] = MPPTchargePowerSet >> 8;
        ret = 1;
        break;

	case 0xA0:      //调试数据
        txBuf[index++] = 31;  
        txBuf[index++] = 0x00;

        txBuf[index++] = pvVolt & 0x00ff;
        txBuf[index++] = pvVolt >> 8;
        txBuf[index++] = pv2Volt & 0x00ff;
        txBuf[index++] = pv2Volt >> 8;

        txBuf[index++] = pv1Curr & 0x00ff;
        txBuf[index++] = pv1Curr >> 8;

		txBuf[index++] = pv2Curr & 0x00ff;
        txBuf[index++] = pv2Curr >> 8;
        txBuf[index++] = pvPower & 0x00ff;
        txBuf[index++] = pvPower >> 8;
	
		txBuf[index++] = chargeVolt & 0x00ff;
        txBuf[index++] = chargeVolt >> 8;
        txBuf[index++] = chargeCurr & 0x00ff;
        txBuf[index++] = chargeCurr >> 8;
        txBuf[index++] = chargePower & 0x00ff;
        txBuf[index++] = chargePower >> 8;

        txBuf[index++] = TemperOffset & 0x00ff;
        txBuf[index++] = TemperOffset >> 8;
		
		txBuf[index++] = pv1CurrLmt & 0x00ff;
        txBuf[index++] = pv1CurrLmt >> 8;
		txBuf[index++] = pv2CurrLmt & 0x00ff;
        txBuf[index++] = pv2CurrLmt >> 8;
		
		txBuf[index++] = alarm1.all & 0x00ff;   
        txBuf[index++] = alarm1.all >> 8;
		
		txBuf[index++] = workState;
		
		txBuf[index++] = MpptworkState;
		txBuf[index++] = Mppt2workState;
		
		txBuf[index++] = MpptUpvRef & 0x00ff;
		txBuf[index++] = MpptUpvRef >> 8;
		txBuf[index++] = Mppt2UpvRef & 0x00ff;
		txBuf[index++] = Mppt2UpvRef >> 8;
		
        ret = 1;
        break;
	case 0xE0:      //校准系数
        if(dataLen)
        {
            if(rxBuf[7] < CALIBRATE_NUM)        //检查下标
            {
                calibrateCoff[rxBuf[7]] = ((UINT16)rxBuf[9] << 8) | rxBuf[8];
                e0Flag = 1;
            }
        }
        txBuf[index++] = CALIBRATE_NUM * 2;
        txBuf[index++] = 0x00;
        
        for(i = 0; i < CALIBRATE_NUM; i++)
        {
            txBuf[index++] = calibrateCoffReal[i] & 0x00ff;
            txBuf[index++] = calibrateCoffReal[i] >> 8;
        }
        ret = 1;
        break;

    case 0xE1:      //校准数据
        txBuf[index++] = CALIBRATE_NUM * 2;
        txBuf[index++] = 0x00;
    
        for(i = 0; i < CALIBRATE_NUM; i++)
        {
            txBuf[index++] = calibrateValue[i] & 0x00ff;
            txBuf[index++] = calibrateValue[i] >> 8;
        }
        ret = 1;
        break;
    

    case 0x0F:          //下电
        powerOffCmd = 1;
        txBuf[index++] = 0x01;
        txBuf[index++] = 0x00;
        txBuf[index++] = 0x01;//不查询设备是否忙吗？？？
        ret = 1;
        break;


    case 0xF0:
        iapCmd = 1;
        txBuf[index++] = 0x01;
        txBuf[index++] = 0x00;
        txBuf[index++] = 0x01;//升级应答码还缺其他5种情况？？？
        ret = 1;
        break;
    default:
        break;
    }

    if(ret)
    {
        temp = crc_modbus(txBuf+2, index-2);
        txBuf[index++] = temp & 0x00FF;
        txBuf[index++] = temp >> 8;

        comm.send(txBuf, index);
    }
}

void protocol::CalibrateCoffInit(UINT16 n, UINT16 arg)
{
    if(n >= CALIBRATE_NUM)
    {
        return;
    }
    calibrateCoff[n] = arg;
    calibrateCoffReal[n] = arg;
}
