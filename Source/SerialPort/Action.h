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

#define HEZHA  0x30
#define FENZHA  0x31
#define WRITE_HEZHA_TIME  0x32 //合闸时间
#define WRITE_FENZHA_TIME  0x33 //分闸时间

#define TURN_ON_INT0 0x34
#define TURN_OFF_INT0 0x35
#define READ_HEZHA_TIME  0x36 //合闸时间
#define READ_FENZHA_TIME  0x37 //分闸时间

#define YONGCI_TONGBU  0x80
#define YONGCI_TONGBU_RESET  0x81
#define YONGCI_WAIT_HE_ACTION 0x85 //永磁等待合闸命令

#define ACK  0xFA

/**
 * @description: 以下为错误代码，返回的错误标号等
 */
#define ERROR_REPLY_ID  0x14
#define ERROR_EXTEND_ID 0xAA
#define ERROR_DATA_LEN  4
/**
 * 错误代码
 */
#define ID_ERROR     0x01       //ID号错误
#define DATA_LEN_ERROR   0x02   //数据长度错误
#define LOOP_ERROR   0x03       //回路数错误
#define SET_VALUE_ERROR   0x04  //设置值错误
#define WORK_MODE_ERROR   0x05  //在处于就地控制时使用了远方控制
#define OVER_TIME_ERROR   0x06  //预制超时错误
#define NOT_PERFABRICATE_ERROR  0x07        //没有预制就先执行的错误
#define SEVERAL_PERFABRICATE_ERROR  0x08    //多次预制警告
#define CAPVOLTAGE_ERROR  0x09      //欠压动作错误
#define HEFEN_STATE_ERROR 0x0A      //欠压动作错误

/**
 * 外部中断所需的固定值
 */
#define SECEND_INT   0x5A
#define TONGBU_HEZHA    0x0505
#define TONGBU_FENZHA   0x0606
#ifdef	__cplusplus
extern "C" {
#endif

void ExecuteFunctioncode(frameRtu* pRtu);

void FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);

extern _PERSISTENT uint16 ReceiveStateFlag;

#ifdef	__cplusplus
}
#endif

#endif	/* ACTION_H */

