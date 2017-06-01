/** 
 * <p>application name： CAN.c</p> 
 * <p>application describing： CAN配置程序</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author FreeGo 
 * @version ver 1.0
 */
#include "../Header.h"
#include "CAN.h"
#include "../DeviceNet/DeviceNet.h"
#include "../SerialPort/RefParameter.h"

#define BITRATE 	100000			 			// 100K
#define NTQ 		16							// Number of Tq cycles which will make the 
												//CAN Bit Timing .
#define BRP_VAL		(((float)FCY * 4/(2*NTQ*BITRATE))-1)  //Formulae used for C2CFG1bits.BRP 

void ConfigCANOneMaskFilterRX1(EIDBits* pRm1, EIDBits* pRf2);
void ConfigCANOneMaskFilterRX0(EIDBits* pRm0, EIDBits* pRf0);
void GetReciveRX0EID(EIDBits* pEID);
void ConfigCANOneBraud(void);


/********************************************
*函数名： ConfigCANOneBraud()
*形参：void
*返回值：void
*功能：配置CAN1 波特率有关
**********************************************/
inline void ConfigCANOneBraud(void)
{
    C2CTRLbits.CANCKS = 1;			// Select the CAN Master Clock . It is equal to Fcy here. 
                                    // equal to Fcy.(Fcy=30Mhz)
    C2CFG1bits.SJW=00;				//1TQ 同步跳转宽度时间 Synchronized jump width time is 1 x TQ when SJW is equal to 00
    C2CFG1bits.BRP = BRP_VAL;		//波特率预分频比位 ((FCY/(2*NTQ*BITRATE))-1) 	
    ClrWdt();
    //C2CFG2 = 0x03F5;               // SEG1PH=6Tq, SEG2PH=3Tq, PRSEG=5Tq    
                                    // Sample 3 times
                                    // Each bit time is 15Tq    
   
    C2CFG2bits.SEG1PH = 1;  //相位缓冲段 （n+1） TQ  
    C2CFG2bits.SEG2PHTS = 1;    //相位段 2 时间选择位 1-可自由编程， 0-SEG1PH 与信息处理时间 （ 3 TQ）中的较大值
    C2CFG2bits.SAM = 0;     //采样次数 1-采样1次， 0-采样3次    
    C2CFG2bits.PRSEG = 1;   //广播时间端bit
    ClrWdt();
    C2CFG2bits.SEG2PH = 2;  //相位缓冲段 2 位 
}

