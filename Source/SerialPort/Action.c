/***********************************************
*Copyright(c) 2015,FreeGo
*保留所有权利
*文件名称:Action.c
*文件标识:
*创建日期： 2015年月4月23日
*摘要:

*2015/4/23:套用历史版本框架.
*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#include "../Header.h"
#include "../Yongci/DeviceParameter.h"
#include "../SerialPort/RefParameter.h"

#define  SUDDEN_ID 0x1A     //突发状态上传ID
/**
 * @description: 以下为错误代码，返回的错误标号等
 */
#define ERROR_REPLY_ID  0x14
#define ERROR_EXTEND_ID 0xAA
#define ERROR_DATA_LEN  4
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

#define TONGBU_HEZHA    0x0055

void SendAckMesssage(uint8 fun);
void SendErrorFrame(uint8 receiveID,uint8 errorID);

extern uint8 volatile SendFrameData[SEND_FRAME_LEN];

uint16 g_offestTimeOne = 50;
uint16 g_offestTimeTwo = 60;

//分合状态指令 单片机在看门狗复位的情况下不会改变该值
_PERSISTENT uint16 ReceiveStateFlag;  

GetState g_GetState;    //需要上传的机构状态值

/**************************************************
 *函数名： SendAckMesssage()
 *功能：  回传校验码
 *形参：  Uint16 fun 功能代码地址
 *返回值：void
****************************************************/
inline void SendAckMesssage(uint8 fun)
{
    uint16 len = 0;
    ClrWdt();
    GenRTUFrame(LOCAL_ADDRESS, ACK, &fun, 1, (uint8*)SendFrameData, (uint8 *)&len);
    ClrWdt();
    SendFrame((uint8*)SendFrameData, len);
    ClrWdt();
}
/**************************************************
 *函数名： ExecuteFunctioncode()
 *功能：  执行功能代码
 *形参：  接收帧指针 frameRtu* pRtu
 *返回值：void
****************************************************/
void ExecuteFunctioncode(frameRtu* pRtu)
{
    ClrWdt();
    //该数据帧未处理过
    if (pRtu->completeFlag == TRUE)
    {
       // LEDE ^= 1;
        if ( pRtu->funcode != YONGCI_WAIT_HE_ACTION)
        {
            SendAckMesssage( pRtu->funcode);
        }
         ClrWdt();
        switch(pRtu->funcode)
        {
            case RESET_MCU:
            {
                Reset(); //软件复位
                break;
            }
            case TURN_ON_INT0:
            {
                TurnOnInt2();
                break;
            }
            case TURN_OFF_INT0:
            {
                TurnOffInt2();
                break;
            }
            case HEZHA: //立即合闸
            {
                if(g_SystemState.workMode == WORK_STATE)
                {
                    TongBuHeZha(g_offestTimeOne,g_offestTimeTwo);
                    ClrWdt();
                    return ;
                }
                break;
            }
           case FENZHA: //立即分闸
            {
                if(g_SystemState.workMode == WORK_STATE) //多加入一重验证
                {
                    FENZHA_Action(SWITCH_ONE , FENZHA_TIME);
                    FENZHA_Action(SWITCH_TWO , FENZHA_TIME);
                    if(CAP3_STATE)
                    {
                        FENZHA_Action(SWITCH_THREE , FENZHA_TIME);
                    }
                    ClrWdt();
                }
                return ;
                break;
            }
            case WRITE_HEZHA_TIME:
            {
                ClrWdt();
                if ( ((pRtu->pData)[3] < 101) && ((pRtu->pData)[3] > 0))
                {
                    g_DelayTime.hezhaTime1 = (pRtu->pData)[3];
                }
                break;
            }
            case WRITE_FENZHA_TIME:
            {
                ClrWdt();
                if ( ((pRtu->pData)[3] < 101) && ((pRtu->pData)[3] > 0))
                {
                    g_DelayTime.fenzhaTime1 = (pRtu->pData)[3];
                }
                break;
            }
            case  YONGCI_WAIT_HE_ACTION:
            {
                ClrWdt();
                break;
            }
            default :
            {
                break;
            }
        }
    pRtu->completeFlag = FALSE;
    }
}


