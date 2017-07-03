/** 
 * <p>application name： yongci.c</p> 
 * <p>application describing： 永磁控制器分合闸主流程</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期2017-05-20] [张东旭][更改分合闸计时方式，不再采用中断计时，增加三个机构的控制]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */

#include "../Header.h"
#include "yongci.h"


#define REFUSE_ACTION   800     //拒动错误检测间隔时间（ms）

frameRtu sendFrame, recvFrame;

SwitchConfig g_SwitchConfig[4];	//配置机构状态
IndexConfig g_Index[4]; //获取同步合闸偏移时间以及合闸顺序

//uint32_t _PERSISTENT g_TimeStampCollect.changeLedTime.delayTime;   //改变LED灯闪烁时间 (ms)
uint16_t _PERSISTENT g_lockUp;  //命令上锁，在执行了一次合分闸命令之后应处于上锁状态，在延时800ms之后才可以第二次执行
uint16_t _PERSISTENT g_Order;    //需要执行的命令，且在单片机发生复位后不会改变
uint16_t g_lastRunOrder = IDLE_ORDER;
CAN_msg ReciveMsg;


void InitSetSwitchState(void);
void UpdateCount(void);
void SwitchOpenFirstPhase(SwitchConfig* pConfig);
void SwitchOpenSecondPhase(SwitchConfig* pConfig);
void SwitchOpenThirdPhase(SwitchConfig* pConfig);
void SwitchCloseFirstPhase(SwitchConfig* pConfig);
void SwitchCloseSecondPhase(SwitchConfig* pConfig);
void SwitchCloseThirdPhase(SwitchConfig* pConfig);
void GetSwitchCloseTime(IndexConfig* pIndex);

//
/**
 * 
 * <p>Function name: [_T3Interrupt]</p>
 * <p>Discription: [定时器3的中断函数]</p>
 */
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;
     ClrWdt();
    uint8_t index = g_SynActionAttribute.currentIndex;
   
    uint8_t loop =  g_SynActionAttribute.Attribute[index].loop - 1;
    if((g_SwitchConfig[loop].currentState == REDAY_STATE) &&
            (g_SwitchConfig[loop].order == IDLE_ORDER))
	{
		g_SwitchConfig[loop].currentState = RUN_STATE;
        g_SwitchConfig[loop].order = HE_ORDER;
		g_SwitchConfig[loop].systemTime = g_TimeStampCollect.msTicks;
        g_SwitchConfig[loop].SwitchClose(g_SwitchConfig + loop);        
      
        index = ++ g_SynActionAttribute.currentIndex;
        if(index < g_SynActionAttribute.count)
        {
            ChangePr3(g_SynActionAttribute.Attribute[index].offsetTime);//偏移时间          
        } 
        else
        {
              ResetTimer3();
        }		
	}
    
}

/**
 * 
 * <p>Function name: [OnLock]</p>
 * <p>Discription: [上锁]</p>
 */
inline void OnLock(void)
{
    g_lockUp = ON_LOCK;    //上锁
    g_lockUp = ON_LOCK;    //上锁
}

/**
 * 
 * <p>Function name: [OffLock]</p>
 * <p>Discription: [解锁状态]</p>
 */
inline void OffLock(void)
{
    g_lockUp = OFF_LOCK;    //解锁
    g_lockUp = OFF_LOCK;    //解锁
}



/**
 * 同步合闸执行
 */
void SynCloseAction(void)
{
    uint8_t i = 0;
    uint8_t count = 0;
    
    #if CAP3_STATE
        count = 3;
    #else
        count = 2;
    #endif

    for(i = 0; i < count; i++)
    {
        //不允许多次执行相同/不同的操作
        if(g_SwitchConfig[i].currentState || g_SwitchConfig[i].order || g_SwitchConfig[i].lastOrder)
        {
            g_Order = IDLE_ORDER;    //将命令清零
            i = 10;
            return;
        }
    }
    if(i == 10) //TODO:为什么？
    {
        return;
    }
    
    OFF_COMMUNICATION_INT();  //关闭通信中断
	uint8_t loop = 0;
    g_SynActionAttribute.currentIndex = 0;
    for(uint8_t i = 1; i < g_SynActionAttribute.count; i++)
    {
        loop =  g_SynActionAttribute.Attribute[i].loop - 1;
        g_SwitchConfig[loop].currentState = REDAY_STATE;
        g_SwitchConfig[loop].order = IDLE_ORDER;
        g_SwitchConfig[loop].powerOnTime =  50;//TODO:固定值
        g_SwitchConfig[loop].offestTime =    g_SynActionAttribute.Attribute[i].offsetTime;
    }
    loop =  g_SynActionAttribute.Attribute[0].loop -1;
	g_SwitchConfig[loop].currentState = RUN_STATE;
    g_SwitchConfig[loop].order = HE_ORDER;
	g_SwitchConfig[loop].powerOnTime = 50;//TODO:固定值
	g_SwitchConfig[loop].systemTime = g_TimeStampCollect.msTicks;
	g_SwitchConfig[loop].SwitchClose(g_SwitchConfig + loop);
    
    //判断执行的路数
      uint8_t nextIndex = ++ g_SynActionAttribute.currentIndex;
    if(nextIndex < g_SynActionAttribute.count)
    {     
       StartTimer3(g_SynActionAttribute.Attribute[nextIndex].offsetTime);
    }
    ClrWdt();
}
/**
 * 
 * <p>Function name: [HEZHA_Action]</p>
 * <p>Discription: [对机构执行合闸操作，对外提供接口，方便引用]</p>
 * @param index 执行机构的索引号
 * @param time IGBT导通时间
 */
