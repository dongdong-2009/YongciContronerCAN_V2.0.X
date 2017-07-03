
#include "Action.h"

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
#include <string.h>
#define  SUDDEN_ID 0x9A     //突发状态上传ID

void SendAckMesssage(uint8_t fun);
void GetLoopSwitch(struct DefFrameData* pReciveFrame);
uint8_t SynCloseReady(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame);
uint8_t GetLoopSet(struct DefFrameData* pReciveFrame);
uint8_t CheckOpenCondition(void);
uint8_t CheckCloseCondition(void);

extern uint8_t volatile SendFrameData[SEND_FRAME_LEN];
RemoteControlState g_RemoteControlState; //远方控制状态标识位

/**
 *同步合闸参数
 */
//ActionParameter g_ActionParameter[3];
/**
 * 同步动作属性
 */
ActionAttribute g_SynActionAttribute;

/**
 * 普通合分闸动作属性
 */
ActionAttribute g_NormalAttribute;


SystemSuddenState g_SuddenState;    //需要上传的机构状态值Action.h
struct DefFrameData g_qSendFrame;   //CAN数据帧
PointUint8 g_pPoint;   

uint8_t static g_overID = 0;
uint8_t static g_startID = 0;


/*
 * Action参数初始化
 */
void ActionParameterInit(void)
{
    g_SynActionAttribute.count = 0;
    g_SynActionAttribute.loopByte = 0;
    g_SynActionAttribute.currentIndex = 0; 
     
    g_SynActionAttribute.Attribute[0].enable = 0;
    g_SynActionAttribute.Attribute[0].loop = 0;    
    g_SynActionAttribute.Attribute[0].offsetTime = 0;
    g_SynActionAttribute.Attribute[0].readyFlag = 0;
      
  
    g_SynActionAttribute.Attribute[1].enable = 0;
    g_SynActionAttribute.Attribute[1].loop = 0;   
    g_SynActionAttribute.Attribute[1].offsetTime = 0;
    g_SynActionAttribute.Attribute[1].readyFlag = 0;
   
    

    g_SynActionAttribute.Attribute[2].enable = 0;
    g_SynActionAttribute.Attribute[2].loop = 0;
    g_SynActionAttribute.Attribute[2].offsetTime = 0;
    g_SynActionAttribute.Attribute[2].readyFlag = 0;
  
    g_NormalAttribute.count = 0;
    g_NormalAttribute.loopByte = 0;
    g_NormalAttribute.currentIndex = 0; 
     
    g_NormalAttribute.Attribute[0].enable = 0;
    g_NormalAttribute.Attribute[0].loop = 0;    
    g_NormalAttribute.Attribute[0].offsetTime = 0;
    g_NormalAttribute.Attribute[0].readyFlag = 0;
      
  
    g_NormalAttribute.Attribute[1].enable = 0;
    g_NormalAttribute.Attribute[1].loop = 0;   
    g_NormalAttribute.Attribute[1].offsetTime = 0;
    g_NormalAttribute.Attribute[1].readyFlag = 0;
   
    

    g_NormalAttribute.Attribute[2].enable = 0;
    g_NormalAttribute.Attribute[2].loop = 0;
    g_NormalAttribute.Attribute[2].offsetTime = 0;
    g_NormalAttribute.Attribute[2].readyFlag = 0;   
    
    
}

/**
 * 暂存命令字
 */
uint8_t LastCommand[8] = {0};//最新的命令字
/**
 * 引用帧服务
 *
 * @param  指向接收数据的指针
 * @param  指向发送数据的指针
 * @bref   对完整帧进行提取判断
 */
