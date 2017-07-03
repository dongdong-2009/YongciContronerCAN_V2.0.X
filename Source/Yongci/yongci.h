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
#define READY_STATE 0x88    //准备状态
    
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
    
#define CHECK_Z_HE_ORDER  0xAA22  //执行同时合闸动作
#define CHECK_Z_FEN_ORDER 0xAA23  //执行同时分闸动作
    
#define CHECK_1_HE_ORDER  0xAA24  //执行机构1合闸动作
#define CHECK_1_FEN_ORDER 0xAA25  //执行机构1分闸动作
    
#define CHECK_2_HE_ORDER  0xAA26  //执行机构2合闸动作
#define CHECK_2_FEN_ORDER 0xAA27  //执行机构2分闸动作
    
#define CHECK_3_HE_ORDER  0xAA28  //执行机构3合闸动作
#define CHECK_3_FEN_ORDER 0xAA29  //执行机构3分闸动作

#define CHECK_1_HE_STATE  0x34  //机构1合闸状态
#define CHECK_1_FEN_STATE 0x35  //机构1分闸状态
    
#define CHECK_2_HE_STATE  0x36  //机构2合闸状态
#define CHECK_2_FEN_STATE 0x37  //机构2分闸状态
    
#define CHECK_3_HE_STATE  0x38  //机构3合闸状态
#define CHECK_3_FEN_STATE 0x39  //机构3分闸状态
    
//************************************************
//拒动错误值
#define Z_HE_ERROR      0xB2    //总合拒动错误
#define Z_FEN_ERROR     0xB3    //总分拒动错误
#define JIGOU1_HE_ERROR     0xB4  //机构1合位错误
#define JIGOU1_FEN_ERROR    0xB5  //机构1分位错误
    
#define JIGOU2_HE_ERROR     0xB6  //机构2合位错误
#define JIGOU2_FEN_ERROR    0xB7  //机构2分位错误
    
#define JIGOU3_HE_ERROR     0xB8  //机构3合位错误
#define JIGOU3_FEN_ERROR    0xB9  //机构3分位错误
//************************************************
    
#define SWITCH_ONE  0   //第一个动作的机构
#define SWITCH_TWO  1   //第二个动作的机构
#define SWITCH_THREE 2  //第三个动作的机构

#define ON_LOCK     0xAA55    
#define OFF_LOCK    0x0000

/**
 * 结构代号
 */    
#define  DEVICE_I  0
#define  DEVICE_II 1
#define  DEVICE_III 2
  
    
    
 /**
  * 分合闸控制
  */
typedef struct TagSwitchConfig
{
	uint8_t   currentState;	//当前的状态
	uint16_t  order;	//分合闸命令
    uint16_t  lastOrder;  //上一次执行的指令
	uint16_t  powerOnTime;   //合闸动作时间
	uint16_t  powerOffTime;	//分闸动作时间
	uint16_t  offestTime;	//偏移时间
	uint32_t  systemTime;      //当前的系统时间
	void (*SwitchClose)(struct  TagSwitchConfig* );     //开关合闸动作函数
	void (*SwitchOpen)(struct TagSwitchConfig* );    //开关分闸动作函数
}SwitchConfig;

/**
 * 获取同步合闸的偏移时间等 
 */
typedef struct SystemIndexConfig
{
    uint8_t indexLoop;
    uint8_t onTime;
    uint16_t offestTime;
    void (*GetTime)(struct SystemIndexConfig* );
}IndexConfig;

void YongciMainTask(void);
void YongciFirstInit(void);

void HEZHA_Action(uint8_t index,uint16_t time);
void FENZHA_Action(uint8_t index,uint16_t time);

uint8_t GetOffestTime(struct DefFrameData* pReciveFrame);
void OnLock(void);
void OffLock(void);
void SynCloseAction(void);
void CloseOperation(void);
void OpenOperation(void);
extern frameRtu sendFrame, recvFrame;
extern uint16_t _PERSISTENT g_Order;  //需要执行的命令,在单片机发生复位的情况下该值依然可以保存
//extern uint32_t _PERSISTENT g_TimeStampCollect.changeLedTime.delayTime; //改变LED灯闪烁时间 (ms) TODO:为什么？
extern uint16_t _PERSISTENT g_lockUp;   //命令上锁，在执行了一次合分闸命令之后应处于上锁状态，在延时800ms之后才可以第二次执行


#ifdef	__cplusplus
}
#endif

#endif	/* YONGCI_H */