/********************************************
* 函数名： ConfigCANOneMaskFilterRX0()
* 形参：void
* 返回值：void
* 功能：配置CAN1 RX0接收屏蔽与滤波寄存器寄存器
**********************************************/
inline void ConfigCANOneMaskFilterRX0(EIDBits* pRm0, EIDBits* pRf0)
{
    C2RX0CON = 0x0000;
    C2RX0CONbits.FILHIT0 = 0; //选择接收滤波器RF0
    //RM0 屏蔽器配置 标准与扩展
    ClrWdt();
    C2RXM0SIDbits.MIDE = 0; //过滤类型 扩展帧; //EXIDE = 1 其它29bit全为1， 0-标准帧
    C2RXM0SIDbits.SID = pRm0->regBits.SID10_0; 
    C2RXF0SIDbits.EXIDE = 0;//1-使能扩展帧 0-标准帧
    C2RXF0SIDbits.SID = pRf0->regBits.SID10_0;	//CAN1 Receive Acceptance Filter2 SID 	 bit 12-2 SID<10:0>： 标准标识符屏蔽位
}
/********************************************
*函数名： ConfigCANOneMaskFilterRX1()
*形参：void
*返回值：void
*功能：配置CAN1 RX1接收屏蔽与滤波寄存器寄存器
**********************************************/
inline void ConfigCANOneMaskFilterRX1(EIDBits* pRm1, EIDBits* pRf2)
{
    //接收缓冲区1配置 RF1 RM1-RF2,RF3,RF4,RF5
    C2RX1CON = 0x0000;
    C2RX1CONbits.FILHIT = 0b010;//选择接收过滤器RXF2
    //RM1 屏蔽器配置 标准与扩展
    ClrWdt();
    C2RXM1SIDbits.MIDE = 1; //过滤类型 扩展帧; //EXIDE = 1 其它29bit全为1
    C2RXM1SIDbits.SID = pRm1->regBits.SID10_0; 
    C2RXM1EIDH = pRm1->regBits.EID17_6;
    C2RXM1EIDLbits.EID5_0 = pRm1->regBits.EID5_0;       
    //接收滤波器 RF2 (RF2)
    ClrWdt();
    C2RXF2SIDbits.EXIDE = 1;//使能扩展帧
    C2RXF2SIDbits.SID     = pRf2->regBits.SID10_0;  //CAN1 Receive Acceptance Filter2 SID 	 bit 12-2 SID<10:0>： 标准标识符屏蔽位
    C2RXF2EIDH            = pRf2->regBits.EID17_6;  //CAN1 Receive Acceptace  Filter2 Extended Identifier high byte  bit 11-0 EID<17:6>： 扩展标识符屏蔽位 17-6
    C2RXF2EIDLbits.EID5_0 = pRf2->regBits.EID5_0;   //CAN1 Receive Acceptance Filter2 Extended identifier low byte   bit 15-10 EID<5:0>： 扩展标识符位
}
/********************************************
*函数名： InitCANOne()
*形参：EIDBits* pRm——屏蔽寄存器设置，EIDBits* pRf——滤波寄存器设置
*返回值：uint16_t 初始化成功返回0xAA 失败返回0
*功能：初始化系统中的 CAN1 模块
**********************************************/
uint16_t InitCANOne(EIDBits* pRm, EIDBits* pRf)
{
    uint16_t i = 0, state = 0;
    
    ClrWdt();
    ///Interrupt Section of CAN Peripheral 中断配置
    C2INTF = 0;					//Reset all The CAN Interrupts 
    IFS2bits.C2IF = 0;  			//Reset the Interrupt Flag status register
    C2INTE = 0x00FF;               //Enable all CAN interrupt sources
    IEC2bits.C2IE = 1;				//Enable the CAN1 Interrupt 
      
    ClrWdt();
    C2CTRLbits.REQOP = REQOP_CONFIG;//设置为配置模式 
    
    ConfigCANOneBraud();//配置CAN1波特率
    ConfigCANOneMaskFilterRX0(pRm, pRf);//配置RX0接收屏蔽滤波器
    ConfigCANOneMaskFilterRX1(pRm, pRf);//配置RX1接收屏蔽滤波器
    
    ClrWdt();
    C2CTRLbits.REQOP = REQOP_LOOPBACK;
    //2ms 看门狗等待
    i = 0;
    state = 0xAA;
    while(C2CTRLbits.OPMODE != REQOP_LOOPBACK)//Wait for CAN1 mode change from Configuration Mode to Loopback mode 
    {
        ClrWdt();
        __delay_us(10);
        ClrWdt();
        if (i++ > 1000)
        {
            state = 0;
            break;//超时错误
        }
    }
    return state;     
}
uint16_t InitStandardCAN(uint16_t id, uint16_t mask)
{
    uint16_t i = 0, state = 0;
    id = 0;
    mask = 0;
    
    ///Interrupt Section of CAN Peripheral 中断配置
    C2INTF = 0;					//Reset all The CAN Interrupts 
    IFS2bits.C2IF = 0;  			//Reset the Interrupt Flag status register
    C2INTE = 0x00FF;               //Enable all CAN interrupt sources
    IEC2bits.C2IE = 1;				//Enable the CAN1 Interrupt 

    C2CTRLbits.REQOP = REQOP_CONFIG;//设置为配置模式 
    ClrWdt();

    //配置CAN比特率
    C2CTRLbits.CANCKS = 0;			// 0-4Fcy 1-Fcy  Select the CAN Master Clock . It is equal to Fcy here. 
                                    // equal to Fcy.(Fcy=30Mhz)
    C2CFG1bits.SJW=00;				//同步跳转宽度时间 Synchronized jump width time is 1 x TQ when SJW is equal to 00
    C2CFG1bits.BRP = BRP_VAL;		//波特率预分频比位 ((FCY/(2*NTQ*BITRATE))-1) 	

    C2CFG2bits.SEG1PH = 6; //相位缓冲段 （n+1） TQ  
    C2CFG2bits.SEG2PHTS = 1; //相位段 2 时间选择位 1-可自由编程， 0-SEG1PH 与信息处理时间 （ 3 TQ）中的较大值
    C2CFG2bits.SAM = 0;    //采样次数 1-采样1次， 0-采样3次    
    C2CFG2bits.PRSEG = 1; //广播时间端bit
    C2CFG2bits.SEG2PH = 5; //相位缓冲段 2 位 

    ClrWdt();

    // Configure Receive registers, Filters and Masks配置接收滤波器与屏蔽滤波器
    //清空接收滤波器
    // 接收缓冲区 0 状态和控制寄存器  RM0-RF0,RF1
    C2RX0CON = 0x0000;
    C2RX0CONbits.FILHIT0 = 0; //选择接收滤波器RF0
    //RM0 屏蔽器配置 标准与扩展
    C2RXM0SIDbits.MIDE = 0; //过滤类型 扩展帧; //EXIDE = 1 其它29bit全为1， 0-标准帧
    C2RXM0SIDbits.SID     =  0;   //全部接收
    //接收滤波器 RF0 (RF1)
    C2RXF0SIDbits.EXIDE = 0;//1-使能扩展帧 0-标准帧
    C2RXF0SIDbits.SID     = 0;	//CAN1 Receive Acceptance Filter2 SID 	 bit 12-2 SID<10:0>： 标准标识符屏蔽位

    //错误中断允许位
    C2INTE = 0xBF;    //允许中断,不允许总线唤醒中断
    C2INTF = 0x00;    //标志位清零

    ClrWdt();
    C2CTRLbits.REQOP = REQOP_WORK   ; //正常工作模式REQOP_WORK
    //2ms 看门狗等待
    i = 0;
    state = 0xAA;
    while(C2CTRLbits.OPMODE !=  REQOP_WORK  )//Wait for CAN1 mode change from Configuration Mode to work mode 
    {
        __delay_us(10);
        ClrWdt();
        if (i++ > 1000)
        {
            state = 0;
            break;//超时错误
        }
    }
    return state;     
}
/********************************************
*函数名：ConfigEIDTX0()
*形参：EIDBits* pEID 发送扩展标识符
*返回值：void
*功能： 配置发送扩展标识符
**********************************************/
inline void ConfigEIDTX0(EIDBits* pEID)
{
    ClrWdt();
    C2TX0SIDbits.TXIDE = 1;//扩展帧
    C2TX0SIDbits.SRR = 1;
    C2TX0SIDbits.SID10_6 = pEID->txBits.SID10_6;
    C2TX0SIDbits.SID5_0  = pEID->txBits.SID5_0;
    
    C2TX0EIDbits.EID17_14 = pEID->txBits.EID17_14;
    C2TX0EIDbits.EID13_6 = pEID->txBits.EID13_6;
    
    C2TX0DLCbits.EID5_0 =  pEID->txBits.EID5_0;
}
/********************************************
*函数名：ConfigDataTXB0()
*形参：uint8 len 待处理数据长度 ，CANFrame* pframe 帧数据
*返回值：uint8 ——发送数据长度 0错误 
*功能： 将待发送数据装入TXB0 发送寄存器
**********************************************/
uint8_t ConfigDataTXB0(uint8_t len, CANFrame* pframe)
{
    uint8_t i = 0, j = 0;
    if (len > 0 && len <= 8)
    {
        ClrWdt();
        C2TX0DLCbits.DLC = len;
        C2TX0DLCbits.TXRTR = 0;//正常报文

        C2TX0CONbits.TXPRI = 3;
        for ( i = 0, j = 0; i < len; i += 2, j++)
        {
            switch (j)
            {
                case 0:
                    ClrWdt();
                    C2TX0B1 = pframe->frameDataWord[j];
                    break;
                case 1:
                    ClrWdt();
                    C2TX0B2 = pframe->frameDataWord[j];
                    break;
                case 2:
                    ClrWdt();
                    C2TX0B3 = pframe->frameDataWord[j];
                    break;
                case 3:
                    ClrWdt();
                    C2TX0B4 = pframe->frameDataWord[j];
                    break;
            }
        }
        ClrWdt();
        len = C2TX0DLCbits.DLC;
    }
    else
    {
        ClrWdt();
        len =  0;
    }
    return len;
}
  /********************************************
*函数名：ConfigDataTXB1()
*形参：uint8_t len 待处理数据长度 ，CANFrame* pframe 帧数据
*返回值：uint8_t ——发送数据长度 0 错误
*功能： 将待发送数据装入TXB1 发送寄存器
**********************************************/
uint8_t ConfigDataTXB1(uint8_t len, CANFrame* pframe)
{
    uint8_t i = 0, j = 0;
    if (len > 0 && len <= 8)
    {
        C2TX1DLCbits.DLC = len ;
        C2TX1DLCbits.TXRTR = 0;//正常报文
        C2TX1CONbits.TXPRI = 2;
        ClrWdt();
        for ( i = 0, j = 0; i < len; i += 2, j++)
        {
            ClrWdt();
            switch (j)
            {
                case 0:
                    C2TX1B1 = pframe->frameDataWord[j];
                    ClrWdt();
                    break;
                case 1:
                    ClrWdt();
                    C2TX1B2 = pframe->frameDataWord[j];
                    break;
                case 2:
                    ClrWdt();
                    C2TX1B3 = pframe->frameDataWord[j];
                    break;
               case 3:
                    ClrWdt();
                    C2TX1B4 = pframe->frameDataWord[j];
                    break;
            }
        }
        len = C2TX1DLCbits.DLC;
    }
    else
    {
        ClrWdt();
        len =  0;
    }
    return len;
}
  