void HEZHA_Action(uint8_t index,uint16_t time)
{
    if(g_lockUp == OFF_LOCK)    //解锁状态下不能进行合闸
    {
        return;
    }
    //不允许多次执行相同/不同的操作
    if(g_SwitchConfig[index].currentState || g_SwitchConfig[index].order || g_SwitchConfig[index].lastOrder)
    {
        g_Order = IDLE_ORDER;    //将命令清零
        return ;
    }
    OFF_COMMUNICATION_INT();  //关闭通信中断
    ClrWdt();
    g_SwitchConfig[index].currentState = RUN_STATE;
    g_SwitchConfig[index].order = HE_ORDER;
    g_SwitchConfig[index].systemTime = g_TimeStampCollect.msTicks;
    g_SwitchConfig[index].powerOnTime = time + 3;   //使用示波器发现时间少3ms左右
    ClrWdt();
    g_SwitchConfig[index].SwitchClose(g_SwitchConfig + index);
    g_SwitchConfig[index].systemTime = g_TimeStampCollect.msTicks;
}

/**
 * 
 * <p>Function name: [HEZHA_Action]</p>
 * <p>Discription: [对机构执行合闸操作，对外提供接口，方便引用]</p>
 * @param index 执行机构的索引号
 * @param time IGBT导通时间
 */
void FENZHA_Action(uint8_t index,uint16_t time)
{
    if(g_lockUp == OFF_LOCK)    //解锁状态下不能进行分闸
    {
        return;
    }
    //不允许多次执行相同/不同的操作
    if(g_SwitchConfig[index].currentState || g_SwitchConfig[index].order || g_SwitchConfig[index].lastOrder)
    {
        g_Order = IDLE_ORDER;    //将命令清零
        return ;
    }
    OFF_COMMUNICATION_INT();  //关闭通信中断
    ClrWdt();
    g_SwitchConfig[index].currentState = RUN_STATE;
    g_SwitchConfig[index].order = FEN_ORDER;
    g_SwitchConfig[index].systemTime = g_TimeStampCollect.msTicks;
    g_SwitchConfig[index].powerOffTime = time + 2;   //使用示波器发现分闸时间少2ms左右
    ClrWdt();
    g_SwitchConfig[index].SwitchOpen(g_SwitchConfig + index);
    g_SwitchConfig[index].systemTime = g_TimeStampCollect.msTicks;
}

/**
 * 
 * <p>Function name: [ RefreshActionState]</p>
 * <p>Discription: [刷新合分闸工作状态]</p>
 * <p>return :[非0 -- 有正在工作的合分闸动作状态]
 *            0--空闲状态
 *    </p>
 */
