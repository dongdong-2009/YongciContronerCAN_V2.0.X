/** 
 * <p>application name： Delay.c</p> 
 * <p>application describing： 使用定时器延时</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.07.11</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */

#include "../Header.h"
#include "Delay.h"

#define UINT32_MAX  0xFFFFFFFF

uint32 g_MsTicks = 0;
uint8 g_ScanTime = 0;   //按键扫描间隔时间
SysTimeStamp g_SysTimeStamp;
/**
 * 判断时间是否超时
 *
 * @param   startTime 启动时间
 * @param   delayTime 延时时间
 * 
 * @return  0xFF-时间到达 0-时间还未到达
 *
 * @bref   比较时间是否达到设定值，对溢出进行超时判断
 */
inline uint8 IsOverTime(uint32 startTime, uint32 delayTime)
{
    if (UINT32_MAX - delayTime < startTime) //判断是否溢出,若溢出则先进行判断是否超出一个周期
    {
        if( g_MsTicks < startTime)//先判断是否小于startTime
        {
            if (g_MsTicks >= (delayTime + startTime))
            {
                return 0xFF;
            }
        }
    }
    else
    {
        if (g_MsTicks >= startTime + delayTime)
        {
            return 0xFF;
        }                
    }
    return 0;
}

/**
 * 
 * <p>Function name: [Delay_ms]</p>
 * <p>Discription: [delays number of tick Systicks ]</p>
 * @param ms 延时时间ms
 */
inline void Delay_ms(uint32 dlyTicks)
{
    uint32 curTicks;
    curTicks = g_MsTicks;
    while ((g_MsTicks - curTicks) < dlyTicks);
}

/**
 * 
 * <p>Function name: [_T2Interrupt]</p>
 * <p>Discription: [定时器2的中断函数，实现时间的累加]</p>
 */
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    g_MsTicks++;                        /* increment counter necessary in Delay() */    
    g_ScanTime++;
}
