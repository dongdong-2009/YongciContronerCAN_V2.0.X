/** 
 * <p>application name： CAN.h</p> 
 * <p>application describing： CAN宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */

#ifndef CAN_H
#define	CAN_H
#include "tydef.h"

//工作模式REQOP
#define  REQOP_MONTOR_ALL      0b111 //监听所有模式
#define  REQOP_CONFIG          0b100 //配置模式
#define  REQOP_MONTOR_ONLY     0b011 //仅仅监听模式
#define  REQOP_LOOPBACK        0b010 //环回模式
#define  REQOP_STOP            0b001 //禁止模式
#define  REQOP_WORK            0b000 //正常工作模式

//一个8字节完整结构 8bit字节到16bit字转换
typedef union {
	uint8_t  framDataByte[8];
    uint16_t frameDataWord[4];
} CANFrame;

//29bit 扩展标识符定义
typedef union {
    uint32_t doubleWord;
    struct {
        uint32_t null       : 3;   //低位在前,高位在后
        uint32_t arttriType : 6;
        uint32_t frameType  : 5;
        uint32_t sourceAddr : 7; 
        uint32_t detAddr    : 7; 
        uint32_t pri        : 4;
    }idBits;
    struct {
        uint32_t null       : 3;   //低位在前高位在后
        uint32_t EID5_0     : 6;
        uint32_t EID17_6    : 12; //EID13_6  EID17_14 
        uint32_t SID10_0    : 11; //SID5_0  SID10_6
    }regBits;
     struct {
        uint32_t null       : 3;   //低位在前高位在后
        uint32_t EID5_0     : 6;
        uint32_t EID13_6    : 8;
        uint32_t EID17_14   : 4; //EID13_6  EID17_14 
        uint32_t SID5_0     : 6;
        uint32_t SID10_6    : 5; //SID5_0  SID10_6
    }txBits;
}EIDBits;
//11bit 扩展标识符定义
typedef union {
    uint16_t word;
    struct {
        uint32_t null       : 3;   //低位在前,高位在后
        uint32_t arttriType : 6;
        uint32_t frameType  : 5;
        uint32_t sourceAddr : 7; 
       
    }idBits;
    struct {
        uint32_t SID10_0    : 11; //SID5_0  SID10_6
    }regBits;
     struct {
        uint16_t null       : 3;   //低位在前高位在后
        uint16_t SID5_0     : 6;
        uint16_t SID10_6    : 5; //SID5_0  SID10_6
    }txBits;
}SIDBits;
#ifdef	__cplusplus
extern "C" {
#endif

#define GET_SID5_0(id) (uint8_t)(id & 0x3F) //获取ID的0-5bit
#define GET_SID10_6(id) (uint8_t)((id>>6 ) & 0x01F) //获取ID的6-10 bit
    
#define ON_CAN_INT()    {IEC2bits.C2IE = 1; C2INTE = 0xBF; C1INTE = 0;}   
#define OFF_CAN_INT()    {IEC2bits.C2IE = 0; C2INTE = 0; C1INTE = 0;}    
    
enum  CANIntType {WakeInt = 0b111, RXB0Int = 0b110, RXB1Int = 0b101,  TXB0Int = 0b100, TXB1Int = 0b011,
      TXB2Int = 0b010, ErrorInt = 0b001, Null = 0b000};

      
#define CAN_PASSIVE_ERROR   0xA1    //接收缓冲器溢出发送被动错误
#define CAN_CLOSE_ERROR     0xA3    //错误计数慢256发送关断错误
#define CAN_TX_OVER_ERROR   0xA5    //错误计数慢256发送关断错误
      
      
extern uint16_t InitCANOne(EIDBits* pRm, EIDBits* pRf);
extern uint16_t InitStandardCAN(uint16_t id, uint16_t mask);

extern uint8_t CANSendData(uint16_t id, uint8_t * pbuff, uint8_t len);

#ifdef	__cplusplus
}
#endif

#endif	/* CAN_H */