uint8_t  RefreshActionState()
{
    uint8_t state = 0;
     ClrWdt();
      //机构1合闸、分闸刷新
        if((g_SwitchConfig[DEVICE_I].order == HE_ORDER) && (g_SwitchConfig[DEVICE_I].currentState == RUN_STATE))
        {
            ClrWdt();
            OFF_COMMUNICATION_INT();  //刷新关闭通信中断
            g_SwitchConfig[DEVICE_I].SwitchClose(g_SwitchConfig);
            state |= 0x81;
        }
        else if((g_SwitchConfig[DEVICE_I].order == FEN_ORDER) && (g_SwitchConfig[DEVICE_I].currentState == RUN_STATE))
        {
            ClrWdt();
            OFF_COMMUNICATION_INT();  //刷新关闭通信中断
            g_SwitchConfig[DEVICE_I].SwitchOpen(g_SwitchConfig);
            state |= 0x82;
        }
        else    //防止持续合闸或者分闸
        {
//            __delay_us(100);
//            g_SwitchConfig[DEVICE_I].order = IDLE_ORDER;
//            RESET_CURRENT_A();            
        }
        
        //机构2合闸、分闸刷新
        if((g_SwitchConfig[DEVICE_II].order == HE_ORDER) && (g_SwitchConfig[DEVICE_II].currentState == RUN_STATE))
        {
            ClrWdt();
            OFF_COMMUNICATION_INT();  //刷新关闭通信中断
            g_SwitchConfig[DEVICE_II].SwitchClose(g_SwitchConfig + 1);
            state |= 0x84;
        }
        else if((g_SwitchConfig[DEVICE_II].order == FEN_ORDER) && (g_SwitchConfig[DEVICE_II].currentState == RUN_STATE))
        {
            ClrWdt();
            OFF_COMMUNICATION_INT();  //刷新关闭通信中断
            g_SwitchConfig[DEVICE_II].SwitchOpen(g_SwitchConfig + 1);
             state |= 0x88;
        }
        else    //防止持续合闸或者分闸
        {
//            __delay_us(100);
//            g_SwitchConfig[DEVICE_II].order = IDLE_ORDER;
//            RESET_CURRENT_B();
        }
        
        #if(CAP3_STATE)
        {
            //机构3合闸、分闸刷新
            if((g_SwitchConfig[DEVICE_III].order == HE_ORDER) && (g_SwitchConfig[DEVICE_III].currentState == RUN_STATE))
            {
                ClrWdt();
                OFF_COMMUNICATION_INT();  //刷新关闭通信中断
                g_SwitchConfig[DEVICE_III].SwitchClose(g_SwitchConfig + 2);
                state |= 0x10;
            }
            else if((g_SwitchConfig[DEVICE_III].order == FEN_ORDER) && (g_SwitchConfig[DEVICE_III].currentState == RUN_STATE))
            {
                ClrWdt();
                g_SwitchConfig[DEVICE_III].SwitchOpen(g_SwitchConfig + 2);
                OFF_COMMUNICATION_INT();  //刷新关闭通信中断
                 state |= 0x20;
            }
            else    //防止持续合闸或者分闸
            {
//                __delay_us(100);
//                g_SwitchConfig[DEVICE_III].order = IDLE_ORDER;
//                RESET_CURRENT_C();
            }
        }
        #endif

        return state;
}
/**
 * 
 * <p>Function name: [RefreshIdleState]</p>
 * <p>Discription: [刷新空闲的工作状态]</p>
 * <p>return : 非0--跳出
 *              0--正常执行
 *    </p>
 */
