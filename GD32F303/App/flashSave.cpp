#include "flashSave.h"
#include <string.h>
#include "func.h"
#include "arch.h"

flashSave::flashSave(/* args */)
{
}

void flashSave::Init(void *arg)
{
    UINT16 crcTemp;
    UINT16 saveLenTemp;
    UINT16 saveNum;
    UINT16 currNum;
    UINT16 i;
    UINT16 *p16;
    UINT16 *currP16;
    
    ptr = arg;
    saveNum = GetSaveLen();                     //��������ݸ���
    saveLenTemp = saveNum << 1;                 //��������ݳ���
    currNum = (sizeof(flashDataStruct) >> 1);   //��ǰ��ʽ���ݸ���
    p16 = (UINT16 *)ptr;
    currP16 = (UINT16 *)&currPara;
    
    
    
    if(saveNum == currNum)              //����Ĳ�������û�б�
    {
        memcpy((void *)&currPara, (void *)arg, sizeof(flashDataStruct));
        crcTemp = crc_modbus((UINT8 *)&currPara, sizeof(flashDataStruct) - 2);
        
        if(crcTemp != currPara.dataCrc)         //У�鲻ͨ��
        {
            for(i = 0; i < (currNum - 1); i++)
            {
                currP16[i] = CALIBRATE_CONSTANT;
            }
            crcTemp = crc_modbus((UINT8 *)&currPara, sizeof(flashDataStruct) - 2);
            currPara.dataCrc = crcTemp;
            ARCH_FlashHalfWord((UINT32)ptr, (UINT16 *)&currPara, sizeof(flashDataStruct) >> 1);
        }
    }
    else if(saveNum != 0)               //����Ĳ�����������
    {
        memcpy((void *)&currPara, (void *)arg, saveLenTemp - 2);
        crcTemp = crc_modbus((UINT8 *)&currPara, saveLenTemp - 2);
        if(crcTemp != p16[saveNum - 1])
        {
            for(i = 0; i < (saveNum - 1); i++)
            {
                currP16[i] = CALIBRATE_CONSTANT;
            }
        }
        for(i = (saveNum - 1); i < (currNum - 1); i++)
        {
            currP16[i] = CALIBRATE_CONSTANT;
        }
         crcTemp = crc_modbus((UINT8 *)&currPara, sizeof(flashDataStruct) - 2);
         currPara.dataCrc = crcTemp;
         ARCH_FlashHalfWord((UINT32)ptr, (UINT16 *)&currPara, sizeof(flashDataStruct) >> 1);  
    }
    else            //һ��������û��
    {
        for(i = 0; i < (currNum - 1); i++)
        {
            currP16[i] = CALIBRATE_CONSTANT;
        }
        crcTemp = crc_modbus((UINT8 *)&currPara, sizeof(flashDataStruct) - 2);
        currPara.dataCrc = crcTemp;
        ARCH_FlashHalfWord((UINT32)ptr, (UINT16 *)&currPara, sizeof(flashDataStruct) >> 1); 
    }
}
void flashSave::WriteToFlash()
{
    UINT16 crcTemp;
    UINT16 i;
    UINT16 *p16;
    
    p16 = (UINT16 *)&currPara;
    for(i = 0; i < (sizeof(flashDataStruct) >> 1) - 1; i++)
    {
        UpDownLimit(p16[i], CALIBRATE_MAX, CALIBRATE_MIN);
    }
    
    crcTemp = crc_modbus((UINT8*)&currPara, sizeof(flashDataStruct) - 2);
    currPara.dataCrc = crcTemp;    
    ARCH_FlashHalfWord((UINT32)ptr, (UINT16 *)&currPara, sizeof(flashDataStruct) >> 1);  
}


UINT16 flashSave:: GetSaveLen()
{
    UINT16 i;
    UINT16 *data;
    UINT16 lenMax;
    
    data = (UINT16 *)(ptr);
    lenMax = sizeof(flashDataStruct) >> 1;
    
    for(i = 0; i < lenMax; i++)
    {
        if((data[i] == 0xFFFF) && (data[i + 1] == 0xFFFF))
        {
            break;
        }
    }
    return i;   
}


    //PV0�����ѹ
    UINT16 flashSave::SetInDcVoltCoff(UINT16 arg){return DataLmt(&currPara.InDcVoltCoff, arg);}
    
    
    //PV1�����ѹ
    UINT16 flashSave::SetInDc2VoltCoff(UINT16 arg) {return DataLmt(&currPara.InDc2VoltCoff, arg);}
    
    
    //��ص�ѹ
    UINT16 flashSave::SetBattVoltCoff(UINT16 arg) {return DataLmt(&currPara.BattVoltCoff, arg);}
    
    
    //PV0�������
    UINT16 flashSave::SetInDcCurrCoff(UINT16 arg) {return DataLmt(&currPara.InDcCurrCoff, arg);}
    
    
    //PV1�������
    UINT16 flashSave::SetInDc2CurrCoff(UINT16 arg) {return DataLmt(&currPara.InDc2CurrCoff, arg);}
    
    
    //��ص���
    UINT16 flashSave::SetBattCurrCoff(UINT16 arg) {return DataLmt(&currPara.BattCurrCoff, arg);}
    

