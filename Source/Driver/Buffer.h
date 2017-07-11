#ifndef __BUFFER_H
#define __BUFFER_H


#include <stdint.h> 


typedef struct  
{
  unsigned int   id;                    /* 29 bit identifier */
  uint8_t data[8];               /* Data field */
  uint8_t len;                   /* Length of data field in bytes */
  uint8_t format;                /* 0 - STANDARD, 1- EXTENDED IDENTIFIER */
  uint8_t type;                  /* 0 - DATA FRAME, 1 - REMOTE FRAME */
} CAN_msg;


typedef struct TagFIFO
{
    uint8_t capacity;//FIFO总容量
    uint8_t count;//运算个数
    uint8_t head ;//队尾索引，入队索引
    uint8_t end;  //队头索引，出队索引
    CAN_msg* pMsg; 
}FifoInformation;


extern void BufferInit(void);
extern uint8_t BufferEnqueue( CAN_msg* pMsg);
extern uint8_t BufferDequeue( CAN_msg* pMsg);

#endif