uint8_t  RefreshIdleState()
{
        static uint8_t runLedState = TURN_ON;
        static uint8_t runLedCount = 0;
        static uint32_t checkOrderTime = UINT32_MAX;
        static uint32_t checkOrderDelay = UINT32_MAX;
        uint8_t result = 0;
        
       //任意一项不是空闲状态
        if((!(g_SwitchConfig[DEVICE_I].order == IDLE_ORDER) &&
             (g_SwitchConfig[DEVICE_II].order == IDLE_ORDER) && 
              CHECK_ORDER3()))
        {
            return 0xF1;        
        }     
        
        
        //空闲状态，状态刷新        
        ON_COMMUNICATION_INT();
        ResetTimer3();  //刷新关闭定时器3
        ClrWdt();
        if((g_SwitchConfig[DEVICE_I].lastOrder != IDLE_ORDER) || 
            (g_SwitchConfig[DEVICE_II].lastOrder != IDLE_ORDER) || 
            CHECK_LAST_ORDER3())
        {
            ON_COMMUNICATION_INT();
            ClrWdt();
            checkOrderDelay = REFUSE_ACTION;
            //此处开启计时功能，延时大约在800ms内判断是否正确执行功能，不是的话返回错误
            UpdateCount();//更新计数
            ReadCapDropVoltage(g_lastRunOrder);  //读取电容跌落电压
            checkOrderTime = g_TimeStampCollect.msTicks;
            //Clear Flag
            g_SwitchConfig[DEVICE_I].order = IDLE_ORDER; //清零
            g_SwitchConfig[DEVICE_II].order = IDLE_ORDER;
            g_SwitchConfig[DEVICE_III].order = IDLE_ORDER;
            g_SwitchConfig[DEVICE_I].currentState = IDLE_ORDER;
            g_SwitchConfig[DEVICE_II].currentState = IDLE_ORDER;
            g_SwitchConfig[DEVICE_III].currentState = IDLE_ORDER;
            g_SwitchConfig[DEVICE_I].lastOrder = IDLE_ORDER;
            g_SwitchConfig[DEVICE_II].lastOrder = IDLE_ORDER;
            g_SwitchConfig[DEVICE_III].lastOrder = IDLE_ORDER;
        }

        //拒动错误检测
        if(g_TimeStampCollect.msTicks - checkOrderTime >= checkOrderDelay)//TODO：错误是
        {
            CheckOrder(g_lastRunOrder);  //检测命令是否执行
            checkOrderDelay = UINT32_MAX;   //设置时间为最大值，防止其启动检测
            checkOrderTime = UINT32_MAX;    //设置当前时间为最大的计数时间
            g_lastRunOrder = IDLE_ORDER;     //上一次执行的命令清空
            g_RemoteControlState.lastReceiveOrder = IDLE_ORDER; //情况上一次执行的命令
            OffLock();  //解锁
        }
        ClrWdt();

        
        //检测是否欠电压， 并更新显示
        if(IsOverTimeStamp( &g_TimeStampCollect.getCapVolueTime)) //大约每300ms获取一次电容电压值
        {
            CheckVoltage();  
            ClrWdt();
            g_TimeStampCollect.getCapVolueTime.startTime = g_TimeStampCollect.msTicks;           
        }  

        if(IsOverTimeStamp( &g_TimeStampCollect.scanTime)) //大约每2ms扫描一次
        {
            SwitchScan();   //执行按键扫描程序 TODO:用时时长
            ClrWdt();
            g_TimeStampCollect.scanTime.startTime = g_TimeStampCollect.msTicks;  
        }
        if (CheckIOState()) //收到合分闸指令，退出后立即进行循环
        {
            g_lastRunOrder = g_Order;
            g_Order = IDLE_ORDER;    //将命令清零
            return 0xff;
        }

        //始终进行处理，处理完缓冲区。 TODO:远方本地检测?
        do
        {
            result = BufferDequeue(&ReciveMsg);
            if (result)
            {
                DeviceNetReciveCenter(&ReciveMsg.id, ReciveMsg.data, ReciveMsg.len);
            }
        }
         while(result);
         
        
        //周期性状态更新
        if((IsOverTimeStamp( &g_TimeStampCollect.sendDataTime)) || (g_SuddenState.SuddenFlag == TRUE))
        {
            ClrWdt();
            DsplaySwitchState();    //更新机构的状态显示
            //建立连接且不是在预制状态下才会周期上传
            if((StatusChangedConnedctionObj.state == STATE_LINKED) && (g_RemoteControlState.ReceiveStateFlag == IDLE_ORDER))   
            {
                UpdataState();  //更新状态
            }
            g_SuddenState.SuddenFlag = FALSE;  //Clear
            //OverflowDetection(SEND_TIME);   //增加溢出判断  TODO:所有事件判断，都将出问题
            g_TimeStampCollect.sendDataTime.startTime = g_TimeStampCollect.msTicks;  
        }

        ClrWdt();
        

        //运行指示灯
        if(IsOverTimeStamp( &g_TimeStampCollect.changeLedTime))
        {
            UpdateLEDIndicateState(RUN_LED, runLedState);
            runLedState = ~runLedState;
            if(g_RemoteControlState.CanErrorFlag == TRUE)
            {
                runLedCount++;
                if(runLedCount >= 10) //1500*4ms
                {
                    InitStandardCAN(0, 0);  //初始化CAN模块
                    g_TimeStampCollect.changeLedTime.delayTime = 500;  //运行指示灯闪烁间隔为500ms
                }
                //TODO:什么时候恢复？
            }
            g_TimeStampCollect.changeLedTime.startTime = g_TimeStampCollect.msTicks;  
        }

         ClrWdt();
        //获取温度数据
        if(IsOverTimeStamp( &g_TimeStampCollect.getTempTime))
        {                   
            g_SystemVoltageParameter.temp = DS18B20GetTemperature();    //获取温度值            
            g_TimeStampCollect.getTempTime.startTime = g_TimeStampCollect.msTicks;  
        }

        //超时检测复位
        if((g_RemoteControlState.overTimeFlag == TRUE) && 
           (g_RemoteControlState.ReceiveStateFlag != IDLE_ORDER) && 
           (g_RemoteControlState.orderId <= 4) && (g_RemoteControlState.orderId > 0))  //判断是否需要超时检测
        {
            if(!CheckIsOverTime())
            {
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER; //Clear order
                g_RemoteControlState.overTimeFlag = FALSE;  //Clear Flag
                SendErrorFrame(g_RemoteControlState.orderId , OVER_TIME_ERROR);
                g_RemoteControlState.orderId = 0;   //Clear
                g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;  //Clear
                OffLock();  //解锁
            }
            else
            {
                g_RemoteControlState.overTimeFlag = TRUE;  //Updata
            }
        }
        if(g_RemoteControlState.SetFixedValue == TRUE)
        {
            WriteAccumulateSum();  //写入累加和
            g_RemoteControlState.SetFixedValue = FALSE;
        }
        if(g_RemoteControlState.GetAllValueFalg == TRUE)
        {
            GetValue(); //获取监控数据

        }

        return 0;
        
        
}
/**************************************************
 *函数名：YongciMainTask()
 *功能: 永磁控制主循环 
 *形参：  void
 *返回值：void
****************************************************/
void YongciMainTask(void)
{
    uint8_t result = 0;
 
    while(0xFFFF) //主循环
    {        
        result = RefreshActionState();
        //有正在执行的合分闸动作，则继续刷新状态。
        if (result)
        {
            continue;
        }
        //均没有处于运行状态：TODO:添加预制状态处理，防止遗漏时间错开的情况
        RESET_CURRENT_A();
        RESET_CURRENT_B();
        RESET_CURRENT_C();
        result =   RefreshIdleState();      
        //检测到按钮动作
       if (result)
        {
            continue;
        }
        
        
        
        //同步合闸预制是单独执行的
        while((g_RemoteControlState.ReceiveStateFlag == TONGBU_HEZHA) &&
                (g_RemoteControlState.orderId == SyncReadyClose) && 
              (g_RemoteControlState.overTimeFlag == TRUE))
        {
            if(!CheckIsOverTime())
            {
                ON_COMMUNICATION_INT();
                TurnOffInt2();
                g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER; //Clear order
                g_RemoteControlState.overTimeFlag = FALSE;  //Clear Flag
                SendErrorFrame(g_RemoteControlState.orderId , OVER_TIME_ERROR);
                g_RemoteControlState.orderId = 0;   //Clear
                g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;  //Clear
                OffLock();  //解锁
            }
            else
            {
                g_RemoteControlState.overTimeFlag = TRUE;  //Updata
            }
        }
    }
}

