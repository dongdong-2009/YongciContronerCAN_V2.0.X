/** 
 * <p>application name： DeviceIO.c</p> 
 * <p>application describing： 对单片机的IO口进行初始化</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.07.07</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */
#include "../Header.h"
#include "DevicdIO.h"

/**
 * <p>Discription: [74HC165-IO口定义]</p>
 */
#define PL         LATDbits.LATD3
#define PL_DIR     TRISDbits.TRISD3
#define CE        LATDbits.LATD1
#define CE_DIR   TRISDbits.TRISD1
#define CP       LATDbits.LATD2
#define CP_DIR    TRISDbits.TRISD2
#define Q7        PORTDbits.RD4
#define Q7_DIR    TRISDbits.TRISD4

/**
 * <p>Discription: [595 继电器IO口定义]</p>
 */
#define IP595_DS       LATCbits.LATC14
#define IP595_DS_DIR   TRISCbits.TRISC14
#define IP595_SHCP     LATDbits.LATD0
#define IP595_SHCP_DIR TRISDbits.TRISD0
#define IP595_STCP     LATGbits.LATG2
#define IP595_STCP_DIR TRISGbits.TRISG2
#define IP595_MR     	 LATCbits.LATC13
#define IP595_MR_DIR   TRISCbits.TRISC13

/**
 * <p>Discription: [595 LEDIO口定义]</p>
 */
#define IP595_B_DS       LATDbits.LATD7
#define IP595_B_DS_DIR   TRISDbits.TRISD7
#define IP595_B_SHCP     LATFbits.LATF1
#define IP595_B_SHCP_DIR TRISFbits.TRISF1
#define IP595_B_STCP     LATFbits.LATF0
#define IP595_B_STCP_DIR TRISFbits.TRISF0
#define IP595_B_MR     	 LATGbits.LATG14
#define IP595_B_MR_DIR   TRISGbits.TRISG14

void HC595BSendData(uint16 SendVal);
void HC595SendData(uint16 SendVal);
void UpdateRelayIndicateState(uint16 port, uint8 state);

//输出状态LED1-LED6 Relay1 Relay7
uint16 g_LEDOutState = 0xFFFF;
uint16 g_RelayOutState = 0;
/**
 * 
 * <p>Function name: [InitDeviceIO]</p>
 * <p>Discription: [初始化基本的IO端口]</p>
 */
void InitDeviceIO(void)
{
    ADPCFG = 0xFFFF; //模拟端口全部作为数字端口
    INTCON1bits.NSTDIS = 1; //禁止中断嵌套
    ClrWdt();
    
    //IGBT引脚
    //A
    DRIVER_A_1_DIR = 0;
    DRIVER_B_1_DIR = 0;
    DRIVER_C_1_DIR = 0;
    DRIVER_D_1_DIR = 0;
    RESET_CURRENT_A();
    
    //IGBT引脚
    //B
    DRIVER_A_2_DIR = 0;
    DRIVER_B_2_DIR = 0;
    DRIVER_C_2_DIR = 0;
    DRIVER_D_2_DIR = 0;
    RESET_CURRENT_B();

    //IGBT引脚
    //C
    DRIVER_A_3_DIR = 0;
    DRIVER_B_3_DIR = 0;
    DRIVER_C_3_DIR = 0;
    DRIVER_D_3_DIR = 0;
    RESET_CURRENT_C();
    
    IP595_B_DS_DIR=0;
    IP595_B_SHCP_DIR =0;  
    IP595_B_STCP_DIR=0;
    IP595_B_MR_DIR=0;
    IP595_B_MR=1;
    
    IP595_DS_DIR=0;
    IP595_SHCP_DIR =0;  
    IP595_STCP_DIR=0;
    IP595_MR_DIR=0;
    IP595_MR=1;
    
    PL_DIR=0;
    CE_DIR=0;
    CP_DIR =0;
    Q7_DIR =1;
    
    RXD1_LASER_DIR = 1;
    RXD2_LASER_DIR = 1; 
    
    TXD1_LASER_DIR = 0;
    TXD2_LASER_DIR = 0;
    
    
    TXD1_LASER = 0;    
    TXD2_LASER = 0;
            
    RESET_CURRENT_A();
    RESET_CURRENT_B();
    RESET_CURRENT_C();
    
    InitInt2(); //初始化外部中断

    UpdateIndicateState(RUN_RELAY,RUN_LED,TURN_ON); //开启运行指示灯、继电器
}
/**
 * 
 * <p>Function name: [HC595BSendData]</p>
 * <p>Discription: [595控制LED灯函数]</p>
 * @param SendVal
 */
