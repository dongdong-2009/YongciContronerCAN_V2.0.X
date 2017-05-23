/** 
 * <p>application name： RtuFrame.h</p> 
 * <p>application describing： RtuFrame宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
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

void SetOverTime(uint16 delayTime);
uint8 CheckIsOverTime(void);

#ifdef	__cplusplus
}
#endif

#endif	/* RTUFRAME_H */

