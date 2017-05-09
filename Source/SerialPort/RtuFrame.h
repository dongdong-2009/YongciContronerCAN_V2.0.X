/* 
 * File:   RtuFrame.h
 * Author: LiDehai
 *
 * Created on 2014年12月4日, 上午7:27
 */

#ifndef RTUFRAME_H
#define	RTUFRAME_H

#ifdef	__cplusplus
extern "C" {
#endif


#include "../Driver/tydef.h"
#define SEND_FRAME_LEN 64UL

typedef struct
{
    uint8 address; //地址
    uint8 funcode; //功能代码
    uint8 datalen; //数据长度
    uint8* pData; //指向发送数据指针
    boolean completeFlag;
} frameRtu;

boolean FrameQueneIn(uint8 recivData);
boolean FrameQueneOut(uint8* pData);
void  ReciveFrameDataInit(void);
unsigned char ReciveBufferDataDealing(frameRtu* pJudgeFrame, frameRtu* pReciveFrame);

void  GenRTUFrame(uint8 addr, uint8 funcode,
                        uint8 sendData[], uint8 datalen, uint8* pRtuFrame, uint8 *plen);

void SendFrame(uint8* pFrame, uint8 len);


#ifdef	__cplusplus
}
#endif

#endif	/* RTUFRAME_H */

