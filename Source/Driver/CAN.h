/* 
 * File:   CAN.h
 * Author: LiDehai
 *
 * Created on 2015年11月10日, 下午2:16
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
	uint8  framDataByte[8];
    uint16 frameDataWord[4];
} CANFrame;

//29bit 扩展标识符定义
typedef union {
    uint32 doubleWord;
    struct {
        uint32 null       : 3;   //低位在前,高位在后
        uint32 arttriType : 6;
        uint32 frameType  : 5;
        uint32 sourceAddr : 7; 
        uint32 detAddr    : 7; 
        uint32 pri        : 4;
    }idBits;
    struct {
        uint32 null       : 3;   //低位在前高位在后
        uint32 EID5_0     : 6;
        uint32 EID17_6    : 12; //EID13_6  EID17_14 
        uint32 SID10_0    : 11; //SID5_0  SID10_6
    }regBits;
     struct {
        uint32 null       : 3;   //低位在前高位在后
        uint32 EID5_0     : 6;
        uint32 EID13_6    : 8;
        uint32 EID17_14   : 4; //EID13_6  EID17_14 
        uint32 SID5_0     : 6;
        uint32 SID10_6    : 5; //SID5_0  SID10_6
    }txBits;
}EIDBits;
//11bit 扩展标识符定义
typedef union {
    uint16 word;
    struct {
        uint32 null       : 3;   //低位在前,高位在后
        uint32 arttriType : 6;
        uint32 frameType  : 5;
        uint32 sourceAddr : 7; 
       
    }idBits;
    struct {
        uint32 SID10_0    : 11; //SID5_0  SID10_6
    }regBits;
     struct {
        uint16 null       : 3;   //低位在前高位在后
        uint16 SID5_0     : 6;
        uint16 SID10_6    : 5; //SID5_0  SID10_6
    }txBits;
}SIDBits;
#ifdef	__cplusplus
extern "C" {
#endif

#define GET_SID5_0(id) (uint8)(id & 0x3F) //获取ID的0-5bit
#define GET_SID10_6(id) (uint8)((id>>6 ) & 0x01F) //获取ID的6-10 bit
    
#define ON_CAN_INT()    {IEC1bits.C1IE = 1;}   
#define OFF_CAN_INT()    {IEC1bits.C1IE = 0;}    
    
enum  CANIntType {WakeInt = 0b111, RXB0Int = 0b110, RXB1Int = 0b101,  TXB0Int = 0b100, TXB1Int = 0b011,
      TXB2Int = 0b010, ErrorInt = 0b001, Null = 0b000};
    
 extern uint16 InitCANOne(EIDBits* pRm, EIDBits* pRf);
 extern uint16 InitStandardCAN(uint16 id, uint16 mask);

 extern uint8 CANSendData(uint16 id, uint8 * pbuff, uint8 len);

#ifdef	__cplusplus
}
#endif

#endif	/* CAN_H */