/********************************************
*函数名：ConfigDataTXB2()
*形参：uint8_t len 待处理数据长度 ，CANFrame* pframe 帧数据
*返回值：uint8_t ——发送数据长度 0错误
*功能： 将待发送数据装入TXB2 发送寄存器
**********************************************/
uint8_t ConfigDataTXB2(uint8_t len, CANFrame* pframe)
{
    uint8_t i = 0, j = 0;
    if (len > 0 && len <= 8)
    {
        ClrWdt();
        C2TX2DLCbits.DLC = len;
        C2TX2DLCbits.TXRTR = 0;//正常报文
        C2TX0CONbits.TXPRI = 1;
        for ( i = 0, j = 0; i < len; i += 2, j++)
        {
            ClrWdt();
            switch (j)
            {
                case 0:
                    ClrWdt();
                    C2TX2B1 = pframe->frameDataWord[j];
                    break;
                case 1:
                    ClrWdt();
                    C2TX2B2 = pframe->frameDataWord[j];
                    break;
                case 2:
                    ClrWdt();
                    C2TX2B3 = pframe->frameDataWord[j];
                    break;
                case 3:
                    ClrWdt();
                    C2TX2B4 = pframe->frameDataWord[j];
                    break;
            }
        }
        len = C2TX2DLCbits.DLC;
    }
    else
    {
        ClrWdt();
        len =  0;
    }
    return len;
}

