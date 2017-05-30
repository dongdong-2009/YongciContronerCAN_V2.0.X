/** 
 * <p>application name： Timer.c</p> 
 * <p>application describing： 配置定时器Timer</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */

#include  "../Header.h"
#include "Timer.h"

//Timer2 周期计数
uint16 g_TPR2Count = 0;

/**
 * 
 * <p>Function name: [Init_Timer1]</p>
 * <p>Discription: [TIMER1 定时器设置 用于永磁合分闸计时]</p>
 * @param ms 定时器周期
 * @drif 由于该定时器作为永磁分合闸的时间定时器，所以其中断优先级应该为最高
 */
void Init_Timer1( unsigned int  ms)
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
    PR1 = (unsigned int)((float)FCY/1000.00/64.0*(float)ms)-1;
    ClrWdt();
} 
/**
 * 
 * <p>Function name: [StartTimer1]</p>
 * <p>Discription: [启动定时器]</p>
 */
inline void StartTimer1(void)
{
    ClrWdt();
    IEC0bits.T1IE = 1; 
    T1CONbits.TON = 1;
}
/**
 * 
 * <p>Function name: [ResetTimer1]</p>
 * <p>Discription: [复位定时器]</p>
 */
inline  void ResetTimer1(void)
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
 * <p>Function name: [SetTimer2]</p>
 * <p>Discription: [TIMER2 定时器设置 —— 用于通讯超时检测]</p>
 * @param ms 定时器周期
 */
void SetTimer2(uint16 ms)
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
    
    PR2 = (unsigned int)((float)FCY/1000.00/256.0*(float)ms)-1;
    ClrWdt();
    g_TPR2Count = PR2;
    T2CONbits.TON = 0;

}
/**
 * 
 * <p>Function name: [StartTimer2]</p>
 * <p>Discription: [启动定时器]</p>
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
 * <p>Function name: [StopTimer2]</p>
 * <p>Discription: [停止定时器]</p>
 */
inline void StopTimer2(void)
{
    T2CONbits.TON = 0;
    IEC0bits.T2IE = 0;
    IFS0bits.T2IF = 0;
}

/**
 * 
 * <p>Function name: [Init_Timer3]</p>
 * <p>Discription: [TIMER3用于永磁同步合闸偏移时间计时]</p>
 * @param ms 定时器周期
 */
void Init_Timer3(void)
{
    ClrWdt();
    T1CON = 0;
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
 * <p>Function name: [StartTimer3]</p>
 * <p>Discription: [启动定时器]</p>
 */
inline void StartTimer3(unsigned int us)
{
    PR3 = us + 2;   //误差量
    ClrWdt();
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 1;
    T3CONbits.TON = 1;
}
/**
 * 
 * <p>Function name: [ResetTimer3]</p>
 * <p>Discription: [复位定时器]</p>
 */
inline  void ResetTimer3(void)
{
    ClrWdt();
    T3CONbits.TON = 0;
    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 0;
}


/**
 * 
 * <p>Function name: [Init_Timer4]</p>
 * <p>Discription: [初始化定时器4，用于同步信号判断]/p>
 * @param ms 定时器周期
 */
void Init_Timer4(void)
{
    ClrWdt();
    T1CON = 0;
    IPC5bits.T4IP = 2;  //优先级较低
    IFS1bits.T4IF = 0;

    T4CONbits.TCKPS = 0b00; //1:1
    T4CONbits.TCS = 0;
    T4CONbits.TGATE = 0;

    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 0;  //不允许中断
    ClrWdt();
    
    TMR4 = 0;
} 
/**
 * 
 * <p>Function name: [StartTimer4]</p>
 * <p>Discription: [启动定时器]</p>
 */
inline void StartTimer4(void)
{
    ClrWdt();
    TMR4 = 0;
    IFS1bits.T4IF = 0;
    T4CONbits.TON = 1;
}
/**
 * 
 * <p>Function name: [ResetTimer4]</p>
 * <p>Discription: [复位定时器]</p>
 */
inline  void ResetTimer4(void)
{
    ClrWdt();
    TMR4 = 0;
}

/**
 * 
 * <p>Function name: [ResetTimer4]</p>
 * <p>Discription: [关闭定时器]</p>
 */
inline  void StopTimer4(void)
{
    ClrWdt();
    TMR4 = 0;
    IFS1bits.T4IF = 0;
    T4CONbits.TON = 0;
}