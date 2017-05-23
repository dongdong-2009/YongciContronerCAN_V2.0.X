/** 
 * <p>application name： AdcSample.c</p> 
 * <p>application describing： 此文件为配置ADC，以便测量电容电压</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */
#include "../Header.h"
#include "AdcSample.h"


/********************************************
*函数名：AdcInit()
*形参：void
*返回值：void
*功能：初始化AN0 为软件触发采样
 * updata 2017-05-09 更改为自动采样、转换触发中断
**********************************************/
void AdcInit(void)
{
    ADPCFG = 0xFFFF;
    ClrWdt();
    ADPC(); //开启ADC功能的IO口
    
    TRISBbits.TRISB0 = 1;   // AN0
    TRISBbits.TRISB1 = 1;   // AN1
    TRISBbits.TRISB2 = 1;   // AN2
    TRISBbits.TRISB3 = 1;   // AN3
    
    ClrWdt();
    ADCHSbits.CH0NA = 0;    //通道0的负输入为Vref-
    
    ADCON1bits.ADON = 0;    //关闭ADC
    
    ADCON1bits.FORM   = 0;  //整数
    
    ADCON1bits.SSRC   = 0b111;  // 自动转换
 
    ADCON1bits.ASAM   = 1;      // 自动采样
    
    ClrWdt();
    ADCON2bits.VCFG = 0;    //参考电源
    
    ADCON2bits.CSCNA = 1;   // 开启扫描输入

    ADCON2bits.SMPI = (NUM_CHS2SCAN - 1);   // 完成四个采样\转换 后产生中断

    ClrWdt();
    ADCON2bits.BUFM = 0;    //配置为一个16字缓冲
    ADCON2bits.ALTS = 0;    //总是使用MUX A输入多路开关设置

    ADCON3bits.ADRC = 0;    // 时钟由系统时钟产生
    ADCON3bits.ADCS = 5;    // ADC Conversion Clock Tad = Tcy/2*(ADCS+1)= (1/4M/2) * 6 = 0.75us
    
    ADCSSL = 0x0000;    
    ADCS(); //扫描的通道数量

    ClrWdt();
    IFS0bits.ADIF = 0;			// Clear the A/D interrupt flag bit
    IEC0bits.ADIE = 0;			// Disable A/D interrupt
    ADCON1bits.ADON = 0;		// Turn on the A/D converter
    INTCON1bits.NSTDIS = 1;     //不允许中断嵌套
}
/********************************************
*函数名： SoftSampleOnce()
*形参：void
*返回值：uint16
*功能：软件启动转换，获取ADC值.
**********************************************/
void SoftSampleOnce(void)
{
    ADCON1bits.ADON = 1; //启动转换
    ClrWdt();
    //若10ms不能完成则启动看门狗复位
    while(!IFS0bits.ADIF)
    {
        
    }
    ClrWdt();
    IFS0bits.ADIF = 0;			// Clear the A/D interrupt flag bit
    ADCON1bits.ADON = 0;
}
