/** 
 * <p>application name： yongci.h.c</p> 
 * <p>application describing： 永磁控制器主要逻辑</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */

#ifndef XC_YONGCI_H
#define	XC_YONGCI_H

#ifdef	__cplusplus
extern "C" {
#endif
#include "../Driver/tydef.h"

#define RUN_STATE   0x77    //运行状态
#define REDAY_STATE 0x88    //准备状态
    
//远方就地
#define YUAN_STATE  0xA1    //远方状态
#define BEN_STATE   0xA5    //就地状态
    
    
//工作模式和调试模式
#define WORK_STATE  0x3F    //工作状态
#define DEBUG_STATE 0xCF    //调试状态
    
    
//IGBT工作状态
#define HE_ORDER     0xAAAA     //总的合闸命令
#define FEN_ORDER    0xAA55     //总的分闸命令
    
#define IDLE_ORDER   0x0000     //空闲命令

#define HEZHA_TIME  50 //合闸时间 默认
#define FENZHA_TIME 30 //分闸时间 默认

    
//IO 检测对应状态
#define CHECK_ERROR1_STATE 0x10  //机构1错误,分合位同时存在
#define CHECK_ERROR2_STATE 0x11  //机构2错误,分合位同时存在
#define CHECK_ERROR3_STATE 0x12  //机构3错误,分合位同时存在
    
#define CHECK_ERROR4_STATE 0x13  //机构1错误，同时输入了合闸、分闸信号
#define CHECK_ERROR5_STATE 0x14  //机构2错误，同时输入了合闸、分闸信号
#define CHECK_ERROR6_STATE 0x15  //机构3错误，同时输入了合闸、分闸信号
    
#define CHECK_ERROR7_STATE 0x16  //同时输入了总的合闸、分闸信号
    
#define NO_ERROR    0x00
    
#define CHECK_Z_HE_ORDER  0x22  //执行同时合闸动作
#define CHECK_Z_FEN_ORDER 0x23  //执行同时分闸动作
    
#define CHECK_1_HE_ORDER  0x24  //执行机构1合闸动作
#define CHECK_1_FEN_ORDER 0x25  //执行机构1分闸动作
    
#define CHECK_2_HE_ORDER  0x26  //执行机构2合闸动作
#define CHECK_2_FEN_ORDER 0x27  //执行机构2分闸动作
    
#define CHECK_3_HE_ORDER  0x28  //执行机构3合闸动作
#define CHECK_3_FEN_ORDER 0x29  //执行机构3分闸动作

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
    
#define SWITCH_ONE  0   //第一个动作的机构
#define SWITCH_TWO  1   //第二个动作的机构
#define SWITCH_THREE 2  //第三个动作的机构

 /**
  * 分合闸控制
  */
typedef struct SynchroSwitchConfig
{
	uint8   State;	//当前的状态
	uint16  Order;	//分合闸命令
    uint16  LastOrder;  //上一次执行的指令
	uint16  SwitchOnTime;   //合闸动作时间
	uint16  SwitchOffTime;	//分闸动作时间
	uint16  OffestTime;	//偏移时间
	uint32  SysTime;	//当前的系统时间
	void (*SwitchOn)(struct SynchroSwitchConfig* );     //开关合闸动作函数
	void (*SwitchOff)(struct SynchroSwitchConfig* );    //开关分闸动作函数
}SwitchConfig;

/**
 * 获取同步合闸的偏移时间等 
 */
typedef struct SystemIndexConfig
{
    uint8 indexLoop;
    uint8 onTime;
    uint16 offestTime;
    void (*GetTime)(struct SystemIndexConfig* );
}IndexConfig;

void YongciMainTask(void);
void YongciFirstInit(void);

void HEZHA_Action(uint8 index,uint16 time);
void FENZHA_Action(uint8 index,uint16 time);
void TongBuHeZha(void);
void GetOffestTime(struct DefFrameData* pReciveFrame , struct DefFrameData* pSendFrame);

extern frameRtu sendFrame, recvFrame;
extern uint8 _PERSISTENT g_Order;  //需要执行的命令,在单片机发生复位的情况下该值依然可以保存


#ifdef	__cplusplus
}
#endif

#endif	/* YONGCI_H */

