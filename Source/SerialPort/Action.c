/** 
 * <p>application name： Action.c</p> 
 * <p>application describing： 配置Action</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */
#include "../Header.h"
#include "../Yongci/DeviceParameter.h"
#include "../SerialPort/RefParameter.h"

#define  SUDDEN_ID 0x9A     //突发状态上传ID

void SendAckMesssage(uint8_t fun);
void GetLoopSwitch(struct DefFrameData* pReciveFrame);

extern uint8_t volatile SendFrameData[SEND_FRAME_LEN];
RemoteControlState g_RemoteControlState; //远方控制状态标识位

SystemSuddenState g_SuddenState;    //需要上传的机构状态值Action.h
struct DefFrameData g_qSendFrame;   //CAN数据帧

/**************************************************
 *函数名： SendAckMesssage()
 *功能：  回传校验码
 *形参：  Uint16 fun 功能代码地址
 *返回值：void
****************************************************/
inline void SendAckMesssage(uint8_t fun)
{
    uint16_t len = 0;
    ClrWdt();
    GenRTUFrame(LOCAL_ADDRESS, ACK, &fun, 1, (uint8_t*)SendFrameData, (uint8_t *)&len);
    ClrWdt();
    SendFrame((uint8_t*)SendFrameData, len);
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
                    if(g_lockUp == OFF_LOCK)    //解锁状态下不能进行合闸
                    {
                        return;
                    }
                    TongBuHeZha();
                    ClrWdt();
                    return ;
                }
                break;
            }
           case FENZHA: //立即分闸
            {
                if(g_SystemState.workMode == WORK_STATE) //多加入一重验证
                {
                    FENZHA_Action(SWITCH_ONE , g_DelayTime.fenzhaTime1);
                    FENZHA_Action(SWITCH_TWO , g_DelayTime.fenzhaTime2);
                    if(CAP3_STATE)
                    {
                        FENZHA_Action(SWITCH_THREE , g_DelayTime.fenzhaTime3);
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
                ClrWdt();
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
    uint8_t i = 1;    
    uint8_t id = pReciveFrame->pBuffer[0];
    uint8_t error = 0;    //错误
    uint8_t idIndex = 1;  //索引，读取属性值时可用到
    
    ClrWdt();
    uint8_t Data[8] = {0,0,0,0,0,0,0,0};
    PointUint8 Point;
    Point.pData = Data;
    Point.len = 8;

    /**
     * 发送数据帧赋值
     */
    pSendFrame->ID = MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
    g_qSendFrame.ID = MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
    pSendFrame->pBuffer[0] = id | 0x80;
    g_qSendFrame.pBuffer[0] = id | 0x80;  //数据传递
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
            g_qSendFrame.pBuffer[i] = pSendFrame->pBuffer[i];
		}
		pSendFrame->len = pReciveFrame->len;
	}
    g_qSendFrame.len = pReciveFrame->len;
    
    switch(id)
    {
        case 1: //合闸预制
        {
            if(g_SystemState.yuanBenState == BEN_STATE) //本地模式不能执行远方操作
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],WORK_MODE_ERROR);
                return;
            }
            if((pReciveFrame->pBuffer[1] < 1) || (pReciveFrame->pBuffer[1] > 7))    //回路数错误
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
            }
            if(g_RemoteControlState.ReceiveStateFlag == IDLE_ORDER)  //防止多次预制,防止多次合闸
            {                
                GetLoopSwitch(pReciveFrame);
                if(g_RemoteControlState.ReceiveStateFlag != IDLE_ORDER)
                {
                    OnLock();   //上锁
                    g_RemoteControlState.overTimeFlage = TRUE;  //预制成功后才会开启超时检测
                    SendData(pSendFrame);
                }
            }
            else
            {
                ClrWdt();
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                SendErrorFrame(pReciveFrame->pBuffer[0],SEVERAL_PERFABRICATE_ERROR);
            }
            break;
        }
        case 2: //合闸执行
        {
            g_RemoteControlState.orderId = pReciveFrame->pBuffer[0];    //获取命令ID号
            if((g_RemoteControlState.ReceiveStateFlag == HE_ORDER) && 
                (g_RemoteControlState.overTimeFlage == TRUE))
            {
                g_RemoteControlState.overTimeFlage = FALSE;  //Clear Overtime Flag
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;  //空闲命令                
                ClrWdt();               
                switch(pReciveFrame->pBuffer[1])
                {
                    case 0x01:
                    case 0x02:
                    case 0x04:
                    {
                        if(pReciveFrame->pBuffer[1] == 0x04)
                        {
                            pReciveFrame->pBuffer[1] --;
                        }
                        HEZHA_Action((pReciveFrame->pBuffer[1] - 1) , pReciveFrame->pBuffer[2]);
                        break;
                    }
                    case 0x03:
                    {
                        HEZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                        HEZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                        break;
                    }
                    case 0x05:
                    {
                        if(CAP3_STATE)
                        {
                            HEZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                            HEZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        break;
                    }
                    case 0x06:
                    {
                        if(CAP3_STATE)
                        {
                            HEZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                            HEZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        break;
                    }
                    case 0x07:
                    {
                        if(CAP3_STATE)
                        {
                            HEZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                            HEZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                            HEZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        break;
                    }
                    default :
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                        g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;  //Clear
                        OffLock();  //解锁
                        return;
                        break;
                    }
                }
            }
            else
            {
                ClrWdt();
                SendErrorFrame(pReciveFrame->pBuffer[0],NOT_PERFABRICATE_ERROR);
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;  //空闲命令
                g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;  //Clear
            }
            break;
        }
        case 3: //分闸预制
        {
            if(g_SystemState.yuanBenState == BEN_STATE) //本地控制不能执行远方操作错误
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],WORK_MODE_ERROR);
                return;
            }
            if((pReciveFrame->pBuffer[1] < 1) || (pReciveFrame->pBuffer[1] > 7))    //回路数错误
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
            }
            if(g_RemoteControlState.ReceiveStateFlag == IDLE_ORDER) //防止多次预制,防止多次分闸
            {
                GetLoopSwitch(pReciveFrame);
                if(g_RemoteControlState.ReceiveStateFlag != IDLE_ORDER)
                {
                    OnLock();   //上锁
                    g_RemoteControlState.overTimeFlage = TRUE;  //预制成功后才会开启超时检测
                    SendData(pSendFrame);
                }
            }
            else
            {
                ClrWdt();
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                SendErrorFrame(pReciveFrame->pBuffer[0],SEVERAL_PERFABRICATE_ERROR);
                OffLock();  //解锁
            }
            break;
        }
        case 4: //分闸执行
        {
            g_RemoteControlState.orderId = pReciveFrame->pBuffer[0];    //获取命令ID号
            if((g_RemoteControlState.ReceiveStateFlag == FEN_ORDER) && 
                (g_RemoteControlState.overTimeFlage == TRUE))
            {
                g_RemoteControlState.overTimeFlage = FALSE;  //Clear Overtime Flag          
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;  //空闲命令
                switch(pReciveFrame->pBuffer[1])
                {
                    case 0x01:
                    case 0x02:
                    case 0x04:
                    {
                        if(pReciveFrame->pBuffer[1] == 0x04)
                        {
                            pReciveFrame->pBuffer[1] --;
                        }
                        FENZHA_Action((pReciveFrame->pBuffer[1] - 1) , pReciveFrame->pBuffer[2]);
                        break;
                    }
                    case 0x03:
                    {
                        FENZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                        FENZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                        break;
                    }
                    case 0x05:
                    {
                        if(CAP3_STATE)
                        {
                            FENZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        break;
                    }
                    case 0x06:
                    {
                        if(CAP3_STATE)
                        {
                            FENZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        break;
                    }
                    case 0x07:
                    {
                        if(CAP3_STATE)
                        {
                            FENZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        break;
                    }
                    default :
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);    //回路数错误
                        OffLock();  //解锁
                        g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;  //Clear
                        return;
                        break;
                    }
                }
            }
            else
            {
                ClrWdt();
                SendErrorFrame(pReciveFrame->pBuffer[0],NOT_PERFABRICATE_ERROR);
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;  //空闲命令
                g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;  //Clear
                OffLock();  //解锁
            }
            break;
        }        
        case 5: //同步合闸预制
        {
            g_RemoteControlState.orderId = pReciveFrame->pBuffer[0];    //获取命令ID号
            //分合位错误
            if((g_SystemState.heFenState1 != CHECK_1_FEN_STATE) || 
               (g_SystemState.heFenState2 != CHECK_2_FEN_STATE) || 
               (g_SystemState.heFenState3 != CHECK_3_FEN_STATE))
            {
                SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                return;
            }
            //电容电压未达到设定电压错误
            if(GetCapVolatageState() == 0)
            {
                ClrWdt();
                SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                return;
            }
            //数据长度不对，数据长度不应为奇数
            if((pReciveFrame->len  & 0x01)) 
            {
                ClrWdt();
                SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                return;
            }
            pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1] & 0x03;
            pSendFrame->pBuffer[2] = (pReciveFrame->pBuffer[1] & 0x0C) >> 2;

            if(CAP3_STATE)
            {
                if((pReciveFrame->pBuffer[1] <= 0x34) && (pReciveFrame->len == 6)) //只有三个回路需要控制
                {
                    pSendFrame->pBuffer[3] = (pReciveFrame->pBuffer[1] & 0x30) >> 4;

                    if((pSendFrame->pBuffer[1] == pSendFrame->pBuffer[2]) || 
                       (pSendFrame->pBuffer[1] == pSendFrame->pBuffer[3]) || 
                       (pSendFrame->pBuffer[2] == pSendFrame->pBuffer[3]))//不应该出现相同的回路数
                    {
                        //错误消息为存在相同的回路数
                        SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                        return;
                    }
                    ClrWdt();
                    pSendFrame->pBuffer[4] = pReciveFrame->pBuffer[2];
                    pSendFrame->pBuffer[5] = pReciveFrame->pBuffer[3];
                    pSendFrame->pBuffer[6] = pReciveFrame->pBuffer[4];
                    pSendFrame->pBuffer[7] = pReciveFrame->pBuffer[5];
                    pSendFrame->len = 8;
                }
            }
            else
            {
                //只有两个回路需要控制
                if((pReciveFrame->pBuffer[1] == 0x01) || (pReciveFrame->pBuffer[1] == 0x04))  //机构1先合、或者机构2先合
                {
                    ClrWdt();
                    pSendFrame->pBuffer[3] = pReciveFrame->pBuffer[2];
                    pSendFrame->pBuffer[4] = pReciveFrame->pBuffer[3];
                    pSendFrame->len = 5;
                }
                else
                {
                    SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);    //回路数错误
                    return;
                }
            }
            //返回的数据帧赋值传递
            for(uint8_t i = 0;i < pSendFrame->len;i++)
            {
                g_qSendFrame.pBuffer[i] = pSendFrame->pBuffer[i];
                ClrWdt();
            }
            
            if(g_RemoteControlState.ReceiveStateFlag == IDLE_ORDER)
            {                
                ClrWdt();
                SendData(pSendFrame);
                ClrWdt();
                GetOffestTime(pReciveFrame , pSendFrame);
                ClrWdt();
                SetOverTime(g_SyncReadyWaitTime);   //设置同步预制超时等待时间
                g_RemoteControlState.ReceiveStateFlag = TONGBU_HEZHA;    //同步合闸命令
                g_RemoteControlState.overTimeFlage = TRUE;  //预制成功后才会开启超时检测
                TurnOnInt2();   //必须是在成功的预制之后才能开启外部中断1
            }
            else
            {
                TurnOffInt2();
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                SendErrorFrame(pReciveFrame->pBuffer[0],SEVERAL_PERFABRICATE_ERROR);    //多次预制
            }
            break;
        }
        case 0x10:  //顺序参数设置
        {
            ClrWdt();
            break;
        }
        case 0x11:  //非顺序参数设置
        {
            ClrWdt();
            pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1];  //配置号
            Point.pData = pReciveFrame->pBuffer + 2;
            error = SetParamValue(pReciveFrame->pBuffer[1],&Point);
            if(error == 0xFF)
            {
                ClrWdt();
                SendErrorFrame(pReciveFrame->pBuffer[0],ID_ERROR);
                return;
            }
            else if(error)
            {
                ClrWdt();
                SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                return;
            }
            pSendFrame->pBuffer[2] = Point.pData[0];
            pSendFrame->pBuffer[3] = Point.pData[1];
            pSendFrame->len = Point.len + 2;
            ClrWdt();
            SendData(pSendFrame);
            g_RemoteControlState.SetFixedValue = TRUE;  //设置定值成功
            
            break;
        }
        case 0x12:  //顺序参数读取
        {
            for(idIndex = pReciveFrame->pBuffer[1];idIndex <= pReciveFrame->pBuffer[2];idIndex++)    //抛除ID号所占的长度
            {
                ClrWdt();
                Point.len = 8;
                error = ReadParamValue(idIndex,&Point);
                if((error == 0xF1)||(error == 0xF3))    //数据长度错误
                {
                    ClrWdt();
                    SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                }
                pSendFrame->pBuffer[1] = idIndex;  //配置号  
                for(uint8_t i = 0;i < Point.len;i++)
                {
                    pSendFrame->pBuffer[i + 2] = Point.pData[i];
                }
                pSendFrame->len = Point.len + 2;
                ClrWdt();
                SendData(pSendFrame);
            }
            break;
        }        
        case 0x13:  //非顺序参数读取
        {
            for(i = 1;i < pReciveFrame->len;i++)    //抛除ID号所占的长度
            {
                ClrWdt();
                Point.len = 8;
                error = ReadParamValue(pReciveFrame->pBuffer[i],&Point);
                if((error == 0xF1)||(error == 0xF3))    //数据长度错误
                {
                    ClrWdt();
                    SendErrorFrame(pReciveFrame->pBuffer[0],DATA_LEN_ERROR);
                }
                if((error == 0xF2)||(error == 0xF4))    //ID号错误
                {
                    ClrWdt();
                    SendErrorFrame(pReciveFrame->pBuffer[0],ID_ERROR);
                }
                pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[i];  //配置号
                pSendFrame->pBuffer[2] = Point.pData[0];
                pSendFrame->pBuffer[3] = Point.pData[1];
                pSendFrame->len = Point.len + 2;
                ClrWdt();
                SendData(pSendFrame);
            }
            break;
        }
        default:
        {
            //错误的ID号处理
            ClrWdt();
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
void SendErrorFrame(uint8_t receiveID,uint8_t errorID)
{
    uint8_t data[8] = {0,0,0,0,0,0,0,0};
    struct DefFrameData pSendFrame;
    
    pSendFrame.pBuffer = data;
    pSendFrame.complteFlag = 0xFF;
    ClrWdt();
    pSendFrame.ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
    pSendFrame.pBuffer[0] = ERROR_REPLY_ID;   //错误应答ID
    pSendFrame.pBuffer[1] = receiveID;  //主站发送ID
    pSendFrame.pBuffer[2] = errorID;   //错误代码
    pSendFrame.pBuffer[3] = ERROR_EXTEND_ID;  //扩展ID号            
    pSendFrame.len = ERROR_DATA_LEN;   //错误帧长度
    ClrWdt();
    SendData(&pSendFrame);
}

/**
 * @description: 发送错误帧数据
 * @param data 发送的数据
 */
void SendMessage(uint16_t sendData)
{
    uint8_t highData = (sendData & 0xFF00) >> 8;
    uint8_t lowData = sendData & 0x00FF;
    uint8_t data[8] = {0,0,0,0,0,0,0,0};
    struct DefFrameData pSendFrame;
    
    pSendFrame.pBuffer = data;
    pSendFrame.complteFlag = 0xFF;
    
    ClrWdt();
    pSendFrame.ID =  MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
    pSendFrame.pBuffer[0] = 0x15;       //应答ID
    pSendFrame.pBuffer[1] = highData;   //主站发送ID
    pSendFrame.pBuffer[2] = lowData;    //错误代码     
    pSendFrame.pBuffer[3] = 0x5A;       //扩展代码        
    pSendFrame.len = 4;   //错误帧长度
    ClrWdt();
    SendData(&pSendFrame);
}
/**
 * 
 * <p>Function name: [UpdataState]</p>
 * <p>Discription: [对运行状态进行更新显示]</p>
 */
void UpdataState(void)
{
    uint8_t Data[8] = {0,0,0,0,0,0,0,0};

    struct DefFrameData pSendFrame;

    pSendFrame.pBuffer = Data;
    pSendFrame.complteFlag = 0xFF;

    ClrWdt();
    pSendFrame.ID = MAKE_GROUP1_ID(GROUP1_STATUS_CYCLE_ACK, DeviceNetObj.MACID);
	pSendFrame.pBuffer[0] = SUDDEN_ID;   //突发状态ID

	pSendFrame.pBuffer[1] = g_SuddenState.SwitchState1 | g_SuddenState.SwitchState2 | g_SuddenState.SwitchState3;	
	pSendFrame.pBuffer[2] = g_SuddenState.ExecuteOrder1 | g_SuddenState.ExecuteOrder2 | g_SuddenState.ExecuteOrder3;	
	pSendFrame.pBuffer[3] = g_SuddenState.CapState1 | g_SuddenState.CapState2 | g_SuddenState.CapState3;	
    
	if(!g_SystemState.warning)
	{
        ClrWdt();
		pSendFrame.pBuffer[4] = 1;
	}
	else
	{
        ClrWdt();
		pSendFrame.pBuffer[4] = 0;
	}
	pSendFrame.pBuffer[5] = g_SystemState.yuanBenState;  

    pSendFrame.len = 6;   //数据帧长度
    SendData(&pSendFrame);
    
    ClrWdt();
    g_SuddenState.ExecuteOrder1 = 0;
    g_SuddenState.ExecuteOrder2 = 0;
    g_SuddenState.ExecuteOrder3 = 0;
}

/**
 * <p>Function name: [_INT2Interrupt]</p>
 * <p>Discription: [外部中断函数]</p>
 */
void __attribute__((interrupt, no_auto_psv)) _INT2Interrupt(void)
{
    uint8_t i = 0;
    uint8_t j = 0;
    uint8_t validCount = 0;
    uint8_t idleCount = 0;
    
    IFS1bits.INT2IF = 0;
    OFF_INT();
    //*************************************************************************************
    //以下判断防止误触发
    if(g_RemoteControlState.ReceiveStateFlag != TONGBU_HEZHA)   //判断是否执行了同步合闸预制
    {
        ON_INT();
        TurnOffInt2();
        return;
    }
    if(g_RemoteControlState.overTimeFlage == FALSE) //发生超时
    {
        ON_INT();
        TurnOffInt2();
        return;
    }
    //*************************************************************************************
    //第一次的高电平持续时间判断
    for(i = 0;i < 21;i++)
    {
        if(RXD1_LASER == 1)
        {
            validCount++;
        }
        else
        {
            idleCount++;
        }
    }
    if(idleCount >= 2)
    {
        return;
    }
    validCount = 0;
    idleCount = 0;
    //判断四次高低电平持续时间
    for(j = 0;j < 4;j++)
    {        
        validCount = 0;
        idleCount = 0;
        for(i = 0;i < 25;i++)
        {
            if(RXD1_LASER == 1)
            {
                validCount++;
            }
            else
            {
                idleCount++;
            }
        }
        if((RXD1_LASER == 1) && (idleCount > 2))
        {
            return;
        }
        else if((RXD1_LASER == 0) && (validCount > 2))
        {
            return;
        }
    }
    //查找最后一个低电平
    for(i = 0;i < 20;i++)
    {
        if(RXD1_LASER == 1)
        {
            validCount++;
        }
        else
        {
            idleCount++;
        }
        if(idleCount >= 5)
        {
            break;
        }
    }
    validCount = 0;
    while(!RXD1_LASER)
    {
        validCount++;
        if(validCount > 10)
        {
            break;
        }
    }
    if(RXD1_LASER || (validCount > 10))
    {
        g_RemoteControlState.lastReceiveOrder = TONGBU_HEZHA;
        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
        g_RemoteControlState.overTimeFlage = FALSE;  //Clear Overtime Flag   
        TongBuHeZha();
        ON_INT();
        TurnOffInt2();
    }
}

/**
 * 
 * <p>Function name: [GetLoopSwitch]</p>
 * <p>Discription: [获取需要动作的回路开关]</p>
 * @param pReciveFrame 接收的数据
 */
void GetLoopSwitch(struct DefFrameData* pReciveFrame)
{
	uint8_t orderID = pReciveFrame->pBuffer[0];
	uint8_t loop = pReciveFrame->pBuffer[1];
    
    if(orderID == 1)
	{
		g_RemoteControlState.ReceiveStateFlag = HE_ORDER;    //合闸命令
	}	
	else if(orderID == 3)
	{
		g_RemoteControlState.ReceiveStateFlag = FEN_ORDER;   //分闸命令
	}
	switch (loop)
	{
		case 1:
		{
			if(g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down)
			{
                ClrWdt();
                if(g_RemoteControlState.ReceiveStateFlag == HE_ORDER)
                {                    
                    if(g_SystemState.heFenState1 != CHECK_1_FEN_STATE)
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                        return;
                    }
                    g_RemoteControlState.lastReceiveOrder = CHECK_1_HE_ORDER;
                }
                else if(g_RemoteControlState.ReceiveStateFlag == FEN_ORDER)
                {   
                    if(g_SystemState.heFenState1 != CHECK_1_HE_STATE)
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                        return;
                    }
                    g_RemoteControlState.lastReceiveOrder = CHECK_1_FEN_ORDER;
                }
                SetOverTime(g_RemoteWaitTime);   //设置预制超时等待时间
			}
			else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                return;
			}
			break;
		}
		case 2:
		{
			if(g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down)
			{
                ClrWdt();
                if(g_RemoteControlState.ReceiveStateFlag == HE_ORDER)
                {                    
                    if(g_SystemState.heFenState2 != CHECK_2_FEN_STATE)
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                        return;
                    }
                    g_RemoteControlState.lastReceiveOrder = CHECK_2_HE_ORDER;
                }
                else if(g_RemoteControlState.ReceiveStateFlag == FEN_ORDER)
                {
                    if(g_SystemState.heFenState2 != CHECK_2_HE_STATE)
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                        return;
                    }
                    g_RemoteControlState.lastReceiveOrder = CHECK_2_FEN_ORDER;
                }
                SetOverTime(g_RemoteWaitTime);   //设置预制超时等待时间
			}
			else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                return;
			}
			break;
		}
		case 4:
		{
			if(CAP3_STATE)
			{
				if(g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down)
				{
                	ClrWdt();
                    if(g_RemoteControlState.ReceiveStateFlag == HE_ORDER)
                    {                    
                        if(g_SystemState.heFenState3 != CHECK_3_FEN_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_3_HE_ORDER;
                    }
                    else if(g_RemoteControlState.ReceiveStateFlag == FEN_ORDER)
                    {
                        if(g_SystemState.heFenState3 != CHECK_3_HE_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_3_FEN_ORDER;
                    }
                	SetOverTime(g_RemoteWaitTime);   //设置预制超时等待时间
				}
				else
				{
					g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
					SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                    return;
				}
			}
			else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
			break;
		}
		case 3:
		{
			if((g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down) && 
		       (g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down))
			{
                ClrWdt();
                if(g_RemoteControlState.ReceiveStateFlag == HE_ORDER)
                {                          
                    if(g_SystemState.heFenState1 != CHECK_1_FEN_STATE && 
                       g_SystemState.heFenState2 != CHECK_2_FEN_STATE)
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                        return;
                    }
                    g_RemoteControlState.lastReceiveOrder = CHECK_Z_HE_ORDER;
                }
                else if(g_RemoteControlState.ReceiveStateFlag == FEN_ORDER)
                {              
                    if(g_SystemState.heFenState1 != CHECK_1_HE_STATE && 
                       g_SystemState.heFenState2 != CHECK_2_HE_STATE)
                    {
                        SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                        return;
                    }
                    g_RemoteControlState.lastReceiveOrder = CHECK_Z_FEN_ORDER;
                }
                SetOverTime(g_RemoteWaitTime);   //设置预制超时等待时间
			}
			else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                return;
			}
			break;
		}
		case 5: 
		{
			if(CAP3_STATE)
			{
				if((g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down) && 
		       	   (g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down))
				{
                	ClrWdt();
                    if(g_RemoteControlState.ReceiveStateFlag == HE_ORDER)
                    {                           
                        if(g_SystemState.heFenState1 != CHECK_1_FEN_STATE && 
                           g_SystemState.heFenState3 != CHECK_3_FEN_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_Z_HE_ORDER;
                    }
                    else if(g_RemoteControlState.ReceiveStateFlag == FEN_ORDER)
                    {
                        if(g_SystemState.heFenState1 != CHECK_1_HE_STATE && 
                           g_SystemState.heFenState3 != CHECK_3_HE_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_Z_FEN_ORDER;
                    }
                	SetOverTime(g_RemoteWaitTime);   //设置预制超时等待时间
				}
				else
				{
					g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
					SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                    return;
				}
			}
			else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
			break;
		}
		case 6:
		{
			if(CAP3_STATE)
			{
				if((g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down) && 
		       	   (g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down))
				{
                	ClrWdt();
                    if(g_RemoteControlState.ReceiveStateFlag == HE_ORDER)
                    {                           
                        if(g_SystemState.heFenState3 != CHECK_3_FEN_STATE && 
                           g_SystemState.heFenState2 != CHECK_2_FEN_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_Z_HE_ORDER;
                    }
                    else if(g_RemoteControlState.ReceiveStateFlag == FEN_ORDER)
                    {
                        if(g_SystemState.heFenState3 != CHECK_3_HE_STATE && 
                           g_SystemState.heFenState2 != CHECK_2_HE_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_Z_FEN_ORDER;
                    }
                	SetOverTime(g_RemoteWaitTime);   //设置分闸预制超时等待时间
				}
				else
				{
					g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
					SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                    return;
				}
			}
			else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
			break;
		}
		case 7:
		{
			if(CAP3_STATE)
			{
				if((g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down) && 
		       	   (g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down) && 
		       	   (g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down))
				{
                	ClrWdt();
                    if(g_RemoteControlState.ReceiveStateFlag == HE_ORDER)
                    {                           
                        if(g_SystemState.heFenState1 != CHECK_1_FEN_STATE && 
                           g_SystemState.heFenState2 != CHECK_2_FEN_STATE && 
                           g_SystemState.heFenState3 != CHECK_3_FEN_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_Z_HE_ORDER;
                    }
                    else if(g_RemoteControlState.ReceiveStateFlag == FEN_ORDER)
                    {
                        if(g_SystemState.heFenState1 != CHECK_1_HE_STATE && 
                           g_SystemState.heFenState2 != CHECK_2_HE_STATE && 
                           g_SystemState.heFenState3 != CHECK_3_HE_STATE)
                        {
                            SendErrorFrame(pReciveFrame->pBuffer[0],HEFEN_STATE_ERROR);
                            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                            return;
                        }
                        g_RemoteControlState.lastReceiveOrder = CHECK_Z_FEN_ORDER;
                    }
                	SetOverTime(g_RemoteWaitTime);   //设置分闸预制超时等待时间
				}
				else
				{
					g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
					SendErrorFrame(pReciveFrame->pBuffer[0],CAPVOLTAGE_ERROR);
                    return;
				}
			}
			else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
			break;
		}
		default:
		{
			g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
			SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
			break;
		}
	}
}

/**
 * 
 * <p>Function name: [CheckOrder]</p>
 * <p>Discription: [检测命令是否正确执行]</p>
 * @param lastOrder 上一次执行的命令
 */
void CheckOrder(uint16_t lastOrder)
{
    //远方与就地命令的转换
    if((lastOrder == IDLE_ORDER) && (g_RemoteControlState.lastReceiveOrder != IDLE_ORDER))
    {
        lastOrder = g_RemoteControlState.lastReceiveOrder;        
        g_qSendFrame.complteFlag = TRUE;
        if(lastOrder != TONGBU_HEZHA)
        {
            SendData(&g_qSendFrame); //发送执行反馈指令
        }        
    }
    switch(lastOrder)
    {
        case TONGBU_HEZHA:
        case CHECK_Z_HE_ORDER:
        {
            if(g_SystemState.heFenState1 != CHECK_1_HE_STATE || 
               g_SystemState.heFenState2 != CHECK_2_HE_STATE || 
               g_SystemState.heFenState3 != CHECK_3_HE_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = Z_HE_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        case CHECK_Z_FEN_ORDER:
        {
            if(g_SystemState.heFenState1 != CHECK_1_FEN_STATE || 
               g_SystemState.heFenState2 != CHECK_2_FEN_STATE || 
               g_SystemState.heFenState3 != CHECK_3_FEN_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = Z_FEN_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        case CHECK_1_HE_ORDER:
        {
            if(g_SystemState.heFenState1 != CHECK_1_HE_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = JIGOU1_HE_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        case CHECK_1_FEN_ORDER:
        {
            if(g_SystemState.heFenState1 != CHECK_1_FEN_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = JIGOU1_FEN_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        case CHECK_2_HE_ORDER:
        {
            if(g_SystemState.heFenState2 != CHECK_2_HE_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = JIGOU2_HE_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        case CHECK_2_FEN_ORDER:
        {
            if(g_SystemState.heFenState2 != CHECK_2_FEN_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = JIGOU2_FEN_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        case CHECK_3_HE_ORDER:
        {
            if(CAP3_STATE == 0x00)
            {
                break;
            }
            if(g_SystemState.heFenState3 != CHECK_3_HE_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = JIGOU3_HE_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        case CHECK_3_FEN_ORDER:
        {
            if(CAP3_STATE == 0x00)
            {
                break;
            }
            if(g_SystemState.heFenState3 != CHECK_3_FEN_STATE)
            {
                SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                g_SuddenState.RefuseAction = JIGOU3_FEN_ERROR;  //发生拒动
            }
            else
            {
                g_SuddenState.RefuseAction = FALSE;  //未发生拒动
            }
            break;
        }
        default:
        {
            break;
        }
    }
    if(g_SuddenState.RefuseAction != FALSE)
    {
        g_changeLedTime = 100;  //发生拒动错误后，指示灯闪烁间隔变短
    }
    else
    {
        g_changeLedTime = 500;  //正常状态下指示灯闪烁的间隔
    }
    g_SuddenState.SuddenFlag = TRUE;  //发送突发错误
}


