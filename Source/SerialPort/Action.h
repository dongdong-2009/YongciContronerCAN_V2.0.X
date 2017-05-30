/*
 * File:   Action.h
 * Author: ZFreeGo
 *
 * Created on 2014年9月22日, 下午3:35
 */

#ifndef ACTION_H
#define	ACTION_H

#include "RtuFrame.h"
#include "../Driver/tydef.h"
#include "../DeviceNet/DeviceNet.h"

#define  RESET_MCU   0x01

//*****************************************************
//485通信所需的状态量
#define HEZHA   0x30
#define FENZHA  0x31
#define WRITE_HEZHA_TIME   0x32 //合闸时间
#define WRITE_FENZHA_TIME  0x33 //分闸时间

#define TURN_ON_INT0  0x34
#define TURN_OFF_INT0 0x35
#define READ_HEZHA_TIME   0x36 //合闸时间
#define READ_FENZHA_TIME  0x37 //分闸时间

#define YONGCI_TONGBU  0x80
#define YONGCI_TONGBU_RESET  0x81
#define YONGCI_WAIT_HE_ACTION 0x85 //永磁等待合闸命令

#define ACK  0xFA
//*****************************************************


#ifdef	__cplusplus
extern "C" {
#endif
typedef struct GetSwitchState
{
	uint8 SwitchState1;		//开关状态
	uint8 SwitchState2;		//开关状态
	uint8 SwitchState3;		//开关状态
	uint8 ExecuteOrder1;	//开关执行的命令
	uint8 ExecuteOrder2;	//开关执行的命令
	uint8 ExecuteOrder3;	//开关执行的命令
	uint8 CapState1;		//电容状态
	uint8 CapState2;		//电容状态
	uint8 CapState3;		//电容状态
	uint8 SuddenFlag;		//更新状态
    
    uint8 Cap1Error;    //电容电压错误
    uint8 Cap2Error;    //电容电压错误
    uint8 Cap3Error;    //电容电压错误
}GetState;


void ExecuteFunctioncode(frameRtu* pRtu);

void FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
void UpdataState(void);

extern _PERSISTENT uint16 ReceiveStateFlag;

extern GetState g_GetState;    //需要上传的机构状态值

#ifdef	__cplusplus
}
#endif

#endif	/* ACTION_H */