/**************************************************
 *函数名：YongciFirstInit()
 *功能:初始化IGBT控制端口，合分闸状态检测端口
 *形参：  void
 *返回值：void
****************************************************/
void YongciFirstInit(void)
{
    uint8_t index = 0;
    uint8_t data[8] = {0,0,0,0,0,0,0,0};
    ClrWdt();
    
    g_pPoint.pData = data;
    g_pPoint.len = 8;
    
    g_lockUp = OFF_LOCK;    //处于解锁状态
    
    //IGBT引脚
    //A
    DRIVER_A_1_DIR = 0;
    DRIVER_B_1_DIR = 0;
    DRIVER_C_1_DIR = 0;
    DRIVER_D_1_DIR = 0;
    RESET_CURRENT_A();
    ClrWdt();
    
    //IGBT引脚
    //B
    DRIVER_A_2_DIR = 0;
    DRIVER_B_2_DIR = 0;
    DRIVER_C_2_DIR = 0;
    DRIVER_D_2_DIR = 0;
    RESET_CURRENT_B();
    ClrWdt();
    
    //IGBT引脚
    //B
    DRIVER_A_3_DIR = 0;
    DRIVER_B_3_DIR = 0;
    DRIVER_C_3_DIR = 0;
    DRIVER_D_3_DIR = 0;
    RESET_CURRENT_C();
    ClrWdt();
    
    g_Order = IDLE_ORDER;   //初始化
    //默认状态下的同步合闸顺序
    index = 0;
    g_Index[index].indexLoop = 0;
    g_Index[index].GetTime = GetSwitchCloseTime;
    g_Index[index].GetTime(g_Index + index);
    index++;
    
    g_Index[index].indexLoop = 1;
    g_Index[index].GetTime = GetSwitchCloseTime;
    g_Index[index].GetTime(g_Index + index);
    index++;

    g_Index[index].indexLoop = 2;
    g_Index[index].GetTime = GetSwitchCloseTime;
    g_Index[index].GetTime(g_Index + index);
    index++;
    
    ClrWdt();
    InitSetSwitchState();
    
    g_TimeStampCollect.getTempTime.startTime = g_TimeStampCollect.msTicks;  
    g_TimeStampCollect.sendDataTime.startTime = g_TimeStampCollect.msTicks;   
    g_TimeStampCollect.scanTime.startTime = g_TimeStampCollect.msTicks;   
    g_TimeStampCollect.changeLedTime.startTime = g_TimeStampCollect.msTicks;    
    g_TimeStampCollect.getCapVolueTime.startTime = g_TimeStampCollect.msTicks;  
    
    
    g_TimeStampCollect.changeLedTime.delayTime = 500;    //初始化值为500ms
    
    //远方控制标识位初始化
    g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
    g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;
    g_RemoteControlState.overTimeFlag = FALSE;
    g_RemoteControlState.orderId = 0x00;    //Clear
    g_RemoteControlState.SetFixedValue = FALSE;    //Clear    
    g_RemoteControlState.GetAllValueFalg = FALSE;    //Clear
    g_RemoteControlState.GetOneValueFalg = FALSE;    //Clear
    g_RemoteControlState.CanErrorFlag = FALSE;    //Clear
    OffLock();  //解锁，可以检测
    
    //****************************************
    //突发状态量更新初始化
    g_SuddenState.CapState1 = 0;
    g_SuddenState.CapState2 = 0;
    g_SuddenState.CapState3 = 0;
    g_SuddenState.ExecuteOrder1 = 0;
    g_SuddenState.ExecuteOrder2 = 0;
    g_SuddenState.ExecuteOrder3 = 0;
    ClrWdt();
    g_SuddenState.SuddenFlag = TRUE;
    g_SuddenState.SwitchState1 = 0;
    g_SuddenState.SwitchState2 = 0;
    g_SuddenState.SwitchState3 = 0;
    g_SuddenState.RefuseAction = FALSE;
    //****************************************
    
    //数据帧初始化
    g_qSendFrame.pBuffer = data;
    g_qSendFrame.complteFlag = 0xFF;
    g_qSendFrame.ID = MAKE_GROUP1_ID(GROUP1_POLL_STATUS_CYCLER_ACK, DeviceNetObj.MACID);
    
    g_lastRunOrder = IDLE_ORDER;
}  