/********************************************
*函数名：CANOneSendByTX0()
*形参：: uint16_t* pID  11bitID标识, uint8_t * pbuff 缓冲数据, uint8_t len 数据长度
*返回值：uint8_t —— 发送数据总长度 0--数据出错
*功能： 通过TX0发送带有CRC16的帧数据
**********************************************/
uint8_t CANSendData(uint16_t id, uint8_t * pbuff, uint8_t len)
{  
    if ((len <= 8) && (len > 0))
    {
        ClrWdt();
        C2TX0SIDbits.TXIDE = 0;//标准帧
        C2TX0SIDbits.SRR = 0; //0-正常报文 1-报文将请求远程发送
        C2TX0SIDbits.SID10_6 = GET_SID10_6(id);
        C2TX0SIDbits.SID5_0  = GET_SID5_0(id);    
        
        C2TX0DLCbits.DLC = len;
        C2TX0DLCbits.TXRTR = 0;//正常报文       
        C2TX0CONbits.TXPRI = 3;
        ClrWdt();
         
        //可考虑使用地址，简化操作
        switch(len) 
        {
            case 1:
            {
                ClrWdt();
                C2TX0B1 =   pbuff[0];                 
                break;
            }
            case 2:
            {
                C2TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];            
                ClrWdt();     
                break;
            }
            case 3:
            {
                ClrWdt();
                C2TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];       
                C2TX0B2 =   pbuff[2];                 
                break;
            }
            case 4:
            {
                ClrWdt();
                C2TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                C2TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                break;
            } 
            case 5:
            {
                ClrWdt();
                C2TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                C2TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                C2TX0B3 =   pbuff[4]; 
                break;
            }
            case 6:
            {
                ClrWdt();
                C2TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                C2TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                C2TX0B3 =   (((uint16_t)pbuff[5]) << 8)  | pbuff[4];
                break;
            } 
            case 7:
            {
                ClrWdt();
                C2TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                C2TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                C2TX0B3 =   (((uint16_t)pbuff[5]) << 8)  | pbuff[4];
                C2TX0B4 =   pbuff[6]; 
                break;
            }
            case 8:
            {
                ClrWdt();
                C2TX0B1 =   (((uint16_t)pbuff[1]) << 8)  | pbuff[0];   
                C2TX0B2 =   (((uint16_t)pbuff[3]) << 8)  | pbuff[2];
                C2TX0B3 =   (((uint16_t)pbuff[5]) << 8)  | pbuff[4];
                C2TX0B4 =   (((uint16_t)pbuff[7]) << 8)  | pbuff[6];
                break;
            }    
        }
        ClrWdt();
        C2TX0CONbits.TXREQ = 1;         //请求发送
        //超时10ms看门狗复位
        while(C2TX0CONbits.TXREQ);      //发送完成后该标志位会被自动清零
    }
    return 0;
}
 
