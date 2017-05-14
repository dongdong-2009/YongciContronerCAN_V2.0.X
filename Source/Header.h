#ifndef _Header_H_
#define _Header_H_

#define FCY 4e6
#include <libpic30.h>

#include "Driver/tydef.h"
#include "Driver/AdcSample.h"
#include "Driver/DevicdIO.h"
#include "Driver/Usart.h"
#include "Driver/Timer.h"
#include "Driver/EEPROMOperate.h"
#include "Driver/CAN.h"
#include "Driver/InitTemp.h"
#include "Driver/Delay.h"

#include "SerialPort/Action.h"
#include "SerialPort/RtuFrame.h"
#include "Yongci/yongci.h"
#include "Yongci/SwtichCondition.h"
#include "Yongci/DeviceParameter.h"

#define MAIN_ADDRESS    0xF0
#define LOCAL_ADDRESS   0xA2 //双路调试控制板子地址

#define LOCAL_CAP_MODULUS   0.127773597592213f

//此处针对第三个控制器做一个全局判断，方便以后更改程序
//**************************************
//#define SMALL_CHOSE 0xF1
#define BIG_CHOSE   0xF2

#ifdef	SMALL_CHOSE
    #define ADCS()  {ADCSSL = 0x000F;}  //ADC扫描通道数,AN0--AN3全部扫描
    #define ADPC()  {ADPCFG = 0xFFF0;}  //AN0--AN3
    #define VOLTAGE_CAP3    {g_SystemVoltageParameter.voltageCap3 = ADCBUF3 * LOCAL_CAP_MODULUS;}
    #define CAP3_STATE  0xFF    //用于判断其是否被激活

#elif BIG_CHOSE
    #define ADCS()  {ADCSSL = 0x0007;}  //ADC扫描通道数，扫描AN0--AN2
    #define ADPC()  {ADPCFG = 0xFFF8;}  //AN0--AN2
//在不使用第三个控制器时，使其变量值始终为0，方便函数GetCapVolatageState（）的移植，以及状态更新
    #define VOLTAGE_CAP3()    {g_SystemVoltageParameter.voltageCap3 = 225;} 
    #define CAP3_STATE  0x00    //用于判断其是否被激活
#endif
//**************************************

#define Reset() {__asm__ volatile ("RESET");}
#endif
