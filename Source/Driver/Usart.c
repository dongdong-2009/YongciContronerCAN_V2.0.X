/** 
 * <p>application name： Usart.c</p> 
 * <p>application describing： 配置Usart</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */
#include "usart.h"
#include <xc.h>
#include "../SerialPort/RtuFrame.h"
#include  "../Header.h"


/**************************************************
 *函数名： InitUART2()
 *功能:初始化UART2
 *形参：  unsigned int baud 波特率
 *返回值：void
****************************************************/
void InitUART1(unsigned int baud)
{
    float value = 0;
    // This is an EXAMPLE, so brutal typing goes into explaining all bit sets

    // The HPC16 board has a DB9 connector wired to UART2, so we will
    // be configuring this port only

    // configure U2MODE
    U1MODEbits.UARTEN = 0;	// Bit15 TX, RX DISABLED, ENABLE at end of func
    
    U1MODEbits.USIDL = 0;	// Bit13 Continue in Idle
    U1MODEbits.WAKE = 0;	// Bit7 No Wake up (since we don't sleep here)
    U1MODEbits.LPBACK = 0;	// Bit6 No Loop Back
    U1MODEbits.ABAUD = 0;	// Bit5 No Autobaud (would require sending '55')
    U1MODEbits.PDSEL = 0;	// Bits1,2 8bit, No Parity
    U1MODEbits.STSEL = 0;	// Bit0 One Stop Bit

    ClrWdt();
    value = (float)FCY /(float)(16*baud) - 1; //波特率 = FCY/(16 * (BRG + 1))
    U1BRG = 25;	//25-9600
    
    U1STAbits.UTXBRK = 0;	//Bit11 Disabled
    U1STAbits.UTXEN = 0;	//Bit10 TX pins controlled by periph
    U1STAbits.UTXBF = 0;	//Bit9 *Read Only Bit*
    U1STAbits.TRMT = 0;	//Bit8 *Read Only bit*
    U1STAbits.URXISEL = 0;	//Bits6,7 Int. on character recieved
    U1STAbits.ADDEN = 0;	//Bit5 Address Detect Disabled
    U1STAbits.RIDLE = 0;	//Bit4 *Read Only Bit*
    U1STAbits.PERR = 0;		//Bit3 *Read Only Bit*
    U1STAbits.FERR = 0;		//Bit2 *Read Only Bit*
    U1STAbits.OERR = 0;		//Bit1 *Read Only Bit*
    U1STAbits.URXDA = 0;	//Bit0 *Read Only Bit*

    ClrWdt();
    IFS0bits.U1TXIF = 0;	// Clear the Transmit Interrupt Flag
    IFS0bits.U1RXIF = 0;	// Clear the Recieve Interrupt Flag
    IEC0bits.U1RXIE = 1;	// 

    U1MODEbits.UARTEN = 1;	// And turn the peripheral on
    U1STAbits.UTXEN = 1;
    
}

/********************************************
*函数名：InitPortsUART2()
*形参：void
*返回值：void
*功能：配置串口端口，尤其相对应的485使能发送端口
**********************************************/
void InitPortsUART1()
{ 
    RX_T = 1;
    TX_T = 0;
    RX_TX_MODE_T = 0;
    RX_TX_MODE = RX_MODE ; //默认为接收
}

void UsartInit(void)
{
    InitPortsUART1();
    InitUART1(9600);
}
void UsartSend(unsigned char abyte)
{
    RX_TX_MODE = TX_MODE;   //--鉴于光耦响应时间，须有一定的延时
    U1TXREG = abyte;
    while(!U1STAbits.TRMT)
    {
        //此处需要添加超时复位
        ClrWdt(); //2ms超时后,看门狗复位
    }
    
    RX_TX_MODE = RX_MODE;
}
void UsartRecive(unsigned char abyte)
{
    ClrWdt();
}

void __attribute__ ((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
    
    ClrWdt();
    IFS0bits.U1RXIF = 0;
    ReciveErrorFlag = FrameQueneIn(U2RXREG);
    
}
void __attribute__ ((interrupt, no_auto_psv)) _U1TXInterrupt(void)
{
	IFS0bits.U1TXIF = 0;
}