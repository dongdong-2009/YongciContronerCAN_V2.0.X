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

uint32_t g_MsTicks = 0;

SysTimeStamp g_SysTimeStamp;    //状态时间

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
inline uint8_t IsOverTime(uint32_t startTime, uint32_t delayTime)
{
    if (UINT32_MAX - delayTime < startTime) //判断是否溢出,若溢出则先进行判断是否超出一个周期
    {
        ClrWdt();
        if(g_MsTicks < startTime)//先判断是否小于startTime
        {
            ClrWdt();
            if (g_MsTicks >= (delayTime + startTime))
            {
                ClrWdt();
                return 0xFF;
            }
        }
    }
    else
    {
        ClrWdt();
        if (g_MsTicks >= startTime + delayTime)
        {
            return 0xFF;
        }                
    }
    return 0;
}

/**
 * 
 * <p>Function name: [OverflowDetection]</p>
 * <p>Discription: [检查系统计数是否会溢出]</p>
 */
inline void OverflowDetection(uint32_t delayTime)
{
    if(UINT32_MAX - g_SysTimeStamp.TickTime <= delayTime) //需要延时等待的时间是否会使系统时钟计数溢出
    {
        g_SysTimeStamp.TickTime = 0;  //会溢出则先进行清零
    }
}

/**
 * 
 * <p>Function name: [Delay_ms]</p>
 * <p>Discription: [delays number of tick Systicks ]</p>
 * @param ms 延时时间ms
 */
inline void Delay_ms(uint32_t dlyTicks)
{
    uint32_t curTicks;
    OverflowDetection(dlyTicks);    //溢出检测
    curTicks = g_SysTimeStamp.TickTime;
    ClrWdt();
    while ((g_SysTimeStamp.TickTime - curTicks) < dlyTicks)
    {
        ClrWdt();
    }
}

/**
 * 
 * <p>Function name: [_T2Interrupt]</p>
 * <p>Discription: [定时器2的中断函数，实现时间的累加]</p>
 */
void __attribute__((interrupt, no_auto_psv)) _T2Interrupt(void)
{
    IFS0bits.T2IF = 0;
    g_MsTicks ++;                        /* increment counter necessary in Delay() */  
    g_SysTimeStamp.TickTime ++;
}
