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

#define SEND_TIME       2000    //发送在线状态间隔时间 (ms)
#define GET_CAP_TIME    300     //按键扫描间隔时间
#define SCAN_TIME       2       //按键扫描间隔时间
#define GET_TEMP_TIME   20000   //获取温度数据时间   (ms)
#define OFFLINE_TIME    50000   //获取温度数据时间   (ms)


/**
 * 时间戳合集
 */
TimeStampCollect g_TimeStampCollect;    //状态时间

void InitSystemTime()
{
    g_TimeStampCollect.msTicks = 0;
    g_TimeStampCollect.getTempTime.delayTime = GET_TEMP_TIME;
    g_TimeStampCollect.getTempTime.startTime = 0;
    
    g_TimeStampCollect.sendDataTime.delayTime = SEND_TIME;
    g_TimeStampCollect.sendDataTime.startTime = 0;
    
    g_TimeStampCollect.scanTime.delayTime = SCAN_TIME;
    g_TimeStampCollect.scanTime.startTime = 0;
    
    g_TimeStampCollect.changeLedTime.delayTime = 500;
    g_TimeStampCollect.changeLedTime.startTime = 0;
    
    g_TimeStampCollect.getCapVolueTime.delayTime = GET_CAP_TIME;    
    g_TimeStampCollect.getCapVolueTime.startTime = 0;
    
    g_TimeStampCollect.overTime.delayTime = 0;    
    g_TimeStampCollect.overTime.startTime = 0;
    
    g_TimeStampCollect.canStartTime.delayTime = 0;
    g_TimeStampCollect.canStartTime.startTime = 0;
    
    g_TimeStampCollect.offlineTime.delayTime = OFFLINE_TIME;
    g_TimeStampCollect.offlineTime.startTime = 0;
    
    
    g_TimeStampCollect.refusalActionTime.delayTime = UINT32_MAX;
    g_TimeStampCollect.refusalActionTime.startTime = 0;
}

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
	if(startTime == UINT32_MAX)
	{
		return 0x00;
	}
    if (UINT32_MAX - delayTime < startTime) //判断是否溢出,若溢出则先进行判断是否超出一个周期
    {
        ClrWdt();
        if(g_TimeStampCollect.msTicks < startTime)//先判断是否小于startTime
        {
            ClrWdt();
            if (g_TimeStampCollect.msTicks >= (startTime + delayTime))
            {
                ClrWdt();
                return 0xFF;
            }
        }
    }
    else
    {
        ClrWdt();
        if (g_TimeStampCollect.msTicks >= startTime + delayTime)
        {
            return 0xFF;
        }                
    }
    return 0;
}
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
inline uint8_t IsOverTimeStamp(TimeStamp* pStamp)
{
	if(pStamp->startTime == UINT32_MAX)
	{
		return 0x00;
	}
    if (UINT32_MAX - pStamp->delayTime < pStamp->startTime) //判断是否溢出,若溢出则先进行判断是否超出一个周期
    {
        ClrWdt();
        if(g_TimeStampCollect.msTicks < pStamp->startTime)//先判断是否小于startTime
        {
            ClrWdt();
            if (g_TimeStampCollect.msTicks >= (pStamp->startTime + pStamp->delayTime))
            {
                ClrWdt();
                return 0xFF;
            }
        }
    }
    else
    {
        ClrWdt();
        if (g_TimeStampCollect.msTicks >= pStamp->startTime + pStamp->delayTime)
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
inline void DelayMs(uint32_t dlyTicks)
{
    uint32_t curTicks;  
    curTicks = g_TimeStampCollect.msTicks;
    ClrWdt();
    while ((g_TimeStampCollect.msTicks - curTicks) < dlyTicks)
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
    g_TimeStampCollect.msTicks ++;                        /* increment counter necessary in Delay() */    
}
