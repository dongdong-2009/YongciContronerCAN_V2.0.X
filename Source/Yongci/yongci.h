/* 
 * File:   yongci.h
 * Author: ZFreeGo
 *
 * 
 */

#ifndef YONGCI_H
#define	YONGCI_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "../Driver/tydef.h"

#define ON_HE_LOCK  0x55AA
#define ON_FEN_LOCK 0xAA55    
#define OFF_LOCK    0x0000
    
//远方就地
#define YUAN_STATE  0xAA
#define BEN_STATE   0x55
    
//IGBT工作状态
#define HE_ORDER_A   0xAA51     //机构1合闸命令
#define HE_ORDER_B   0xAA52     //机构2合闸命令
#define HE_ORDER_C   0xAA53     //机构3合闸命令
    
#define FEN_ORDER_A  0xAA54     //机构1分闸命令
#define FEN_ORDER_B  0xAA55     //机构2分闸命令
#define FEN_ORDER_C  0xAA56     //机构3分闸命令
    
#define HE_ORDER     0xAA57     //总的合闸命令
#define FEN_ORDER    0xAA58     //总的分闸命令
    
#define IDLE_ORDER   0x0000     //空闲命令

#define WAIT_ORDER 0xBBBB //等待指令
#define NULL_ORDER 0x0000 //等待指令  
    
#define HEZHA_TIME_A  50 //A路 合闸时间 默认
#define FENZHA_TIME_A 30 //A路 分闸时间 默认

    
//IO 检测对应状态
#define CHECK_ERROR1_STATE 0x10  //机构1错误
#define CHECK_ERROR2_STATE 0x11  //机构2错误
#define CHECK_ERROR3_STATE 0x12  //机构3错误
    
#define CHECK_ERROR4_STATE 0x13  //机构1错误，同时输入了合闸、分闸信号
#define CHECK_ERROR5_STATE 0x14  //机构2错误，同时输入了合闸、分闸信号
#define CHECK_ERROR6_STATE 0x15  //机构3错误，同时输入了合闸、分闸信号
    
#define NO_ERROR    0x1F
#define CHECK_ERROR_STATE    0x1E
    
#define CHECK_Z_HE_ORDER  0x22  //同时合闸动作
#define CHECK_Z_FEN_ORDER 0x23  //同时分闸动作
    
#define CHECK_1_HE_ORDER  0x24  //机构1合闸动作
#define CHECK_1_FEN_ORDER 0x25  //机构1分闸动作
    
#define CHECK_2_HE_ORDER  0x26  //机构2合闸动作
#define CHECK_2_FEN_ORDER 0x27  //机构2分闸动作
    
#define CHECK_3_HE_ORDER  0x28  //机构3合闸动作
#define CHECK_3_FEN_ORDER 0x29  //机构3分闸动作
    
#define CHECK_Z_HE_STATE  0x32  //总的合闸状态
#define CHECK_Z_FEN_STATE 0x33  //总的分闸状态

#define CHECK_1_HE_STATE  0x34  //机构1合闸状态
#define CHECK_1_FEN_STATE 0x35  //机构1分闸状态
    
#define CHECK_2_HE_STATE  0x36  //机构2合闸状态
#define CHECK_2_FEN_STATE 0x37  //机构2分闸状态
    
#define CHECK_3_HE_STATE  0x38  //机构3合闸状态
#define CHECK_3_FEN_STATE 0x39  //机构3分闸状态
    
//存储合闸、分闸次数的EEPROM地址
//*********************************************
#define JG1_HE_COUNT_ADDRESS  0x007FF002
#define JG1_FEN_COUNT_ADDRESS 0x007FF004
    
#define JG2_HE_COUNT_ADDRESS  0x007FF006
#define JG2_FEN_COUNT_ADDRESS 0x007FF008
    
#define JG3_HE_COUNT_ADDRESS  0x007FF00A
#define JG3_FEN_COUNT_ADDRESS 0x007FF00C
//*********************************************
    
void YongciMainTask(void);
void YongciStopCurrentA(void);
void YongciFirstInit(void);

void HeZhaActionA(void);
void FenZhaActionA(void);

void SetRunStateFlagA(uint16 newFlag);
uint16 GetRunStateFlagA();

void HeOnLock(void);
void FenOnLock(void);
void OffLock(void);

void UpdateCount(uint16 states);


extern volatile uint16 HezhaTimeA;
extern volatile uint16 FenzhaTimeA;

extern volatile uint16 WaitIntOrder;

#ifdef	__cplusplus
}
#endif

#endif	/* YONGCI_H */

