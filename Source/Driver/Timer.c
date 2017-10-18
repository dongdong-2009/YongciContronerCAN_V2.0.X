/**
 * @file Timer.c
 * @brief 配置定时器Timer
 * copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.
 * company： SOJO
 * @date 2017.06.5
 *
 * @author Zhangxiaomou
 * @version ver 1.0
 */
#include  "../Header.h"
#include "Timer.h"

//Timer2 周期计数
uint16_t g_TPR2Count = 0;

//Timer4 周期计数
uint16_t TPR4Count = 0;
/**
 * 
 * @fn InitTimer1
 * @brief TIMER1 定时器设置 用于永磁合分闸计时
 * @param ms 定时器周期
 * @drif 由于该定时器作为永磁分合闸的时间定时器，所以其中断优先级应该为最高
 */
void InitTimer1( unsigned int  ms)
{
    ClrWdt();
    T1CON = 0;
    IPC0bits.T1IP = 7;  //最高的优先级
    IFS0bits.T1IF = 0;
    IPC0bits.T1IP = 4; 

    T1CONbits.TCKPS = 0b10; //1:64
    T1CONbits.TCS = 0;
    T1CONbits.TGATE = 0;

    IFS0bits.T1IF = 0;
    
    TMR1 = 0;
    PR1 = (unsigned int)((float32_t)FCY/1000.00/64.0*(float32_t)ms)-1;
    ClrWdt();
} 
/**
 * 
 * @fn StartTimer1
 * @brief 启动定时器
 */
inline void StartTimer1(void)
{
    ClrWdt();
    IEC0bits.T1IE = 1; 
    T1CONbits.TON = 1;
}
/**
 * 
 * @fn StopTimer1
 * @brief 复位定时器
 */
inline  void StopTimer1(void)
{
    T1CON = 0;
    TMR1 = 0; 
    PR1 = 0;
    ClrWdt();
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 0;
}

/**
 * 
 * @fn InitTimer2
 * @brief TIMER2 定时器设置 —— 用于系统ms中断
 * @param ms 定时器周期
 */
void InitTimer2(uint16_t ms)
{
    ClrWdt();
    IPC1bits.T2IP = 1;  //最低的优先级
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = 0b11; //1:256
    T2CONbits.TCS = 0;  //Fcy = Fosc/4
    T2CONbits.TGATE = 0;

    IFS0bits.T2IF = 0;
    IEC0bits.T2IE = 0;
    TMR2 = 0;
    
    PR2 = (unsigned int)((float32_t)FCY/1000.00/256.0*(float32_t)ms)-1;
    ClrWdt();
    g_TPR2Count = PR2;
    T2CONbits.TON = 0;

}
/**
 * 
 * @fn StartTimer2
 * @brief 启动定时器
 */
inline void StartTimer2(void)
{
    IFS0bits.T2IF = 0;
    TMR2 = 0;
    PR2 =  g_TPR2Count;
    ClrWdt();
    IEC0bits.T2IE = 1;
    T2CONbits.TON = 1;
}
/**
 * 
 * @fn StopTimer2
 * @brief 停止定时器
 */
inline void StopTimer2(void)
{
    T2CONbits.TON = 0;
    IEC0bits.T2IE = 0;
    IFS0bits.T2IF = 0;
}

/**
 * 
 * @fn InitTimer3
 * @brief TIMER3用于永磁同步合闸偏移时间计时
 * @param ms 定时器周期
 */
void InitTimer3(void)
{
    ClrWdt();
    T3CON = 0;
    IPC1bits.T3IP = 7;  //最高的优先级
    IFS0bits.T3IF = 0;

    T3CONbits.TCKPS = 0b01; //1:8
    T3CONbits.TCS = 0;
    T3CONbits.TGATE = 0;

    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;  //允许中断
    ClrWdt();
    
    TMR3 = 0;
} 
/**
 * 
 * @fn StartTimer3
 * @brief 启动定时器
 */
inline void StartTimer3(unsigned int us)
{
    TMR3 = 0;
    PR3 = us/2 + 2;   //误差量
    ClrWdt();
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    T3CONbits.TON = 1;
}
/**
 * 
 * @fn StartTimer3
 * @brief 启动定时器
 * @param us 延时时间
 */
inline void ChangeTimerPeriod3(unsigned int us)
{
    T3CONbits.TON = 0;
    IFS0bits.T3IF = 0;
    TMR3 = 0;
    PR3 = us/2 + 2;   //误差量
    T3CONbits.TON = 1;
}
/**
 * 
 * @fn StopTimer3
 * @brief 复位定时器
 */
inline  void StopTimer3(void)
{
    ClrWdt();
    T3CONbits.TON = 0;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 0;
}


/**
 * 
 * @fn InitTimer4
 * @brief 初始化定时器4，用于脉冲宽度测试，1:1定时器脉冲宽度/p>
 * @param ms 定时器周期
 */
void InitTimer4()
{
    ClrWdt();
   
    IPC5bits.T4IP = 2;  //优先级较低
    IFS1bits.T4IF = 0;

    T4CON = 0;
    T4CONbits.TCKPS = 0b01; //1：8
    T4CONbits.TCS = 0;
    T4CONbits.TGATE = 0;

    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 0;  //不允许中断
    ClrWdt();
    
    
    TMR4 = 0;     
 //   PR4 = (unsigned int)((float32_t)FCY/256.0*(float32_t)us)-1;
    PR4 = 0xFFFF;
    ClrWdt();
    TPR4Count = PR2;
    T4CONbits.TON = 0;
    
    
} 

/**
 * 改变定时器4的定时周期
 * @param us  定时时间us
 */
inline void ChangeTimerPeriod4(unsigned int us)
{
  
    TPR4Count = (unsigned int)((float32_t)FCY/(float32_t)us)-1;
    PR4 =  TPR4Count;
}
inline uint16_t GetTimeUs(void)
{
    return TMR4 * 2; //1:1分频比1:8 8/4
}
/**
 * 
 * @fn StartTimer4
 * @brief 启动定时器
 */
inline void StartTimer4(void)
{
    T4CONbits.TON = 0;
    ClrWdt();
    TMR4 = 0;    
    PR4 =  0xFFFF;
    IFS1bits.T4IF = 0;
    T4CONbits.TON = 1;
}
/**
 * 
 * @fn StopTimer4
 * @brief 复位定时器
 */
inline  void StopTimer4(void)
{
    ClrWdt();
    T4CONbits.TON = 0;
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 0;
}




