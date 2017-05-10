#define FCY 4e6 
#include "Timer.h"
#include <xc.h>
#include "tydef.h"

//Timer2 周期计数
uint16 g_TPRCount = 0;

//Timer3 周期计数
uint16 g_TPR3Count = 0;

//Timer4 周期计数
uint16 g_TPR4Count = 0;

//Timer5 周期计数
uint16 g_TPR5Count = 0;

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

    //INTCON1bits.NSTDIS = 1;
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
    IPC1bits.T2IP = 0;//最高的优先级
    T2CONbits.TON = 0;
    T2CONbits.TCKPS = 0b11; //1:256
    T2CONbits.TCS = 0; //Fcy = Fosc/4
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
    IFS0bits.T2IF = 0;
}

/**
 * 
 * <p>Function name: [SetTimer3]</p>
 * <p>Discription: [TIMER3 定时器设置，注意最大定时时间 —— 用于DeviceNet]</p>
 * @param ms 定时器周期
 */
 void SetTimer3(uint16 ms)
{
    ClrWdt();
    IPC1bits.T3IP = 1;//优先级
    T3CONbits.TON = 0;
    T3CONbits.TCKPS = 0b11; //1:256
    T3CONbits.TCS = 0; //Fcy = Fosc/4
    T3CONbits.TGATE = 0;

    IFS0bits.T3IF = 0;
    IEC0bits.T3IE = 0;
    TMR3 = 0;
    
    PR3 = (unsigned int)((float)FCY/1000.00/256.0*(float)ms)-1;
    g_TPR3Count = PR3;
    T3CONbits.TON = 0;
}
 
/**
 * 
 * <p>Function name: [StartTimer3]</p>
 * <p>Discription: [启动定时器]</p>
 */
inline void StartTimer3(void)
{
    IFS0bits.T3IF = 0;
    TMR3 = 0;
    PR3 =  g_TPR3Count;
    T3CONbits.TON = 1;
}
/**
 * 
 * <p>Function name: [StopTimer3]</p>
 * <p>Discription: [停止定时器]</p>
 */
inline void StopTimer3(void)
{
    T3CONbits.TON = 0;
    IFS0bits.T3IF = 0;
}

/**
 * 
 * <p>Function name: [SetTimer4]</p>
 * <p>Discription: [定时器设置]</p>
 * @param ms 定时时间
 */
void SetTimer4(uint16 ms)
{
    ClrWdt();
    IPC5bits.T4IP = 1;  //优先级
    T4CONbits.TON = 0;
    T4CONbits.TCKPS = 0b11; //1:256
    T4CONbits.TCS = 0; //Fcy = Fosc/4
    T4CONbits.TGATE = 0;

    //INTCON1bits.NSTDIS = 1;// 允许嵌套
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 0;
    TMR4 = 0;
    
    PR4 = (unsigned int)((float)FCY/1000.00/256.0*(float)ms)-1;
    g_TPR4Count = PR4;
    T4CONbits.TON = 0;
}
 
/**
 * 
 * <p>Function name: [StartTimer4]</p>
 * <p>Discription: [启动定时器]</p>
 */
inline void StartTimer4(void)
{
    IFS1bits.T4IF = 0;
    IEC1bits.T4IE = 1;
    TMR4 = 0;
    PR4 =  g_TPR4Count;
    T4CONbits.TON = 1;
}
/**
 * 
 * <p>Function name: [StopTimer4]</p>
 * <p>Discription: [停止定时器]</p>
 */
inline void StopTimer4(void)
{
    T4CONbits.TON = 0;
    IEC1bits.T4IE = 0;
    IFS1bits.T4IF = 0;
}

/**
 * 
 * <p>Function name: [SetTimer5]</p>
 * <p>Discription: [用于按键扫描与消抖，总共的检测次数为50次]</p>
 * @param ms 定时时间
 */
 void SetTimer5(uint16 ms)
{
    ClrWdt();
    IPC5bits.T5IP = 1;  //优先级
    T5CONbits.TON = 0;
    T5CONbits.TCKPS = 0b11; //1:256
    T5CONbits.TCS = 0;  //Fcy = Fosc/4
    T5CONbits.TGATE = 0;

    IFS1bits.T5IF = 0;
    IEC1bits.T5IE = 0;
    TMR5 = 0;
    
    PR5 = (unsigned int)((float)FCY/1000.00/256.0*(float)ms)-1;
    g_TPR5Count = PR5;
    T5CONbits.TON = 0;
}
/**
 * 
 * <p>Function name: [StartTimer5]</p>
 * <p>Discription: [启动定时器]</p>
 */
inline void StartTimer5(void)
{
    IFS1bits.T5IF = 0;
    IEC1bits.T5IE = 1;
    TMR5 = 0;
    PR5 =  g_TPR5Count;
    T5CONbits.TON = 1;
}
/**
 * 
 * <p>Function name: [StopTimer5]</p>
 * <p>Discription: [停止定时器]</p>
 */
inline void StopTimer5(void)
{
    T5CONbits.TON = 0;
    IEC1bits.T5IE = 0;
    IFS1bits.T5IF = 0;
}
