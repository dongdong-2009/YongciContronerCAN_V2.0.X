/** 
 * <p>application name： Header.h</p> 
 * <p>application describing： 主函数宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */
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
#include "Driver/ImitationIIC.h"
#include "Driver/SD2405.h"

#include "SerialPort/Action.h"
#include "SerialPort/RtuFrame.h"
#include "Yongci/SwtichCondition.h"
#include "Yongci/yongci.h"
#include "Yongci/DeviceParameter.h"

#define LOCAL_ADDRESS   0xA2 //双路调试控制板子地址

#define LOCAL_CAP_MODULUS   0.125732421875f

//此处针对第三个控制器做一个全局判断，方便以后更改程序
//**************************************
#define SMALL_CHOSE 0xF1
//#define BIG_CHOSE   0xF2

#ifdef	SMALL_CHOSE
    #define ADCS()  {ADCSSL = 0x000F;}  //ADC扫描通道数,AN0--AN3全部扫描
    #define ADPC()  {ADPCFG = 0xFFF0;}  //AN0--AN3
    #define CAP3_DROP_VOLTAGE() {g_SystemVoltageParameter.capDropVoltage3 = ADCBUF3 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient3;}    
    #define CAP3_STATE  0xFF    //用于判断其是否被激活
    #define NUM_CHS2SCAN 4 //扫描几路ADC就相应的赋值即可
    #define CHECK_ORDER3()    (g_SetSwitchState[2].Order == IDLE_ORDER)
    #define CHECK_LAST_ORDER3()     (g_SetSwitchState[2].LastOrder != IDLE_ORDER)
    #define CHECK_VOLTAGE_CAP3()    (g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down)

#elif BIG_CHOSE
    #define ADCS()  {ADCSSL = 0x0007;}  //ADC扫描通道数，扫描AN0--AN2
    #define ADPC()  {ADPCFG = 0xFFF8;}  //AN0--AN2
//在不使用第三个控制器时，使其变量值始终为0，方便函数GetCapVolatageState（）的移植，以及状态更新
    #define CAP3_DROP_VOLTAGE() {g_SystemVoltageParameter.capDropVoltage3 = 225;}
    #define CAP3_STATE  0x00    //用于判断其是否被激活
    #define NUM_CHS2SCAN 3 //扫描几路ADC就相应的赋值即可
    #define CHECK_ORDER3()          (0xFF)
    #define CHECK_LAST_ORDER3()     (0x00)
    #define CHECK_VOLTAGE_CAP3()    (0xFF)
#endif
//**************************************

//选择使用何种通信方式 判断开启何种中断
//**************************************
//#define USE_RS485 0xB2
#define USE_CAN 0xB1
#ifdef  USE_CAN
    #define ON_INT()  {IEC2bits.C2IE = 1;C2INTE = 0xBF;C1INTE = 0;}  //C2INTE = 0xBF;
    #define OFF_INT() {IEC2bits.C2IE = 0;C2INTE = 0;C1INTE = 0;}
    #define APPLY_CAN    TRUE
    #define APPLY_485    FALSE

#elif   USE_RS485
    #define ON_INT()  {ON_UART_INT();}
    #define OFF_INT() {OFF_UART_INT();}
    #define APPLY_CAN    FALSE
    #define APPLY_485    TRUE

#endif
//**************************************

#define Reset() {__asm__ volatile ("RESET");}

#endif