/***********************************************************
*函数名：ReadRx0Frame()
*形参：CANFrame* pframe--保存帧地址
*返回值：uint8 —— 接收数据长度 0--数据出错
*功能： 读取帧数据 
**************************************************************/
uint8_t ReadRx0Frame(CANFrame* pframe)
{
    uint8_t len = 0, i = 0, j = 0;

    if ((C2RX0CONbits.RXFUL == 1) && (C2RX0CONbits.RXRTRRO == 0) ) //包含一个有效报文 且不是远程帧
    {
        len = C2RX0DLCbits.DLC;
        for ( i = 0, j = 0; i < len; i += 2, j++)
        {
            ClrWdt();
            switch (j)
            {
                case 0:
                    pframe->frameDataWord[j] = C2RX0B1 ;
                    break;
                case 1:
                    pframe->frameDataWord[j] = C2RX0B2 ;
                    break;
                case 2:
                    pframe->frameDataWord[j] = C2RX0B3 ;
                    break;
                case 3:
                    pframe->frameDataWord[j] = C2RX0B4 ;
                    break;
                default:
                    return 0;//错误跳出
            }
        }
    }
    ClrWdt();
    C2RX0CONbits.RXFUL = 0; 
    return len;
}

/***********************************************************
* 函数名：GetReciveRX0EID()
* 形参：uint8 len --数据长度, CANFrame* pframe --待检测帧数据
* 返回值：uint8 —— 若为0代表失败， 大于3 代表成功，其值为数据长度
* 功能： 对接收帧数据进行CRC校验
**************************************************************/
inline void GetReciveRX0EID(EIDBits* pEID)
{
    ClrWdt();
    pEID->regBits.SID10_0 = C2RX0SIDbits.SID;
    pEID->regBits.EID17_6 = C2RX0EID;
    pEID->regBits.EID5_0 = C2RX0DLCbits.EID5_0;         
}

//-----------------------------------------------------------------------------
//Interrupt Section for CAN1
//-----------------------------------------------------------------------------
CANFrame Rframe;
uint8_t rlen = 0;
EIDBits rEID;
 
