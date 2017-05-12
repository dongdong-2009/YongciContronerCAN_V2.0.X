#define FCY 4e6 
#include "Timer.h"
#include <xc.h>
#include "tydef.h"

//Timer2 周期计数
uint16 g_TPRCount = 0;

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
} 
/**
 * 
 * <p>Function name: [StartTimer1]</p>
 * <p>Discription: [启动定时器]</p>
 */
inline void StartTimer1(void)
{
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
    g_TPRCount = PR2;
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
    PR2 =  g_TPRCount;
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



