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
    uint8_t address; //地址
    uint8_t funcode; //功能代码
    uint8_t datalen; //数据长度
    uint8_t* pData; //指向发送数据指针
    uint8_t completeFlag;
} frameRtu;

uint8_t FrameQueneIn(uint8_t recivData);
uint8_t FrameQueneOut(uint8_t* pData);
void  ReciveFrameDataInit(void);
uint8_t ReciveBufferDataDealing(frameRtu* pJudgeFrame, frameRtu* pReciveFrame);

void  GenRTUFrame(uint8_t addr, uint8_t funcode,
                        uint8_t sendData[], uint8_t datalen, uint8_t* pRtuFrame, uint8_t *plen);

void SendFrame(uint8_t* pFrame, uint8_t len);

void SetOverTime(uint16_t delayTime);
uint8_t CheckIsOverTime(void);

extern uint8_t volatile ReciveErrorFlag;

#ifdef	__cplusplus
}
#endif

#endif	/* RTUFRAME_H */

