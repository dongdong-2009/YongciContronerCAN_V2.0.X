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


void HC595BSendData(uint16_t SendVal);
void HC595SendData(uint16_t SendVal);
void UpdateRelayIndicateState(uint16_t port, uint8_t state);
void AllHC595SendData(uint16_t lValue , uint16_t rValue);

//输出状态LED1-LED6 Relay1 Relay7
uint16_t LEDOutState = 0xFFFF;
uint16_t RelayOutState = 0;
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
    DECTA_DIR = 0;
    RESET_CURRENT_A();
    ClrWdt();
    
    //IGBT引脚
    //B
    DRIVER_A_2_DIR = 0;
    DRIVER_B_2_DIR = 0;
    DRIVER_C_2_DIR = 0;
    DRIVER_D_2_DIR = 0;
    DECTB_DIR = 0;
    RESET_CURRENT_B();
    ClrWdt();

    //IGBT引脚
    //C
    DRIVER_A_3_DIR = 0;
    DRIVER_B_3_DIR = 0;
    DRIVER_C_3_DIR = 0;
    DRIVER_D_3_DIR = 0;
    DECTC_DIR = 0;
    RESET_CURRENT_C();
    ClrWdt();
    
    IP595_B_DS_DIR=0;
    IP595_B_SHCP_DIR =0;  
    IP595_B_STCP_DIR=0;
    IP595_B_MR_DIR=0;
    IP595_B_MR=1;
    ClrWdt();
    
    IP595_DS_DIR=0;
    IP595_SHCP_DIR =0;  
    IP595_STCP_DIR=0;
    IP595_MR_DIR=0;
    IP595_MR=1;
    ClrWdt();
    
    ClrWdt();
    UpdateIndicateState(RUN_RELAY, RUN_LED, TURN_ON); //开启运行指示灯和指示继电器   
    ClrWdt();
    
    PL_DIR=0;
    CE_DIR=0;
    CP_DIR =0;
    Q7_DIR =1;
    ClrWdt();
    
    RXD1_LASER_DIR = 1;
    RXD2_LASER_DIR = 1; 
    
    TXD1_LASER_DIR = 0;
    TXD2_LASER_DIR = 0;
    ClrWdt();
    
    
    TXD1_LASER = 0;    
    TXD2_LASER = 0;
            
    RESET_CURRENT_A();
    RESET_CURRENT_B();
    RESET_CURRENT_C();
    ClrWdt();
    
    SCL_DIR = 0;
    SCL = 1;
            
    InitInt2(); //初始化外部中断

    ClrWdt();
}
/**
 * 
 * <p>Function name: [HC595BSendData]</p>
 * <p>Discription: [595控制LED灯函数]</p>
 * @param SendVal
 */
void HC595BSendData(uint16_t SendVal)
{  
    uint8_t i;
    for(i=0;i<16;i++) 
    {
        ClrWdt();
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
        Nop();
        IP595_B_SHCP = 1;	
        Nop();
        SendVal = SendVal << 1;
    }  
    ClrWdt();
    IP595_B_STCP = 0; //set dataline low
    Nop();
    IP595_B_STCP = 1; 
    Nop();
}

/**
 * 
 * <p>Function name: [HC595SendData]</p>
 * <p>Discription: [595控制继电器函数]</p>
 * @param SendVal
 */
void HC595SendData(uint16_t SendVal)
{ 
    uint8_t i;
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
        Nop();
        IP595_SHCP = 1;	
        Nop();
        SendVal = SendVal << 1;
    }  
    ClrWdt();
    IP595_STCP = 0; //set dataline low
    Nop();
    IP595_STCP = 1; 
    Nop();
}

/**
 * 
 * <p>Function name: [AllHC595SendData]</p>
 * <p>Discription: [595控制继电器函数]</p>
 * @param lValue
 * @param rValue
 */
void AllHC595SendData(uint16_t lValue , uint16_t rValue)
{
    uint8_t i = 0;
    ClrWdt();
    for(i = 0; i < 16; i++)
    {
        IP595_DS = (lValue & 0x8000) ? 1 : 0;
        IP595_B_DS = (rValue & 0x8000) ? 1 : 0;
        //产生上升沿
        IP595_SHCP = LOW;
        IP595_B_SHCP = LOW;
        Nop();
        IP595_SHCP = HIGH;
        IP595_B_SHCP = HIGH;
        Nop();
        lValue = lValue << 1; 
        rValue = rValue << 1;
    }
    IP595_STCP = 0; //set dataline low
    IP595_B_STCP = 0; //set dataline low
    Nop();
    IP595_STCP = 1; 
    IP595_B_STCP = 1; 
    Nop();
}

/**
 * 
 * <p>Function name: [ReadHC165]</p>
 * <p>Discription: [读取165中的数值]</p>
 * @return 返回165的值
 */
