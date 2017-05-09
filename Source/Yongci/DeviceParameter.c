/***********************************************
*Copyright(c) 2016,FreeGo
*保留所有权利
*文件名称:Main.c
*文件标识:
*创建日期： 2016年12月23日 
*摘要:

*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#include "../Header.h"
#include "xc.h"
#include "../Driver/AdcSample.h"
#include "DeviceParameter.h"
#include "../SerialPort/RefParameter.h"
/********************************************
*函数名：  GetCapVoltage()
*形参：void
*返回值：uint16--电容电压ADC值
*功能：软件启动转换，获取ADC值.
**********************************************/
void GetCapVoltage(void)
{
    SoftSampleOnce();
    g_SystemVoltageParameter.workVoltage = ADCBUF0 * 0.001220703125;
    g_SystemVoltageParameter.voltageCap1 = ADCBUF1 * 0.001220703125;
    g_SystemVoltageParameter.voltageCap2 = ADCBUF2 * 0.001220703125;
//    g_SystemVoltageParameter.voltageCap3 = ADCBUF3 * 0.001220703125;
    ClrWdt();
    ClrWdt();
}

/********************************************
*函数名：  GetCapVolatageState()
*形参：void
*返回值：uint16 --电压状态，大于最小值为0xAAAA
*功能：获取电压状态.
**********************************************/
uint16 GetCapVolatageState(void)
{
    GetCapVoltage();
    if ((g_SystemVoltageParameter.voltageCap1  >= LOW_VOLTAGE_ADC) && 
        (g_SystemVoltageParameter.voltageCap2  >= LOW_VOLTAGE_ADC))
    {
        ClrWdt();
        return 0xAAAA;
    }
    else
    {
        return 0;
    }
}