/**
 * 引用帧服务
 *
 * @param  指向接收数据的指针
 * @param  指向发送数据的指针
 * @bref   对完整帧进行提取判断
 */
void FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    uint8 i = 1;    
    uint8 id = pReciveFrame->pBuffer[0];
    uint8 error = 0;    //错误
    uint8 idIndex = 1;  //索引，读取属性值时可用到
    uint16 highTime = 0;
    uint16 lowTime = 0;
    
    ClrWdt();
    uint8 Data[8] = {0,0,0,0,0,0,0,0};
    PointUint8 Point;
    Point.pData = Data;
    Point.len = 8;

    /**
     * 发送数据帧赋值
     */
    pSendFrame->ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
    pSendFrame->pBuffer[0] = id | 0x80;
    ClrWdt();

    /*就地控制时可以读取和设置参数，而不能执行分合闸、以及阈值指令*/
	if((id < 0x10) && (g_SystemState.yuanBenState == 0x55))
    {
        ClrWdt();
        SendErrorFrame(pReciveFrame->pBuffer[0],WORK_MODE_ERROR);
        return;
    }
    
	if(id < 5)
	{
		for(i = 1;i < pReciveFrame->len;i++)
		{
            ClrWdt();
			pSendFrame->pBuffer[i] = pReciveFrame->pBuffer[i];
		}
		pSendFrame->len = pReciveFrame->len;
	}  
    
    switch(id)
    {
        case 1: //合闸预制
        {
            if((pReciveFrame->pBuffer[1] <= 0x00) || (pReciveFrame->pBuffer[1] > 0x07))
            {
                //判断回路数，回路数超过要求则返回错误
                SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
            }
            if((ReceiveStateFlag == IDLE_ORDER) && (g_SystemState.heFenState1 == CHECK_1_FEN_STATE) && 
                    (g_SystemState.heFenState2 == CHECK_2_FEN_STATE) && 
                    (g_SystemState.heFenState3 == CHECK_3_FEN_STATE))  //防止多次预制,防止多次合闸
            {
                ReceiveStateFlag = HE_ORDER;    //合闸命令
                SetOverTime(g_RemoteWaitTime);  //设置合闸预制超时等待时间
                SendData(pSendFrame);
            }
            else if((g_SystemState.heFenState1 != CHECK_1_FEN_STATE) || 
                    (g_SystemState.heFenState2 != CHECK_2_FEN_STATE) || 
                    (g_SystemState.heFenState3 != CHECK_3_FEN_STATE))
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
            }
            else
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],SEVERAL_PERFABRICATE_ERROR);
            }
            break;
        }
        case 2: //合闸执行
        {
            if((pReciveFrame->pBuffer[1] <= 0x00) || (pReciveFrame->pBuffer[1] > 0x07))
            {
                //判断回路数，回路数超过要求则返回错误
                SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
            }
            if(ReceiveStateFlag == HE_ORDER)
            {
                if(CheckIsOverTime())
                {
                    if(GetCapVolatageState()) //电容电压足够
                    {
                        TongBuHeZha(50,60);   //在远方状态下只能是执行工作模式
                        SendData(pSendFrame);
                    }
                    else
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                    }
                }
                else
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],OVER_TIME_ERROR);
                }
                ReceiveStateFlag = IDLE_ORDER;  //空闲命令
            }
            else
            {
                ReceiveStateFlag = IDLE_ORDER;
                SendErrorFrame(pReciveFrame->pBuffer[0],NOT_PERFABRICATE_ERROR);
            }
            break;
        }
        case 3: //分闸预制
        {
            if((pReciveFrame->pBuffer[1] <= 0x00) || (pReciveFrame->pBuffer[1] > 0x07))
            {
                //判断回路数，回路数超过要求则返回错误
                SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
            }
            if((ReceiveStateFlag == IDLE_ORDER) && (g_SystemState.heFenState1 == CHECK_1_HE_STATE) && 
                    (g_SystemState.heFenState2 == CHECK_2_HE_STATE) && 
                    (g_SystemState.heFenState3 == CHECK_3_HE_STATE))  //防止多次预制,防止多次合闸
            {
                ReceiveStateFlag = FEN_ORDER;   //分闸命令
                
                SetOverTime(g_RemoteWaitTime);   //设置分闸预制超时等待时间
                SendData(pSendFrame);
            }
            else if((g_SystemState.heFenState1 != CHECK_1_HE_STATE) || 
                    (g_SystemState.heFenState2 != CHECK_2_HE_STATE) || 
                    (g_SystemState.heFenState3 != CHECK_3_HE_STATE))
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
            }
            else
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],SEVERAL_PERFABRICATE_ERROR);
            }
            break;
        }
        case 4: //分闸执行
        {
            if((pReciveFrame->pBuffer[1] <= 0x00) || (pReciveFrame->pBuffer[1] > 0x07))
            {
                //判断回路数，回路数超过要求则返回错误
                SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return; 
            }
            if(ReceiveStateFlag == FEN_ORDER)
            {  
                if(CheckIsOverTime())
                {
                    if(GetCapVolatageState()) //处于空闲状态,且储能
                    {                        
                        //执行的是同步分闸操作
                        FENZHA_Action(SWITCH_ONE , FENZHA_TIME);
                        FENZHA_Action(SWITCH_TWO , FENZHA_TIME);
                        if(CAP3_STATE)
                        {
                            FENZHA_Action(SWITCH_THREE , FENZHA_TIME);
                        }
                        SendData(pSendFrame);
                    }
                    else
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                    }
                }
                else
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],OVER_TIME_ERROR);
                }
                ReceiveStateFlag = IDLE_ORDER;  //空闲命令
            }
            else
            {
                ReceiveStateFlag = IDLE_ORDER;
                SendErrorFrame(pReciveFrame->pBuffer[0],NOT_PERFABRICATE_ERROR);
            }
            break;
        }        
        case 5: //同步合闸预制
        {
            if((pReciveFrame->len  & 0x01)) //数据长度不对，数据长度不应为奇数
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                return;
            }

            pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1] & 0x03;

            if((pReciveFrame->pBuffer[1] <= 0x03) && (pReciveFrame->len == 2))  //只有一路需要控制
            {
                pSendFrame->len = 2;
            }
            else if((pReciveFrame->pBuffer[1] <= 0x0F) && (pReciveFrame->len == 4)) //只有两个回路需要控制
            {
                pSendFrame->pBuffer[2] = (pReciveFrame->pBuffer[1] & 0x0C) >> 2;
                
                if(pSendFrame->pBuffer[1] == pSendFrame->pBuffer[2])    //不应该出现相同的回路数
                {
                    //错误消息为存在相同的回路数
                    SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                    return;
                }
                
                lowTime = pReciveFrame->pBuffer[2];
                highTime = pReciveFrame->pBuffer[3];
                g_offestTimeOne = (highTime << 8) | lowTime;
                
                pSendFrame->len = 3;
            }
            else if((pReciveFrame->pBuffer[1] <= 0x3F) && (pReciveFrame->len == 6)) //只有三个回路需要控制
            { pSendFrame->pBuffer[2] = (pReciveFrame->pBuffer[1] & 0x0C) >> 2;
                pSendFrame->pBuffer[3] = (pReciveFrame->pBuffer[1] & 0x30) >> 4;
               
                if(pSendFrame->pBuffer[1] == pSendFrame->pBuffer[2] || (pSendFrame->pBuffer[1] == pSendFrame->pBuffer[3])
                        || (pSendFrame->pBuffer[2] == pSendFrame->pBuffer[3]))//不应该出现相同的回路数
                {
                    //错误消息为存在相同的回路数
                    SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                    return;
                }
                
                lowTime = pReciveFrame->pBuffer[2];
                highTime = pReciveFrame->pBuffer[3];
                g_offestTimeOne = (highTime << 8) | lowTime;
                
                lowTime = pReciveFrame->pBuffer[4];
                highTime = pReciveFrame->pBuffer[5];
                g_offestTimeTwo = (highTime << 8) | lowTime;
                
                pSendFrame->len = 4;
            }
            else if(pReciveFrame->len == 8) //只有四个回路需要控制
            {
                pSendFrame->pBuffer[2] = (pReciveFrame->pBuffer[1] & 0x0C) >> 2;
                pSendFrame->pBuffer[3] = (pReciveFrame->pBuffer[1] & 0x30) >> 4;
                pSendFrame->pBuffer[4] = (pReciveFrame->pBuffer[1] & 0xC0) >> 6;
                
                if((pSendFrame->pBuffer[1] == pSendFrame->pBuffer[2]) || (pSendFrame->pBuffer[1] == pSendFrame->pBuffer[3])
                 || (pSendFrame->pBuffer[1] == pSendFrame->pBuffer[4]) || (pSendFrame->pBuffer[2] == pSendFrame->pBuffer[3])
                 || (pSendFrame->pBuffer[2] == pSendFrame->pBuffer[4])|| (pSendFrame->pBuffer[3] == pSendFrame->pBuffer[4]))//不应该出现相同的回路数
                {
                    //错误消息为存在相同的回路数
                    SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                    return;
                }

                lowTime = pReciveFrame->pBuffer[2];
                highTime = pReciveFrame->pBuffer[3];
                g_offestTimeOne = (highTime << 8) | lowTime;
                
                lowTime = pReciveFrame->pBuffer[4];
                highTime = pReciveFrame->pBuffer[5];
                g_offestTimeTwo = (highTime << 8) | lowTime;
                
                pSendFrame->len = 5;
            }
            else
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;                
            }
            if(ReceiveStateFlag == IDLE_ORDER)
            {
                if((g_SystemState.heFenState1 == CHECK_1_FEN_STATE) && (GetCapVolatageState()) && 
                   (g_SystemState.heFenState2 == CHECK_2_FEN_STATE) && 
                   (g_SystemState.heFenState3 == CHECK_3_FEN_STATE))
                {
                    ReceiveStateFlag = TONGBU_HEZHA;    //同步合闸命令
                    SetOverTime(g_SyncReadyWaitTime);   //设置同步预制超时等待时间
                    
                    SendData(pSendFrame);
                    TurnOnInt2();   //必须是在成功的预制之后才能开启外部中断1
                }
                else if((g_SystemState.heFenState1 != CHECK_1_FEN_STATE) || 
                        (g_SystemState.heFenState2 != CHECK_2_FEN_STATE) || 
                        (g_SystemState.heFenState3 != CHECK_3_FEN_STATE))
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                    return;
                }
                else
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                    return;
                }
            }
            else
            {
                TurnOffInt2();
                SendErrorFrame(pReciveFrame->pBuffer[0],SEVERAL_PERFABRICATE_ERROR);
                ReceiveStateFlag = IDLE_ORDER;
            }
                
            break;
        }
        case 6: //同步分闸预制
        {
            break;
        }
        case 0x10:  //顺序参数设置
        {
            break;
        }
        case 0x11:  //非顺序参数设置
        {
            pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1];  //配置号
            Point.pData = pReciveFrame->pBuffer + 2;
            error = SetParamValue(pReciveFrame->pBuffer[1],&Point);
            if(error == 0xFF)
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],ID_ERROR);
                return;
            }
            else if(error)
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                return;
            }
            pSendFrame->pBuffer[2] = Point.pData[0];
            pSendFrame->pBuffer[3] = Point.pData[1];
            pSendFrame->len = Point.len + 2;
            SendData(pSendFrame);
            
            if(pSendFrame->complteFlag == 0)    //当数据发送完毕后进行写累加和
            {
                WriteAccumulateSum();  //写入累加和
            }
            
            break;
        }
        case 0x12:  //顺序参数读取
        {
            for(idIndex = pReciveFrame->pBuffer[1];idIndex <= pReciveFrame->pBuffer[2];idIndex++)    //抛除ID号所占的长度
            {
                Point.len = 8;
                error = ReadParamValue(idIndex,&Point);
                if((error == 0xF1)||(error == 0xF3))    //数据长度错误
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                }
                pSendFrame->pBuffer[1] = idIndex;  //配置号  
                pSendFrame->pBuffer[2] = Point.pData[0];
                pSendFrame->pBuffer[3] = Point.pData[1];
                pSendFrame->len = Point.len + 2;
                SendData(pSendFrame);
            }
            break;
        }        
        case 0x13:  //非顺序参数读取
        {
            for(i = 1;i < pReciveFrame->len;i++)    //抛除ID号所占的长度
            {
                Point.len = 8;
                error = ReadParamValue(pReciveFrame->pBuffer[i],&Point);
                if((error == 0xF1)||(error == 0xF3))    //数据长度错误
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                }
                if((error == 0xF2)||(error == 0xF4))    //ID号错误
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],ID_ERROR);
                }
                pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[i];  //配置号
                pSendFrame->pBuffer[2] = Point.pData[0];
                pSendFrame->pBuffer[3] = Point.pData[1];
                pSendFrame->len = Point.len + 2;
                SendData(pSendFrame);
            }
            break;
        }
        default:
        {
            //错误的ID号处理
            SendErrorFrame(pReciveFrame->pBuffer[0],ID_ERROR);
            break;
        }          
    }
}


