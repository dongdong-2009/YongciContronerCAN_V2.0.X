/*
 * File:   Action.h
 * Author: ZFreeGo
 *
 * Created on 2014年9月22日, 下午3:35
 */

#ifndef ACTION_H
#define	ACTION_H

#include "../Header.h"
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

enum CommandIdentify
{
	/// <summary>
	/// 合闸预制
	/// </summary>
	ReadyClose = 1,
	/// <summary>
	/// 合闸执行
	/// </summary>
	CloseAction = 2,
	/// <summary>
	/// 分闸预制
	/// </summary>
	ReadyOpen = 3,
	/// <summary>
	/// 分闸执行
	/// </summary>
	OpenAction = 4,

	/// <summary>
	/// 同步合闸预制
	/// </summary>
	SyncReadyClose = 5,
	/// <summary>
	/// 同步分闸预制
	/// </summary>
	//SyncReadyClose = 6,





	/// <summary>
	/// 主站参数设置,顺序
	/// </summary>
  //  MasterParameterSet = 0x10,
	// <summary>
	/// 主站参数设置，非顺序——按点设置
	/// </summary>
	MasterParameterSetOne = 0x11,
	/// <summary>
	/// 主站参数读取，顺序
	/// </summary>
	MasterParameterRead = 0x12,
	/// <summary>
	/// 主站参数读取，非顺序——按点读取
	/// </summary>
   // MasterParameterReadOne = 0x13,

	/// <summary>
	/// 错误
	/// </summary>
	ErrorACK = 0x14,


	/// <summary>
	/// 配置模式
	/// </summary>
	ConfigMode = 0x15,

	/// <summary>
	/// 子站状态改变信息上传
	/// </summary>
	SubstationStatuesChange = 0x1A,

	/// <summary>
	/// 多帧数据
	/// </summary>
	MutltiFrame = 0x1B,

	/// <summary>
	/// 同步控制器 合闸预制
	/// </summary>
	SyncOrchestratorReadyClose = 0x30,
	/// <summary>
	/// 同步控制器 合闸执行
	/// </summary>
	SyncOrchestratorCloseAction = 0x31,
   // SyncOrchestratorReadyOpen = 0x32,
	//SyncOrchestratorOpenAction = 0x32,




    };







//同步合闸命令
#define TONGBU_HEZHA    0x5555

/**
 * @description: 以下为错误代码，返回的错误标号等
 */
//*************************************************************************
#define ERROR_REPLY_ID  0x14    //错误帧ID
#define ERROR_EXTEND_ID 0xAA    //错误附加码
#define ERROR_DATA_LEN  4       //错误数据帧长度



/**
 * @description: 错误代码
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
#define HEFEN_STATE_ERROR 0x0A      //合、分位错误
#define REFUSE_ERROR 0x0B       //拒动错误
#define ERROR_DIFF_CONFIG    0x0C//合闸预制配置不同
#define ERROR_CHARGED_CONFIG    0x0D//开关带电分闸操作错误
    
//*************************************************************************



#ifdef	__cplusplus
extern "C" {
#endif

typedef struct TagSaveSystemSuddenState
{
	uint8_t switchState[LOOP_QUANTITY];		//开关状态
	uint8_t executeOrder[LOOP_QUANTITY];	//开关执行的命令
	uint8_t capState[LOOP_QUANTITY];		//电容状态
	uint8_t buffer[LOOP_QUANTITY];		//存储状态
	uint8_t suddenFlag;		//突发状态
    uint8_t switchsuddenFlag;   //分合位状态突变
    uint8_t capSuddentFlag; //电容状态突变
    
    uint8_t RefuseAction;   //拒动状态
}SystemSuddenState;

typedef struct RemoteControl
{
    uint16_t receiveStateFlag;  //分合状态指令 单片机在看门狗复位的情况下不会改变该值
    uint8_t  overTimeFlag;     //超时标识位
    uint8_t  orderId;           //执行的命令
    uint8_t  setFixedValue;     //设置定值指令
    uint8_t  GetAllValueFalg;   //读取所有的连续数据
    uint8_t  GetOneValueFalg;   //读取单个数据
    uint8_t  CanErrorFlag;      //CAN错误标识位
    uint8_t  FrameErrorFlag;	//接收数据错误标识位
    uint8_t  TongbuReadFlag;	//同步预制到达标识位
    uint8_t  HezhaReadFlag;		//合闸预制到达标识位
    uint8_t  RunHezhaFlag;		//合闸执行命令到达标识位
    uint8_t  FenzhaReadFlag;	//分闸预制到达标识位
    uint8_t  RunFenzhaFlag;		//分闸执行命令到达标识位
}RemoteControlState;

/**
 *执行合分闸参数
 */
typedef struct TagActionParameter
{
	uint8_t readyFlag; //相应控制器应答 准备标志 0--未准备，非0--准备动作
	uint8_t enable; //使能标志，0-禁止，非零使能
    uint8_t loop; //相I-1, II-2, III-3
    uint16_t offsetTime;//偏移时间   
}ActionParameter;

/**
 *执行同步合闸参数,
 */
typedef struct TagActionAttribute
{
    ActionParameter  Attribute[3]; //动作属性合集
    uint8_t loopByte;//回路控制字
    uint8_t count; //控制数量  
    uint8_t currentIndex; //当前执行位置
    uint8_t powerOnTime;
}ActionAttribute;






void FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
void UpdataState(void);
void CheckOrder(void);
void SendErrorFrame(uint8_t receiveID,uint8_t errorID);

void SendMonitorParameter(struct DefFrameData* pReciveFrame);
void ActionParameterInit(void);

extern RemoteControlState g_RemoteControlState; //远方控制状态标识位
extern SystemSuddenState g_SuddenState;    //需要上传的机构状态值
extern PointUint8 g_ParameterBuffer;   
extern ActionAttribute g_SynActionAttribute;
extern ActionAttribute g_NormalAttribute;


#ifdef	__cplusplus
}
#endif

#endif	/* ACTION_H */

