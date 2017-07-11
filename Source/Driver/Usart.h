/** 
 * <p>application name： Usart.h</p> 
 * <p>application describing： Usart宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */
#ifndef USART_H
#define	USART_H

#include "tydef.h"

#ifdef	__cplusplus
extern "C" {
#endif


#define TX   LATFbits.LATF3
#define TX_T TRISFbits.TRISF3
#define RX   PORTFbits.RF2
#define RX_T TRISFbits.TRISF2

#define RX_TX_MODE  LATGbits.LATG7  //0-接收模式， 1-发送模式
#define RX_TX_MODE_T TRISGbits.TRISG7
#define RX_MODE 0
#define TX_MODE 1
    
#define ON_UART_INT()  {IEC0bits.U1RXIE = 1;	} //应该配对使用
#define OFF_UART_INT()  {IEC0bits.U1RXIE = 0;	}    
    
void UsartInit();
void UsartSend(uint8_t abyte);

#ifdef	__cplusplus
}
#endif

#endif	/* USART_H */