uint32_t ReadHC165(void)
{  
    uint8_t i;
    uint32_t indata = 0;	 
    
    ClrWdt();
    PL = 0;  
    Nop();
    PL = 1;
    
    CE = 0;
    Nop();
    for(i = 0; i < 24; i++)
    {
        ClrWdt();
        CP = 0;
        Nop();
        indata <<= 1;
        indata |= Q7;
        CP = 1;
        Nop();
    }
    PL = 1;
    CP = 1;
    CE = 1;
    ClrWdt();
    indata |= 0xFF000000;
    return(~indata);       
   
}

/**
 * @description 外部中断2初始化
 */
void InitInt2(void)
{
    
    ClrWdt();
    IPC5bits.INT2IP = 7;    //外部中断优先级为最高优先级
//    INTCON2bits.INT2EP = 1; //负边沿触发中断
    INTCON2bits.INT2EP = 0;   //正边沿触发中断
    IFS1bits.INT2IF = 0;
    IEC1bits.INT2IE = 0;    //首先禁止中断
    
    
}

/**
 * 
 * <p>Function name: [TurnOnInt2]</p>
 * <p>Discription: [开启外部中断2]</p>
 */
inline void TurnOnInt2(void)
{
    IFS1bits.INT2IF = 0;
    IEC1bits.INT2IE = 1;
}

/**
 * 
 * <p>Function name: [TurnOffInt2]</p>
 * <p>Discription: 关闭外部中断2，与此同时关闭中断3</p>
 */
inline void TurnOffInt2(void)
{
    IFS1bits.INT2IF = 0;
    IEC1bits.INT2IE = 0;
    
   
}
/**
 * @description 外部中断3初始化
 */
void InitInt3(void)
{
    
    ClrWdt();
    IPC9bits.INT3IP = 6;    //优先级次于同步脉冲
//    INTCON2bits.INT3EP = 1; //负边沿触发中断
    INTCON2bits.INT3EP = 0;   //正边沿触发中断
    IFS2bits.INT3IF = 0;
    IEC2bits.INT3IE = 0;    //首先禁止中断
    
}

/**
 * 
 * <p>Function name: [TurnOnInt3]</p>
 * <p>Discription: [开启外部中断3]</p>
 */
inline void TurnOnInt3(void)
{
    IFS2bits.INT3IF = 0;
    IEC2bits.INT3IE = 1;
}

/**
 * 
 * <p>Function name: [TurnOffInt3]</p>
 * <p>Discription: [关闭外部中断3]</p>
 */
inline void TurnOffInt3(void)
{
    IFS2bits.INT3IF = 0;
    IEC2bits.INT3IE = 0;
}
/**
 * 
 * <p>Function name: [UpdateLEDIndicateState]</p>
 * <p>Discription: [更新LED状态]</p>
 * @param port 从LED,Relay宏定义中选择
 * @param state 开启/关闭 TURN_ON/TURN_OFF
 */
void UpdateLEDIndicateState(uint16_t port, uint8_t state)
{
    //开启状态
    if (state == TURN_ON)
    {
        ClrWdt();
        LEDOutState &= port;
    }
    else if (state == TURN_OFF)
    {
        ClrWdt();
        LEDOutState |= ~port;     //取反即可关闭
    }
    else
    {
        return;
    }
    HC595BSendData(LEDOutState);
}

/**
 * 
 * <p>Function name: [UpdateRelayIndicateState]</p>
 * <p>Discription: [更新继电器状态]</p>
 * @param port 从LED,Relay宏定义中选择
 * @param state 开启/关闭 TURN_ON/TURN_OFF
 */
void UpdateRelayIndicateState(uint16_t port, uint8_t state)
{
    //开启状态
    if (state == TURN_ON)
    {
        ClrWdt();
        RelayOutState |= port;
    }
    else if (state == TURN_OFF)
    {
        ClrWdt();
        RelayOutState &= ~port;   //取反即可关闭
    }
    else
    {
        return;
    }
    HC595SendData(RelayOutState);
}

/**
 * 
 * <p>Function name: [UpdateIndicateState]</p>
 * <p>Discription: [更新LED、继电器的状态]</p>
 * @param relayPort 继电器
 * @param ledPort   LED灯的位
 * @param state 状态，TURN_ON \ TURN_OFF
 */
void UpdateIndicateState(uint16_t relayPort, uint16_t ledPort, uint8_t state)
{
    ClrWdt();
    //开启状态
    if (state == TURN_ON)
    {
        ClrWdt();
        LEDOutState &= ledPort;
        RelayOutState |= relayPort;
    }
    else if (state == TURN_OFF)
    {
        ClrWdt();
        LEDOutState |= ~ledPort;      //取反即可关闭
        RelayOutState &= ~relayPort;   //取反即可关闭
    }
    else
    {
        return;
    }
    AllHC595SendData(RelayOutState , LEDOutState);  //LED & Relay Synchronize Action
    ClrWdt();
}

