#include "piecewise.h"

piecewise::piecewise(/* args */)
{
    number = 0;
    linePtr = 0;
    pointPtr = 0;
}

/***************************************
 * 设置参数
 * n：分割点个数
 * pp：分割点数组指针
 * lp：线数组指针
 * ************************************/
void piecewise::SetPara(UINT16 n, const FLOAT32 *pp, const line *lp)
{
    number = n;
    pointPtr = (FLOAT32 *)pp;
    linePtr = (line *)lp;
}

/***************************************
 * 顺序查找，获取值
 * ************************************/
FLOAT32 piecewise::GetValue(FLOAT32 input)
{
    UINT16 index;

    for(index = 0; index < number; index++)  //找到是哪一段
    {
        if(input < pointPtr[index])
        {
            break;
        }
    }

    return input * linePtr[index].gain + linePtr[index].bais;  //带入直线方程    
}
/***************************************
 * 折半查找，获取值
 * ************************************/
FLOAT32 piecewise::GetValueHalf(FLOAT32 input)
{
    UINT16 index, min, max;

    min = 0;
    max = number-1;
    if(pointPtr[max] < input)
    {
        index = number;
    }
    else if(pointPtr[min] > input)
    {
        index = 0;
    }
    else
    {  
        while (max > min)
        {
            index = (min + max) >> 1;
            if (pointPtr[index] > input)
            {
                max = index;
            }
            else
            {
                min = index + 1;
            }            
        }
        index = max;
    }
    
    return input * linePtr[index].gain + linePtr[index].bais;  //带入直线方程    
}

