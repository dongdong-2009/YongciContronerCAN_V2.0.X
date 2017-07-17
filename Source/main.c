/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Main.c
*文件标识:
 * 大改日期：2015/7/13
*创建日期： 2015年4月23日 
*摘要:
 * 2016/12/15:更换调试器后调试，调试驱动可用
 * 2016/12/3:将程序修改为适应新硬件版本，修改相当一部分端口
 * 2015/11/29:屏蔽通讯无效果。改为11ms软件检测.
 * 2015/10/30:添加写EEPROM超时复位，超过20ms停止等待跳出。
 * 
 * 2015/10/25：取消CO时候的电压检测
 *      
 * 2015/10/22:添加合分闸锁，只有在执行命令后同时上锁，才能执行动作。
 *            添加EEPROM计数，并设置下载程序保留EEPROM
 * 2015/10/21：测试版本，测试合分闸时序.
 * 2015/10/19:注意第二次做板晶振由16M改为4M，晶振模式为XT，4倍频。
 *             合闸时间50ms，分闸时间30ms
 * 2015/10/15: 进一步优化接收程序，试图消灭接收机问题。
 * 2015/10/14:
 *  调试完毕ADC转换 
 *  修正闭锁功能。
 * 2015/10/13:减少非按钮等待时间，由20ms改为2ms。取消对命令重复执行结构.
 *             添加防跳与闭锁功能.——有待测试.
 * 2015/8/12：添加抬起检测，本地控制.
 * 2015/8/10:完善远本转换。
 * 2015/8/6:添加合闸分闸闭锁功能，一次按钮动作只能进行一次操作。
 * 2015/8/3:添加看门狗1:1 1：1 2ms以内喂狗
 * 2015/8/2:发现通讯不能正常接收，经跟踪发现系循环延时所致，遂更改循环优先级。
 * 2015/7/3: 改为F4011 ,外部晶振16M，主频4M
 * 
 * 2015/7/2:完善生成帧的CRC校验。增大发送延时.
 * 2015/7/1:发现上位机不能修改分闸时间需要进一步确认问题所在.
 * 2015/6/17: 添加动作中断控制。合分闸时关闭通讯与外部中断。
 * 添加等待状态。等待中断接收分合闸指令。
*2015/4/27:改FOSC为PLL16，M = FOSC/4 = 16M
*2015/4/23:开始编码；基于外部晶振4M，PLL4
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
// DSPIC30F6012A Configuration Bit Settings

// 'C' source line config statements

// FOSC
#pragma config FOSFPR = XT_PLL4         // Oscillator (XT w/PLL 4x)
#pragma config FCKSMEN = CSW_FSCM_OFF   // Clock Switching and Monitor (Sw Disabled, Mon Disabled)

// FWDT
#pragma config FWPSB = WDTPSB_5         // WDT Prescaler B (1:5)
#pragma config FWPSA = WDTPSA_1         // WDT Prescaler A (1:1)
#pragma config WDT = WDT_ON             // Watchdog Timer (Enabled)
//#pragma config WDT = WDT_OFF            // Watchdog Timer (Disabled)

// FBORPOR
#pragma config FPWRT = PWRT_64          // POR Timer Value (64ms)
#pragma config BODENV = BORV_42         // Brown Out Voltage (4.2V)
#pragma config BOREN = PBOR_ON          // PBOR Enable (Enabled)
#pragma config MCLRE = MCLR_EN          // Master Clear Enable (Enabled)

// FBS
#pragma config BWRP = WR_PROTECT_BOOT_OFF// Boot Segment Program Memory Write Protect (Boot Segment Program Memory may be written)
#pragma config BSS = NO_BOOT_CODE       // Boot Segment Program Flash Memory Code Protection (No Boot Segment)
#pragma config EBS = NO_BOOT_EEPROM     // Boot Segment Data EEPROM Protection (No Boot EEPROM)
#pragma config RBS = NO_BOOT_RAM        // Boot Segment Data RAM Protection (No Boot RAM)

// FSS
#pragma config SWRP = WR_PROT_SEC_OFF   // Secure Segment Program Write Protect (Disabled)
#pragma config SSS = NO_SEC_CODE        // Secure Segment Program Flash Memory Code Protection (No Secure Segment)
#pragma config ESS = NO_SEC_EEPROM      // Secure Segment Data EEPROM Protection (No Segment Data EEPROM)
#pragma config RSS = NO_SEC_RAM         // Secure Segment Data RAM Protection (No Secure RAM)

// FGS
#pragma config GWRP = GWRP_OFF          // General Code Segment Write Protect (Disabled)
#pragma config GCP = GSS_OFF            // General Segment Code Protection (Disabled)

// FICD
#pragma config ICS = ICS_PGD            // Comm Channel Select (Use PGC/EMUC and PGD/EMUD)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>


#include "Header.h"
#include "DeviceNet/DeviceNet.h"


int main()
{
    uint16_t cn = 0;
    InitDeviceIO(); //IO初始化 首先禁止中断   517cys
    //延时3s判断启动
    while(cn++ < 3000)
    {
        __delay_ms(1);
        if((cn % 100) == 0)
        {
            InitDeviceIO(); //IO初始化 首先禁止中断   517cys
        }
        ClrWdt();
    }
    ClrWdt();

    AdcInit(); //ADC采样初始化
    
    InitTimer2(1);  //系统心跳时钟，优先级为1，时钟1ms
    InitTimer3();   //用于永磁控制器的同步合闸偏移时间，精度2us    
    InitSystemTime();     //初始化系统时间    
    StartTimer2();  //开启系统时钟
    ClrWdt(); 
    ActionParameterInit();
    //SD2405_Init();  //时钟芯片初始化    

    BufferInit();     
    InitStandardCAN(0, 0);      //初始化CAN模块
    ClrWdt();
    InitDeviceNet();            //初始化DeviceNet服务
    ClrWdt();
    RefParameterInit(); //参数设置初始化
        
    YongciFirstInit();      //永磁合闸参数初始化
    ClrWdt(); //33cys    
    UpdateIndicateState(RUN_RELAY, RUN_LED, TURN_ON); //开启运行指示灯和指示继电器    
    while(TRUE)
    {
        ClrWdt();
        YongciMainTask();
    }
}
  