void HC595BSendData(uint16 SendVal)
{  
    uint8 i;
    for(i=0;i<16;i++) 
    {
        ClrWdt();
        if ((SendVal & 0x8000) == 0x8000)
        {
            IP595_B_DS = 1; 
        }
        else
        {
            IP595_B_DS = 0;
        }
        IP595_B_SHCP = 0;
        __delay_us(1);
        IP595_B_SHCP = 1;	
        __delay_us(1);
        SendVal = SendVal << 1;
    }  
    ClrWdt();
    IP595_B_STCP = 0; //set dataline low
    __delay_us(1);
    IP595_B_STCP = 1; 
    __delay_us(1);
}

/**
 * 
 * <p>Function name: [HC595SendData]</p>
 * <p>Discription: [595控制继电器函数]</p>
 * @param SendVal
 */
void HC595SendData(uint16 SendVal)
{ 
    uint8 i;
    for(i = 0;i < 16;i++) 
    {
        ClrWdt();
        if ((SendVal & 0x8000) == 0x8000)
        {
            IP595_DS = 1; 
        }
        else
        {
            IP595_DS = 0;
        }
        IP595_SHCP = 0;
        __delay_us(1);
        IP595_SHCP = 1;	
        __delay_us(1);
        SendVal = SendVal << 1;
    }  
    ClrWdt();
    IP595_STCP = 0; //set dataline low
    __delay_us(1);
    IP595_STCP = 1; 
    __delay_us(1);
}

/**
 * 
 * <p>Function name: [ReHC74165]</p>
 * <p>Discription: [读取165中的数值]</p>
 * @return 返回165的值
 */
unsigned long ReHC74165(void)
{  
    unsigned char i;
    unsigned long indata = 0;	 
    PL = 0;  
    __delay_us(1);
    PL = 1;
    
    CE = 0;
    __delay_us(1);
    for(i = 0; i < 24; i++)
    {
        ClrWdt();
        CP = 0;
        __delay_us(1);
        indata <<= 1;
        indata |= Q7;
        CP = 1;
        __delay_us(1);
    }
    PL = 1;
    CP = 1;
    CE = 1;
    ClrWdt();
    indata |= 0xFF000000;
    return(~indata);       
   
}

/**
 * @description 外部中断1初始化
 */
void InitInt2(void)
{
    IPC5bits.INT2IP = 7;    //外部中断优先级为最高优先级
//    INTCON2bits.INT2EP = 1; //负边沿触发中断
    INTCON2bits.INT2EP = 0;   //正边沿触发中断
    IFS1bits.INT2IF = 0;
    IEC1bits.INT2IE = 0;    //首先禁止中断
    
    ClrWdt();
}

/**
 * @description 外部中断1开启
 */
inline void TurnOnInt2(void)
{
    IFS1bits.INT2IF = 0;
    IEC1bits.INT2IE = 1;
}

/**
 * @description 外部中断1关闭
 */
inline void TurnOffInt2(void)
{
    IFS1bits.INT2IF = 0;
    IEC1bits.INT2IE = 0;
}

/**
 * 
 * <p>Function name: [UpdateLEDIndicateState]</p>
 * <p>Discription: [更新LED状态]</p>
 * @param port 从LED,Relay宏定义中选择
 * @param state 开启/关闭 TURN_ON/TURN_OFF
 */
void UpdateLEDIndicateState(uint16 port, uint8 state)
{
    //开启状态
    if (state == TURN_ON)
    {
        ClrWdt();
        g_LEDOutState &= port;
        HC595BSendData(g_LEDOutState);
    }
     if (state == TURN_OFF)
    {
        ClrWdt();
        g_LEDOutState |= ~port;     //取反即可关闭
        HC595BSendData(g_LEDOutState);
    }
}

/**
 * 
 * <p>Function name: [UpdateRelayIndicateState]</p>
 * <p>Discription: [更新继电器状态]</p>
 * @param port 从LED,Relay宏定义中选择
 * @param state 开启/关闭 TURN_ON/TURN_OFF
 */
void UpdateRelayIndicateState(uint16 port, uint8 state)
{
    //开启状态
    if (state == TURN_ON)
    {
        ClrWdt();
        g_RelayOutState |= port;
        HC595SendData(g_RelayOutState);
    }
     if (state == TURN_OFF)
    {
        ClrWdt();
        g_RelayOutState &= ~port;   //取反即可关闭
        HC595SendData(g_RelayOutState);
    }
}

/**
 * 
 * <p>Function name: [UpdateIndicateState]</p>
 * <p>Discription: [更新LED、继电器的状态]</p>
 * @param relayPort 继电器
 * @param ledPort   LED灯的位
 * @param state 状态，TURN_ON \ TURN_OFF
 */
void UpdateIndicateState(uint16 relayPort,uint16 ledPort,uint8 state)
{
    ClrWdt();
    UpdateRelayIndicateState(relayPort,state);
    ClrWdt();
    UpdateLEDIndicateState(ledPort,state);
}