void FrameServer(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    if(pReciveFrame->pBuffer[0] > 0x13)
    {
        return;
    }
    uint8_t i = 1;    
    uint8_t id = pReciveFrame->pBuffer[0];
    uint8_t error = 0;  //错误号
    uint8_t result = 0;
    
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
    
	if(id <= 5)
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
        case ReadyClose : //合闸预制
        {
            
            if(g_SystemState.yuanBenState == BEN_STATE) //本地模式不能执行远方操作
            {
                SendErrorFrame(id, WORK_MODE_ERROR);
                return;
            }      
             if (g_RemoteControlState.ReceiveStateFlag != IDLE_ORDER)//检测是否多次预制
            {                
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                SendErrorFrame(id, SEVERAL_PERFABRICATE_ERROR);
                return;
            }   
            result = GetLoopSet(pReciveFrame);   //获取回路   
            if (result)
            {
                 SendErrorFrame(id, result);
                return;
            }                   
            result =  CheckCloseCondition(); //检测合分闸条件
            if (result)
            {
                 SendErrorFrame(id, result);
                return;
            } 
            ClrWdt();  
            
            memcpy(LastCommand, pReciveFrame->pBuffer, pReciveFrame->len);//暂存预制指令
            g_RemoteControlState.overTimeFlag = TRUE;  //预制成功后才会开启超时检测
            SendData(pSendFrame);
            g_RemoteControlState.ReceiveStateFlag = HE_ORDER;    //合闸命令
            SetOverTime(g_RemoteWaitTime);   //设置预制超时等待时间         
            break;
        }
        case CloseAction: //合闸执行
        {
             if(g_SystemState.yuanBenState == BEN_STATE) //本地模式不能执行远方操作
            {
                SendErrorFrame(id, WORK_MODE_ERROR);
                return;
            }   
             //是否已经预制或者超时                 
            if((g_RemoteControlState.ReceiveStateFlag != HE_ORDER) || 
                (g_RemoteControlState.overTimeFlag != TRUE))
            {
                ClrWdt();
                SendErrorFrame(id, NOT_PERFABRICATE_ERROR);
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;  //空闲命令
                g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;  //Clear
            
            }
               //比较配置字是否一致
            for(uint8_t i = 1 ; i < pReciveFrame->len; i++)
            {
                if (pReciveFrame->pBuffer[i] != LastCommand[i])
                {
                    SendErrorFrame(id, ERROR_DIFF_CONFIG);
                    return;
                }
            }      
            g_RemoteControlState.orderId = id;
            OnLock();   //上锁             
            g_RemoteControlState.overTimeFlag = FALSE;  //Clear Overtime Flag
            g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;  //空闲命令                
            ClrWdt();               
            SendData(pSendFrame);
            CloseOperation();
            
            break;
        }
        case ReadyOpen: //分闸预制
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
                    g_RemoteControlState.overTimeFlag = TRUE;  //预制成功后才会开启超时检测
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
        case OpenAction: //分闸执行
        {
            g_RemoteControlState.orderId = pReciveFrame->pBuffer[0];    //获取命令ID号
            if((g_RemoteControlState.ReceiveStateFlag == FEN_ORDER) && 
                (g_RemoteControlState.overTimeFlag == TRUE))
            {
                g_RemoteControlState.overTimeFlag = FALSE;  //Clear Overtime Flag          
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
                        #if(CAP3_STATE)
                        {
                            FENZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        #endif
                        break;
                    }
                    case 0x06:
                    {
                        #if(CAP3_STATE)
                        {
                            FENZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        #endif
                        break;
                    }
                    case 0x07:
                    {
                        #if(CAP3_STATE)
                        {
                            FENZHA_Action(SWITCH_ONE , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_TWO , pReciveFrame->pBuffer[2]);
                            FENZHA_Action(SWITCH_THREE , pReciveFrame->pBuffer[2]);
                        }
                        #endif
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
        case SyncReadyClose: //同步合闸预制
        {
            SynCloseReady(pReciveFrame, pSendFrame);
            break;
        }
       
        case MasterParameterSetOne:  //非顺序参数设置
        {
            ClrWdt();
            pSendFrame->pBuffer[1] = pReciveFrame->pBuffer[1];  //配置号
            g_pPoint.pData = pReciveFrame->pBuffer + 2;
            error = SetParamValue(pReciveFrame->pBuffer[1],&g_pPoint);
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
            pSendFrame->pBuffer[2] = g_pPoint.pData[0];
            pSendFrame->pBuffer[3] = g_pPoint.pData[1];
            pSendFrame->len = g_pPoint.len + 2;
            ClrWdt();
            SendData(pSendFrame);
            g_RemoteControlState.SetFixedValue = TRUE;  //设置定值成功
            
            break;
        }
        case MasterParameterRead:  //顺序参数读取
        {
            g_startID = pReciveFrame->pBuffer[1];
            g_overID = pReciveFrame->pBuffer[2];
            g_RemoteControlState.GetAllValueFalg = TRUE;
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


uint8_t  SynCloseReady(struct DefFrameData* pReciveFrame, struct DefFrameData* pSendFrame)
{
    uint8_t id = 0;
    uint8_t configbyte = 0;
    uint8_t loop[3] = {0};
    //数据长度不对，数据长度不应为奇数
    if(pReciveFrame->len % 2 != 0) 
    {
        ClrWdt();      
        return DATA_LEN_ERROR;
    }
    //必须不小于4
    if (pReciveFrame->len < 2)
    {
        return DATA_LEN_ERROR;
    }
        
    id  = pReciveFrame->pBuffer[0];
    configbyte = pReciveFrame->pBuffer[1];
           
    //分合位错误
    if((g_SystemState.heFenState1 != CHECK_1_FEN_STATE) || 
       (g_SystemState.heFenState2 != CHECK_2_FEN_STATE) || 
       (g_SystemState.heFenState3 != CHECK_3_FEN_STATE))
    {       
        return HEFEN_STATE_ERROR;
    }   
    
    //电容电压未达到设定电压错误
    if(GetCapVolatageState() == 0)
    {
        ClrWdt();
        SendErrorFrame(id, CAPVOLTAGE_ERROR);
        return CAPVOLTAGE_ERROR;
    }
    
    uint8_t count = (pReciveFrame->len-2) / 2 + 1;//参数时间为差值，比回路数少一个
    
     //检查回路数
    if (count > 3)
    {
           return LOOP_ERROR;
    }
     //获取回路参数   
    for(uint8_t i = 0; i < count; i++)
    {
        loop[i] = (uint8_t)((configbyte>>(2*i)) & 0x03); 
        if (loop[i] == 0)
        {
            return  LOOP_ERROR;
        }
        //TODO:添加回路数限制        
    }
    
    //简单查重
    if (count == 2)
    {
        if (loop[0] == loop[1])
        {
            return LOOP_ERROR;
        }
    }
    else  if (count == 3)
    {
        if ((loop[0] == loop[1]) || (loop[1] == loop[2])
                || (loop[0] == loop[2]))
        {
            return LOOP_ERROR;
        } 
    }
    
    //按照执行顺序参数赋值    
     g_SynActionAttribute.count = count;
     g_SynActionAttribute.loopByte = configbyte;
     g_SynActionAttribute.currentIndex = 0;
      
     g_SynActionAttribute.Attribute[0].enable = TRUE;
     g_SynActionAttribute.Attribute[0].loop = loop[0];        
     g_SynActionAttribute.Attribute[0].offsetTime = 0;
        
    uint16_t time = 0;
    for(uint8_t i = 1; i < count; i++)
    {
         
        g_SynActionAttribute.Attribute[i].enable = TRUE;
        g_SynActionAttribute.Attribute[i].loop = loop[i];       
        time = pReciveFrame->pBuffer[2*i + 1];   
        time = (time<<8)|  pReciveFrame->pBuffer[2*i];   
        g_SynActionAttribute.Attribute[i].offsetTime = time;
    }   
    for (uint8_t i = count; i < 3;i++)
    {
         g_SynActionAttribute.Attribute[i].enable = FALSE;
    }
    
    g_RemoteControlState.orderId = id;
    
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
        SetOverTime(g_SyncReadyWaitTime);   //设置同步预制超时等待时间
        ClrWdt();
        g_RemoteControlState.ReceiveStateFlag = TONGBU_HEZHA;    //同步合闸命令
        g_RemoteControlState.overTimeFlag = TRUE;  //预制成功后才会开启超时检测
        TurnOnInt2();   //必须是在成功的预制之后才能开启外部中断1
       
    }
    else
    {
        TurnOffInt2();
        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
        return SEVERAL_PERFABRICATE_ERROR;    //多次预制
    }
    return 0;
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
    uint8_t data[8] = {0,0,0,0,0,0,0,0};

    struct DefFrameData pSendFrame;

    pSendFrame.pBuffer = data;
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

uint16_t g_validCount = 0;
void __attribute__((interrupt, no_auto_psv)) _INT2Interrupt(void)
{
    uint8_t i = 0;
    uint8_t state = 0x00;
    
    g_validCount = 0;   //初始化全局变量
    
    IFS1bits.INT2IF = 0;
    OFF_COMMUNICATION_INT();
    //*************************************************************************************
    //以下判断防止误触发
    if(g_RemoteControlState.ReceiveStateFlag != TONGBU_HEZHA)   //判断是否执行了同步合闸预制
    {
        ON_COMMUNICATION_INT();
        TurnOffInt2();
        return;
    }
    if(g_RemoteControlState.overTimeFlag == FALSE) //发生超时
    {
        ON_COMMUNICATION_INT();
        TurnOffInt2();
        return;
    }
	//检测高电平
	for(i = 0; i < 4; i++)
	{
		state = ~state;
		state &= 0x01;
		while(RXD1_LASER == state)
		{
			g_validCount++;
            if(g_validCount >= 0xFFFF)    //防止进入死循环//TODO:设定最大值
            {
                i = 10;
                return;
            }
		}
		if((g_validCount < 30) || (g_validCount > 50))  //较宽的范围85~125
		{
            i = 10;
			return;
		}
        g_validCount = 0;
	}
	if((RXD1_LASER == 1) && (i == 4))
	{
        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
        g_RemoteControlState.overTimeFlag = FALSE;  //Clear Overtime Flag   
        SynCloseAction();
        g_RemoteControlState.lastReceiveOrder = TONGBU_HEZHA;
        TurnOffInt2();
	}
}



/**
 * 获取合分闸设置回路参数
 * @retrun 错误代码
 */
uint8_t GetLoopSet(struct DefFrameData* pReciveFrame)
{
    if(pReciveFrame->len != 3)
    {
        return DATA_LEN_ERROR;
    }
    uint8_t configLoop =  pReciveFrame->pBuffer[1];//回路选择配置字
    uint8_t actionTime =  pReciveFrame->pBuffer[2];//合分闸时间
    if ((configLoop == 0) || (configLoop > 0x07))
    {
        return  LOOP_ERROR;
    }    
     g_NormalAttribute.count = 0;
    for(uint8_t i = 0; i < 3; i++)
    {       
        if (configLoop & (1<<i))
        {
            g_NormalAttribute.Attribute[i].enable = TRUE;
            g_NormalAttribute.Attribute[i].loop = i+1;
            g_NormalAttribute.Attribute[i].offsetTime = 0;            
            g_NormalAttribute.count++;            
        }
        else
        {
            g_NormalAttribute.Attribute[i].enable = FALSE;
        }
            
    }
    g_NormalAttribute.powerOnTime = actionTime;   
    return 0;
}

/**
 * 检查合闸条件
 * @return 错误代码
 */
uint8_t CheckCloseCondition(void)
{
    
    for(uint8_t i = 0; i< 3; i++)
    {
        if( g_NormalAttribute.Attribute[i].enable)
        {
            switch( g_NormalAttribute.Attribute[i].loop)
            {
                case 1:
                {
                    if(g_SystemVoltageParameter.voltageCap1  < g_SystemLimit.capVoltage1.down)
                    {
                        return CAPVOLTAGE_ERROR;
                    }
                    if (g_SystemState.heFenState1 != CHECK_1_FEN_STATE)
                    {
                        return HEFEN_STATE_ERROR;
                    }

                    break;
                }
                case 2:
                {
                    if(g_SystemVoltageParameter.voltageCap2  < g_SystemLimit.capVoltage2.down)
                    {
                        return CAPVOLTAGE_ERROR;
                    }
                    if (g_SystemState.heFenState2 != CHECK_2_FEN_STATE)
                    {
                        return HEFEN_STATE_ERROR;
                    }
                    break;
                }
                case 3:
                {
                     if(g_SystemVoltageParameter.voltageCap3  < g_SystemLimit.capVoltage3.down)
                    {
                        return CAPVOLTAGE_ERROR;
                    }
                    if (g_SystemState.heFenState3 != CHECK_3_FEN_STATE)
                    {
                        return HEFEN_STATE_ERROR;
                    }
                     break;
                }
                default:
                {
                    return  LOOP_ERROR;
                }

            }
        }
    }
    if (g_NormalAttribute.count != 0)
    {
        return 0;
    }
    else
    {
         return  LOOP_ERROR;
    }
    
}

/**
 * 检查分闸条件
 * @return 错误代码
 */
uint8_t CheckOpenCondition(void)
{
    
    for(uint8_t i = 0; i< 3; i++)
    {
        if ( g_NormalAttribute.Attribute[i].enable)
        {
            switch( g_NormalAttribute.Attribute[i].loop)
            {
                case 1:
                {
                    if(g_SystemVoltageParameter.voltageCap1  < g_SystemLimit.capVoltage1.down)
                    {
                        return CAPVOLTAGE_ERROR;
                    }
                    if (g_SystemState.heFenState1 != CHECK_1_HE_STATE)
                    {
                        return HEFEN_STATE_ERROR;
                    }

                    break;
                }
                case 2:
                {
                    if(g_SystemVoltageParameter.voltageCap2  < g_SystemLimit.capVoltage2.down)
                    {
                        return CAPVOLTAGE_ERROR;
                    }
                    if (g_SystemState.heFenState2 != CHECK_2_HE_STATE)
                    {
                        return HEFEN_STATE_ERROR;
                    }
                    break;
                }
                case 3:
                {
                     if(g_SystemVoltageParameter.voltageCap3  < g_SystemLimit.capVoltage3.down)
                    {
                        return CAPVOLTAGE_ERROR;
                    }
                    if (g_SystemState.heFenState3 != CHECK_3_HE_STATE)
                    {
                        return HEFEN_STATE_ERROR;
                    }
                     break;
                }
                default:
                {
                    return  LOOP_ERROR;
                }
            }
                            
        }
    }
    if (g_NormalAttribute.count != 0)
    {
        return 0;
    }
    else
    {
         return  LOOP_ERROR;
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
			#if(CAP3_STATE)
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
			#else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
            #endif
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
			#if(CAP3_STATE)
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
			#else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
            #endif
			break;
		}
		case 6:
		{
			#if(CAP3_STATE)
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
			#else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
            #endif
			break;
		}
		case 7:
		{
			#if(CAP3_STATE)
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
			#else
			{
				g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
				SendErrorFrame(pReciveFrame->pBuffer[0],LOOP_ERROR);
                return;
			}
            #endif
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
            #if(CAP3_STATE)
            {
                if(g_SystemState.heFenState3 != CHECK_3_HE_STATE)
                {
                    SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                    g_SuddenState.RefuseAction = JIGOU3_HE_ERROR;  //发生拒动
                }
                else
                {
                    g_SuddenState.RefuseAction = FALSE;  //未发生拒动
                }
            }
            #endif  
            break;
        }
        case CHECK_3_FEN_ORDER:
        {
            #if(CAP3_STATE)
            {
                if(g_SystemState.heFenState3 != CHECK_3_FEN_STATE)
                {
                    SendErrorFrame(g_RemoteControlState.orderId , REFUSE_ERROR);
                    g_SuddenState.RefuseAction = JIGOU3_FEN_ERROR;  //发生拒动
                }
                else
                {
                    g_SuddenState.RefuseAction = FALSE;  //未发生拒动
                }
            }
            #endif  
            break;
        }
        default:
        {
            break;
        }
    }
    if(g_SuddenState.RefuseAction != FALSE)
    {
        g_TimeStampCollect.changeLedTime.delayTime = 100;  //发生拒动错误后，指示灯闪烁间隔变短
    }
    else
    {
        g_TimeStampCollect.changeLedTime.delayTime = 500;  //正常状态下指示灯闪烁的间隔
    }
    g_SuddenState.SuddenFlag = TRUE;  //发送突发错误
}

/**
 * 
 * <p>Function name: [CheckOrder]</p>
 * <p>Discription: [检测命令是否正确执行]</p>
 */
void GetValue(void)
{
    uint8_t idIndex = g_qSendFrame.pBuffer[1];
    uint8_t error = 0;
    uint8_t i = 0;
    struct DefFrameData pSendFrame;   //要发送的数据
    uint8_t data[8] = {0};
    pSendFrame.pBuffer = data;
    pSendFrame.ID = g_qSendFrame.ID;
    pSendFrame.complteFlag = 0xFF;
    
    if(g_RemoteControlState.GetAllValueFalg == TRUE)
    {
        pSendFrame.pBuffer[0] = 0x92;
        for(idIndex = g_startID ;idIndex <= g_overID;idIndex++)    //抛除ID号所占的长度
        {
            ClrWdt();
            g_pPoint.len = 8;
            error = ReadParamValue(idIndex,&g_pPoint);
            if((error == 0xF1)||(error == 0xF3))    //数据长度错误
            {
                ClrWdt();
                SendErrorFrame(pSendFrame.pBuffer[0],DATA_LEN_ERROR);
            }
            pSendFrame.pBuffer[1] = idIndex;  //配置号  
            pSendFrame.len = g_pPoint.len + 2;
            for(i = 0;i < pSendFrame.len;i++)
            {
                pSendFrame.pBuffer[i + 2] = g_pPoint.pData[i];
            }
            ClrWdt();
            SendData(&pSendFrame);
        }
    }
    if(g_RemoteControlState.GetOneValueFalg == TRUE)
    { 
//        for(i = 1;i < pReciveFrame->len;i++)    //抛除ID号所占的长度
//        {
//            ClrWdt();
//            g_pPoint.len = 8;
//            error = ReadParamValue(g_qSendFrame.pBuffer[i],&g_pPoint);
//            if((error == 0xF1)||(error == 0xF3))    //数据长度错误
//            {
//                ClrWdt();
//                SendErrorFrame(g_qSendFrame.pBuffer[0],DATA_LEN_ERROR);
//            }
//            if((error == 0xF2)||(error == 0xF4))    //ID号错误
//            {
//                ClrWdt();
//                SendErrorFrame(g_qSendFrame.pBuffer[0],ID_ERROR);
//            }
//            g_pSendFrame->pBuffer[1] = g_qSendFrame.pBuffer[i];  //配置号
//            g_pSendFrame->pBuffer[2] = g_pPoint.pData[0];
//            g_pSendFrame->pBuffer[3] = g_pPoint.pData[1];
//            g_pSendFrame->len = g_pPoint.len + 2;
//            ClrWdt();
//            SendData(&g_pSendFrame);
//        }
    }
    g_RemoteControlState.GetAllValueFalg = FALSE;
    g_RemoteControlState.GetOneValueFalg = FALSE;
}