/**
 * @description: 发送错误帧数据
 * @param receiveID 主站发送的ID号
 * @param errorID   错误代码
 */
void SendErrorFrame(uint8 receiveID,uint8 errorID)
{
    uint8 data[8] = {0,0,0,0,0,0,0,0};
    struct DefFrameData pSendFrame;
    
    pSendFrame.pBuffer = data;
    pSendFrame.complteFlag = 0xFF;
    
    pSendFrame.ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
    pSendFrame.pBuffer[0] = ERROR_REPLY_ID;   //错误应答ID
    pSendFrame.pBuffer[1] = receiveID;  //主站发送ID
    pSendFrame.pBuffer[2] = errorID;   //错误代码
    pSendFrame.pBuffer[3] = ERROR_EXTEND_ID;  //扩展ID号            
    pSendFrame.len = ERROR_DATA_LEN;   //错误帧长度
    SendData(&pSendFrame);
}

/**
 * 
 * <p>Function name: [UpdataState]</p>
 * <p>Discription: [对运行状态进行更新显示]</p>
 */
void UpdataState(void)
{
    uint8 Data[8] = {0,0,0,0,0,0,0,0};

    struct DefFrameData pSendFrame;

    pSendFrame.pBuffer = Data;
    pSendFrame.complteFlag = 0xFF;

	pSendFrame.ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
	pSendFrame.pBuffer[0] = SUDDEN_ID;   //突发状态ID

	pSendFrame.pBuffer[1] = g_GetState.SwitchState1 | g_GetState.SwitchState2 | g_GetState.SwitchState3;	
	pSendFrame.pBuffer[2] = g_GetState.ExecuteOrder1 | g_GetState.ExecuteOrder2 | g_GetState.ExecuteOrder3;	
	pSendFrame.pBuffer[3] = g_GetState.CapState1 | g_GetState.CapState2 | g_GetState.CapState3;	
    
	if(!g_SystemState.warning)
	{
		pSendFrame.pBuffer[4] = 1;
	}
	else
	{
		pSendFrame.pBuffer[4] = 0;
	}
	pSendFrame.pBuffer[5] = g_SystemState.yuanBenState;  

    pSendFrame.len = 6;   //数据帧长度
    SendData(&pSendFrame);
}

/**
 * @description 外部中断1的中断处理函数
 * 思想： 选择是下降沿中断，在进入中断时对其下降沿时间进行检测，然后再主控端更改其为
 * 上升沿，此时不退出中断，检测上升沿的时间，以上条件都满足后，在下一个下降沿到来时
 * 选相同步合闸（退出中断）。
 */
void __attribute__((interrupt, no_auto_psv)) _INT2Interrupt(void)
{
    if((ReceiveStateFlag == TONGBU_HEZHA) && (CheckIsOverTime()))
    {
        OFF_INT();
        TongBuHeZha(g_offestTimeOne,g_offestTimeTwo);   //执行同步合闸命令
        UpdateLEDIndicateState(CAP3_LED,TURN_ON);
    }
    else
    {
        SendErrorFrame(0x05,OVER_TIME_ERROR);   //超时错误
    }
    TurnOffInt2();
    ReceiveStateFlag = IDLE_ORDER;
}