void __attribute__((interrupt, no_auto_psv)) _C2Interrupt(void)
{    
    ClrWdt();
    uint8_t rxErrorCount = C2EC & 0x00FF;
    uint8_t txErrorCount = (C2EC & 0xFF00) >> 8;
    
    ClrWdt();
    IFS2bits.C2IF = 0;         //Clear interrupt flag
    /*该错误是在CAN总线上产生任何一个错误都会引起该标志位置位*/
    C2INTFbits.IVRIF = 0; //该错误表示在CAN总线上不需要采取任何动作

    if(C2INTFbits.TX0IF)
    {
        ClrWdt();
        C2INTFbits.TX0IF = 0;  //If the Interrupt is due to Transmit0 of CAN1 Clear the Interrupt
    }  
    else if(C2INTFbits.TX1IF)
    {
        C2INTFbits.TX1IF = 0;   //If the Interrupt is due to Transmit1 of CAN1 Clear the Interrupt 
    }  

    if(C2INTFbits.RX0IF)
    {   
        ClrWdt();
        C2INTFbits.RX0IF = 0; 	//If the Interrupt is due to Receive0 of CAN1 Clear the Interrupt
        uint16_t id = C2RX0SIDbits.SID;
        uint8_t len = C2RX0DLCbits.DLC;
        ClrWdt();
        ReadRx0Frame(&Rframe);
        ClrWdt();
        DeviceNetReciveCenter(&id,Rframe.framDataByte, len);
    }
    else if(C2INTFbits.RX1IF)
    {  
        C2INTFbits.RX1IF = 0;  	//If the Interrupt is due to Receive1 of CAN1 Clear the Interrupt
        ClrWdt();
    }

    if((rxErrorCount < 120) || (txErrorCount < 120))
    {
        C2INTEbits.ERRIE = 1;   //开启错误中断
        IEC2bits.C2IE = 1;      //允许CAN中断
        ClrWdt();
//        UpdateIndicateState(ON_COM_ERROR_LED, TURN_OFF); //开启错误指示灯
    }
    
    /*总线关闭错误中断处理*/
    if((C2INTFbits.TXBO) && (C2INTFbits.ERRIF))
    {
        ClrWdt();
        //总线关断，需要报错，但是此时可以退出中断服务程序，但是不会改变TXBO位
        //可以选择不退出中断函数，或者报警，进行人为的总线关断恢复
        C2INTFbits.ERRIF = 0;   //退出中断服务
        C2INTEbits.ERRIE = 0;   //禁止错误中断，以允许其他程序的正常运行
//        UpdateIndicateState(ON_ERROR_LED, TURN_ON);     //开启错误指示灯
    }
    
    if(C2INTFbits.ERRIF == 1)
    {
        /*接收错误中断处理*/
        if(C2INTFbits.RX0OVR)
        {
            ClrWdt();
            C2INTFbits.RX0OVR = 0;  //清除接收缓冲器1溢出中断
        }
        else if(C2INTFbits.RX1OVR)
        {
            ClrWdt();
            C2INTFbits.RX1OVR = 0;  //清除接收缓冲器1溢出中断
        }
        
        if((C2INTFbits.EWARN) && (C2INTFbits.RXWAR)) //接收错误计数器警告
        {
            ClrWdt();
            //此时应该发出警告指示,错误计数器已经大于95,暂时不做处理
        }
        if(C2INTFbits.RXBP) //接收错误计数器警告
        {
            ClrWdt();
            //此时应该发出警告指示,错误计数器已经大于127，且装置处在总线被动状态
//            UpdateIndicateState(ON_COM_ERROR_LED, TURN_ON); //开启错误指示灯
        }

        /*发送错误中断处理*/
        if((C2INTFbits.EWARN) && (C2INTFbits.TXWAR)) //发送错误计数器警告
        {
            ClrWdt();
            //此时应该发出警告指示,错误计数器已经大于95,暂时不做处理
        }
        if(C2INTFbits.TXEP) //发送错误计数器警告
        {
            ClrWdt();
            //此时应该发出警告指示,错误计数器已经大于127，且装置处在总线被动状态
//            UpdateIndicateState(ON_COM_ERROR_LED, TURN_ON); //开启错误指示灯
        }
        C2INTFbits.ERRIF = 0;
    }
}