/**
 * 
 * <p>Function name: [InitSetSwitchState]</p>
 * <p>Discription: [机构状态的初始化]</p>
 */
void InitSetSwitchState(void)
{
	g_SwitchConfig[DEVICE_I].currentState = IDLE_ORDER;	//默认为空闲状态
	g_SwitchConfig[DEVICE_I].order = IDLE_ORDER; //默认未执行
    g_SwitchConfig[DEVICE_I].lastOrder = IDLE_ORDER; //默认上一次未执行任何指令
	g_SwitchConfig[DEVICE_I].powerOnTime = HEZHA_TIME;  //默认合闸时间50ms
	g_SwitchConfig[DEVICE_I].powerOffTime = FENZHA_TIME;    //默认合闸时间50ms
	g_SwitchConfig[DEVICE_I].offestTime = 0; //默认无偏移时间
	g_SwitchConfig[DEVICE_I].systemTime = 0;    //默认系统时间为零
	g_SwitchConfig[DEVICE_I].SwitchClose = SwitchCloseFirstPhase;
	g_SwitchConfig[DEVICE_I].SwitchOpen = SwitchOpenFirstPhase;
    ClrWdt();

	g_SwitchConfig[DEVICE_II].currentState = IDLE_ORDER;	//默认为空闲状态
	g_SwitchConfig[DEVICE_II].order = IDLE_ORDER; //默认未执行
    g_SwitchConfig[DEVICE_II].lastOrder = IDLE_ORDER; //默认上一次未执行任何指令
	g_SwitchConfig[DEVICE_II].powerOnTime = HEZHA_TIME;  //默认合闸时间50ms
	g_SwitchConfig[DEVICE_II].powerOffTime = FENZHA_TIME;    //默认分闸时间30ms
	g_SwitchConfig[DEVICE_II].offestTime = 0; //默认无偏移时间
	g_SwitchConfig[DEVICE_II].systemTime = 0;    //默认系统时间为零
	g_SwitchConfig[DEVICE_II].SwitchClose = SwitchCloseSecondPhase;
	g_SwitchConfig[DEVICE_II].SwitchOpen = SwitchOpenSecondPhase;
    ClrWdt();

	g_SwitchConfig[DEVICE_III].currentState = IDLE_ORDER;	//默认为空闲状态
	g_SwitchConfig[DEVICE_III].order = IDLE_ORDER; //默认未执行
    g_SwitchConfig[DEVICE_III].lastOrder = IDLE_ORDER; //默认上一次未执行任何指令
	g_SwitchConfig[DEVICE_III].powerOnTime = HEZHA_TIME;  //默认合闸时间50ms
	g_SwitchConfig[DEVICE_III].powerOffTime = FENZHA_TIME;    //默认分闸时间30ms
	g_SwitchConfig[DEVICE_III].offestTime = 0; //默认无偏移时间
	g_SwitchConfig[DEVICE_III].systemTime = 0;    //默认系统时间为零
	g_SwitchConfig[DEVICE_III].SwitchClose = SwitchCloseThirdPhase;
	g_SwitchConfig[DEVICE_III].SwitchOpen = SwitchOpenThirdPhase;

    ClrWdt();
    
}


//各个机构的合闸函数
//*************************************

/**
 * 
 * <p>Function name: [SwitchCloseFirstPhase]</p>
 * <p>Discription: [第一相合闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchCloseFirstPhase(SwitchConfig* pConfig)
{
	if((pConfig->order == HE_ORDER) && (pConfig->currentState == RUN_STATE))	//首先判断是否是合闸命令,且是否是可以执行状态
	{
		HEZHA_A();
		HEZHA_A();
	}
	if(IsOverTime(pConfig->systemTime, pConfig->powerOnTime))	//超时时间到则复位
	{
		RESET_CURRENT_A();
		RESET_CURRENT_A();
		pConfig->order = IDLE_ORDER;
		pConfig->currentState = IDLE_ORDER;
        ClrWdt();
        pConfig->lastOrder = HE_ORDER; //刚执行完合闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchCloseSecondPhase]</p>
 * <p>Discription: [第二相合闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchCloseSecondPhase(SwitchConfig* pConfig)
{
	if((pConfig->order == HE_ORDER) && (pConfig->currentState == RUN_STATE))	//首先判断是否是合闸命令,且是否是可以执行状态
	{
		HEZHA_B();
		HEZHA_B();
	}
	if(IsOverTime(pConfig->systemTime, pConfig->powerOnTime))	//超时时间到则复位
	{
		RESET_CURRENT_B();
		RESET_CURRENT_B();
		pConfig->order = IDLE_ORDER;
		pConfig->currentState = IDLE_ORDER;
        ClrWdt();
        pConfig->lastOrder = HE_ORDER; //刚执行完合闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchCloseThirdPhase]</p>
 * <p>Discription: [第三相合闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchCloseThirdPhase(SwitchConfig* pConfig)
{
	if((pConfig->order == HE_ORDER) && (pConfig->currentState == RUN_STATE))	//首先判断是否是合闸命令,且是否是可以执行状态
	{
		HEZHA_C();
		HEZHA_C();
	}
	if(IsOverTime(pConfig->systemTime,pConfig->powerOnTime))	//超时时间到则复位
	{
		RESET_CURRENT_C();
		RESET_CURRENT_C();
		pConfig->order = IDLE_ORDER;
		pConfig->currentState = IDLE_ORDER;
        ClrWdt();
        pConfig->lastOrder = HE_ORDER; //刚执行完合闸指令
	}
}


//*************************************

//各个机构的分闸函数
//*************************************
/**
 * 
 * <p>Function name: [SwitchOpenFirstPhase]</p>
 * <p>Discription: [第一相分闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOpenFirstPhase(SwitchConfig* pConfig)
{
	if((pConfig->order == FEN_ORDER) && (pConfig->currentState == RUN_STATE))	//首先判断是否是分闸命令
	{
		FENZHA_A();
		FENZHA_A();
	}
	if(IsOverTime(pConfig->systemTime,pConfig->powerOffTime))	//超时时间到则复位
	{
		RESET_CURRENT_A();
		RESET_CURRENT_A();
		pConfig->order = IDLE_ORDER;
		pConfig->currentState = IDLE_ORDER;
        ClrWdt();
        pConfig->lastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchOpenSecondPhase]</p>
 * <p>Discription: [第二相分闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOpenSecondPhase(SwitchConfig* pConfig)
{
	if((pConfig->order == FEN_ORDER) && (pConfig->currentState == RUN_STATE))	//首先判断是否是分闸命令
	{
		FENZHA_B();
		FENZHA_B();
	}
	if(IsOverTime(pConfig->systemTime, pConfig->powerOffTime))	//超时时间到则复位
	{
		RESET_CURRENT_B();
		RESET_CURRENT_B();
		pConfig->order = IDLE_ORDER;
		pConfig->currentState = IDLE_ORDER;
        ClrWdt();
        pConfig->lastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchOpenThirdPhase]</p>
 * <p>Discription: [第三相分闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOpenThirdPhase(SwitchConfig* pConfig)
{
    if((pConfig->order == FEN_ORDER) && (pConfig->currentState == RUN_STATE))	//首先判断是否是分闸命令
	{
		FENZHA_C();
		FENZHA_C();
	}
	if(IsOverTime(pConfig->systemTime,pConfig->powerOffTime))	//超时时间到则复位
	{
		RESET_CURRENT_C();
		RESET_CURRENT_C();
		pConfig->order = IDLE_ORDER;
		pConfig->currentState = IDLE_ORDER;
        ClrWdt();
        pConfig->lastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

//*************************************

/**
 * 
 * <p>Function name: [GetSwitchCloseTime]</p>
 * <p>Discription: [获取机构合闸时间]</p>
 * @param pIndex 
 */
void GetSwitchCloseTime(IndexConfig* pIndex)
{
    switch(pIndex->indexLoop)
    {
        case 0:
            pIndex->onTime = g_DelayTime.hezhaTime1 + 3;
            break;
        case 1:
            pIndex->onTime = g_DelayTime.hezhaTime2 + 3;
            break;
        case 2:
            pIndex->onTime = g_DelayTime.hezhaTime3 + 3;
            break;
        default:
            pIndex->onTime = HEZHA_TIME + 3;
            break;
    }
}

/**
 * 
 * <p>Function name: [GetOffestTime]</p>
 * <p>Discription: [获取机构同步合闸的顺序以及偏移时间]</p>
 * @param pReciveFrame  接收到的数据指针
 * @param pSendFrame    要发送的数据指针
 * @return 0则回路数错误，0xFF正常
 */
uint8_t GetOffestTime(struct DefFrameData* pReciveFrame)
{
    uint16_t highTime = 0;
    uint16_t lowTime = 0;

	uint8_t len = pReciveFrame->len;

	switch (len)
	{
		case 4:	//有两个回路需要同步控制
		case 6:	//有三个回路需要同步控制
        {
			g_Index[0].indexLoop = pReciveFrame->pBuffer[1] & 0x03;
			g_Index[1].indexLoop = (pReciveFrame->pBuffer[1] & 0x0C) >> 2;
            g_Index[0].indexLoop -= 1;
            g_Index[1].indexLoop -= 1;

            ClrWdt();
            lowTime = pReciveFrame->pBuffer[2];
            highTime = pReciveFrame->pBuffer[3];
            g_Index[1].offestTime = (highTime << 8) | lowTime;
            g_Index[1].offestTime = g_Index[1].offestTime / 2;  //单片机计时时间为2us
            
            ClrWdt();
			g_Index[0].GetTime(g_Index);
			g_Index[1].GetTime(g_Index + 1);
            
            #if(CAP3_STATE)
            {
                g_Index[2].indexLoop = (pReciveFrame->pBuffer[1] & 0x30) >> 4;
                g_Index[2].indexLoop -= 1;
                ClrWdt();
                lowTime = pReciveFrame->pBuffer[4];
                highTime = pReciveFrame->pBuffer[5];
                g_Index[2].offestTime = (highTime << 8) | lowTime;
                g_Index[2].offestTime = g_Index[2].offestTime / 2;  //单片机计时时间为2us
                
                ClrWdt();
                g_Index[2].GetTime(g_Index + 2);
            }
            #endif
            return 0xFF;
        }
        default:
        {
            ClrWdt();
            return 0;
        }
	}
}

/**************************************************
 *函数名： UpdateCount()
 *功能: 根据合闸与分闸更新EEPROM存在的计数
 *形参：  uint16 state--合闸或分闸命令
 *返回值：void
****************************************************/
void UpdateCount(void)
{
    //应禁止中断
    if (g_SwitchConfig[DEVICE_I].lastOrder == HE_ORDER) //机构1合闸
    {
        ClrWdt();
        WriteFenzhaCount(JG1_HE_COUNT_ADDRESS , &g_ActionCount.hezhaCount1);
    }
    else if (g_SwitchConfig[DEVICE_I].lastOrder == FEN_ORDER)  //机构1分闸
    {
        ClrWdt();
        WriteFenzhaCount(JG1_FEN_COUNT_ADDRESS , &g_ActionCount.fenzhaCount1);
    }
    
    if (g_SwitchConfig[DEVICE_II].lastOrder == HE_ORDER)   //机构2合闸
    {
        ClrWdt();
        WriteFenzhaCount(JG2_HE_COUNT_ADDRESS , &g_ActionCount.hezhaCount2);
    }
    else if (g_SwitchConfig[DEVICE_II].lastOrder == FEN_ORDER)  //机构2分闸
    {
        ClrWdt();
        WriteFenzhaCount(JG2_FEN_COUNT_ADDRESS , &g_ActionCount.fenzhaCount2);
    }
    
    #if(CAP3_STATE)
    {
        if (g_SwitchConfig[DEVICE_III].lastOrder == HE_ORDER)   //机构3合闸
        {
            ClrWdt();
            WriteFenzhaCount(JG3_HE_COUNT_ADDRESS , &g_ActionCount.hezhaCount3);
        }
        else if (g_SwitchConfig[DEVICE_III].lastOrder == FEN_ORDER)  //机构3分闸
        {
            ClrWdt();
            WriteFenzhaCount(JG3_FEN_COUNT_ADDRESS , &g_ActionCount.fenzhaCount3);
        }
    }
    #endif
    g_SwitchConfig[DEVICE_I].lastOrder = IDLE_ORDER;    //清零防止重复写入
    g_SwitchConfig[DEVICE_II].lastOrder = IDLE_ORDER;    //清零防止重复写入
    g_SwitchConfig[DEVICE_III].lastOrder = IDLE_ORDER;    //清零防止重复写入
    ClrWdt();
    
}



