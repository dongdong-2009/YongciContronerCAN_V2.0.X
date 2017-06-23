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

#define SEND_TIME   2000        //发送在线状态间隔时间 (ms)
#define SCAN_TIME   2           //按键扫描间隔时间
#define GET_TEMP_TIME   10000   //获取温度数据时间   (ms)

frameRtu sendFrame, recvFrame;
uint8_t _PERSISTENT g_Order;    //需要执行的命令，且在单片机发生复位后不会改变

SwitchConfig g_SetSwitchState[4];	//配置机构状态
IndexConfig g_Index[4]; //获取同步合闸偏移时间以及合闸顺序

uint32_t _PERSISTENT g_changeLedTime; //改变LED灯闪烁时间 (ms)
uint16_t _PERSISTENT g_lockUp;   //命令上锁，在执行了一次合分闸命令之后应处于上锁状态，在延时800ms之后才可以第二次执行

void InitSetSwitchState(void);
void UpdateCount(void);
void SwitchOffFirstPhase(SwitchConfig* pConfig);
void SwitchOffSecondPhase(SwitchConfig* pConfig);
void SwitchOffThirdPhase(SwitchConfig* pConfig);
void SwitchOnFirstPhase(SwitchConfig* pConfig);
void SwitchOnSecondPhase(SwitchConfig* pConfig);
void SwitchOnThirdPhase(SwitchConfig* pConfig);
void GetSwitchOnTime(IndexConfig* pIndex);


/**
 * 
 * <p>Function name: [_T1Interrupt]</p>
 * <p>Discription: [定时器1的中断函数]</p>
 */
//void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
//{
//    
//}

/**
 * 
 * <p>Function name: [_T3Interrupt]</p>
 * <p>Discription: [定时器3的中断函数]</p>
 */
void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;
	if((g_SetSwitchState[g_Index[1].indexLoop].State == REDAY_STATE) && (g_SetSwitchState[g_Index[1].indexLoop].Order == IDLE_ORDER))
	{
		g_SetSwitchState[g_Index[1].indexLoop].State = RUN_STATE;
        g_SetSwitchState[g_Index[1].indexLoop].Order = HE_ORDER;
		g_SetSwitchState[g_Index[1].indexLoop].SysTime = g_MsTicks;
        g_SetSwitchState[g_Index[1].indexLoop].SwitchOn(g_SetSwitchState + g_Index[1].indexLoop);
        ResetTimer3();
        if(CAP3_STATE)
        {
            StartTimer3(g_SetSwitchState[g_Index[2].indexLoop].OffestTime);//偏移时间
        }
		
	}
    else if((g_SetSwitchState[g_Index[2].indexLoop].State == REDAY_STATE) && (g_SetSwitchState[g_Index[1].indexLoop].Order == IDLE_ORDER))
	{
		g_SetSwitchState[g_Index[2].indexLoop].State = RUN_STATE;
        g_SetSwitchState[g_Index[2].indexLoop].Order = HE_ORDER;
		g_SetSwitchState[g_Index[2].indexLoop].SysTime = g_MsTicks;
        g_SetSwitchState[g_Index[2].indexLoop].SwitchOn(g_SetSwitchState + g_Index[2].indexLoop);
        ResetTimer3();
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
 * 
 * <p>Function name: [TongBuHeZha]</p>
 * <p>Discription: [对机构执行同步合闸操作]</p>
 */
void TongBuHeZha(void)
{
    OFF_INT();
	g_SetSwitchState[g_Index[1].indexLoop].State = REDAY_STATE;
    g_SetSwitchState[g_Index[1].indexLoop].Order = IDLE_ORDER;
	g_SetSwitchState[g_Index[1].indexLoop].SwitchOnTime = g_Index[1].onTime;
	g_SetSwitchState[g_Index[1].indexLoop].OffestTime = g_Index[1].offestTime;
    ClrWdt();
    
    if(CAP3_STATE)
    {
        g_SetSwitchState[g_Index[2].indexLoop].State = REDAY_STATE;
        g_SetSwitchState[g_Index[2].indexLoop].Order = IDLE_ORDER;
        g_SetSwitchState[g_Index[2].indexLoop].SwitchOnTime = g_Index[2].onTime;
        g_SetSwitchState[g_Index[2].indexLoop].OffestTime = g_Index[2].offestTime;
    }

	g_SetSwitchState[g_Index[0].indexLoop].State = RUN_STATE;
    g_SetSwitchState[g_Index[0].indexLoop].Order = HE_ORDER;
	g_SetSwitchState[g_Index[0].indexLoop].SwitchOnTime = g_Index[0].onTime;;
	g_SetSwitchState[g_Index[0].indexLoop].SysTime = g_MsTicks;
	g_SetSwitchState[g_Index[0].indexLoop].SwitchOn(g_SetSwitchState + g_Index[0].indexLoop);
    
    StartTimer3(g_SetSwitchState[g_Index[1].indexLoop].OffestTime);
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
    OFF_INT();
    ClrWdt();
    g_SetSwitchState[index].State = RUN_STATE;
    g_SetSwitchState[index].Order = HE_ORDER;
    g_SetSwitchState[index].SysTime = g_MsTicks;
    g_SetSwitchState[index].SwitchOnTime = time + 3;   //使用示波器发现时间少3ms左右
    ClrWdt();
    g_SetSwitchState[index].SwitchOn(g_SetSwitchState + index);
    g_SetSwitchState[index].SysTime = g_MsTicks;
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
    OFF_INT();
    ClrWdt();
    g_SetSwitchState[index].State = RUN_STATE;
    g_SetSwitchState[index].Order = FEN_ORDER;
    g_SetSwitchState[index].SysTime = g_MsTicks;
    g_SetSwitchState[index].SwitchOffTime = time + 2;   //使用示波器发现分闸时间少2ms左右
    ClrWdt();
    g_SetSwitchState[index].SwitchOff(g_SetSwitchState + index);
    g_SetSwitchState[index].SysTime = g_MsTicks;
}

/**************************************************
 *函数名：YongciMainTask()
 *功能: 永磁控制主循环 
 *形参：  void
 *返回值：void
****************************************************/
void YongciMainTask(void)
{
    uint8_t state = TURN_ON;
    uint8_t cn = 0;
    uint8_t lastOrder = IDLE_ORDER;
    uint32_t checkOrderTime = g_MsTicks;
    uint32_t checkOrderDelay = UINT32_MAX;
    while(0xFFFF) //主循环
    {
        ClrWdt();
        //机构1合闸、分闸刷新
        if((g_SetSwitchState[0].Order == HE_ORDER) && (g_SetSwitchState[0].State == RUN_STATE))
        {
            ClrWdt();
            OFF_INT();
            g_SetSwitchState[0].SwitchOn(g_SetSwitchState);
        }
        else if((g_SetSwitchState[0].Order == FEN_ORDER) && (g_SetSwitchState[0].State == RUN_STATE))
        {
            ClrWdt();
            OFF_INT();
            g_SetSwitchState[0].SwitchOff(g_SetSwitchState);
        }
        else    //防止持续合闸或者分闸
        {
            RESET_CURRENT_A();
        }
        
        //机构2合闸、分闸刷新
        if((g_SetSwitchState[1].Order == HE_ORDER) && (g_SetSwitchState[1].State == RUN_STATE))
        {
            ClrWdt();
            OFF_INT();
            g_SetSwitchState[1].SwitchOn(g_SetSwitchState + 1);
        }
        else if((g_SetSwitchState[1].Order == FEN_ORDER) && (g_SetSwitchState[1].State == RUN_STATE))
        {
            ClrWdt();
            OFF_INT();
            g_SetSwitchState[1].SwitchOff(g_SetSwitchState + 1);
        }
        else    //防止持续合闸或者分闸
        {
            RESET_CURRENT_B();
        }
        
        if(CAP3_STATE)
        {
            //机构3合闸、分闸刷新
            if((g_SetSwitchState[2].Order == HE_ORDER) && (g_SetSwitchState[2].State == RUN_STATE))
            {
                ClrWdt();
                OFF_INT();
                g_SetSwitchState[2].SwitchOn(g_SetSwitchState + 2);
            }
            else if((g_SetSwitchState[2].Order == FEN_ORDER) && (g_SetSwitchState[2].State == RUN_STATE))
            {
                ClrWdt();
                OFF_INT();
                g_SetSwitchState[2].SwitchOff(g_SetSwitchState + 2);
            }
            else    //防止持续合闸或者分闸
            {
                RESET_CURRENT_C();
            }
        }
        
        //机构空闲状态刷新
        if((g_SetSwitchState[0].Order == IDLE_ORDER) && (g_SetSwitchState[1].Order == IDLE_ORDER) && 
           CHECK_ORDER3())
        {
            ClrWdt();
            if((g_SetSwitchState[0].LastOrder != IDLE_ORDER) || 
                (g_SetSwitchState[1].LastOrder != IDLE_ORDER) || 
                CHECK_LAST_ORDER3())
            {
                ON_INT();
                checkOrderDelay = 500;
                //此处开启计时功能，延时大约在800ms内判断是否正确执行功能，不是的话返回错误
                UpdateCount();//更新计数
                ReadCapDropVoltage(lastOrder);  //读取电容跌落电压
                checkOrderTime = g_SysTimeStamp.TickTime;
            }
            
            //拒动错误检测
            if(g_SysTimeStamp.TickTime - checkOrderTime >= checkOrderDelay)
            {
                CheckOrder(lastOrder);  //检测命令是否执行
                checkOrderDelay = UINT32_MAX;   //设置时间为最大值，防止其启动检测
                checkOrderTime = UINT32_MAX;    //设置当前时间为最大的计数时间
                lastOrder = IDLE_ORDER;     //上一次执行的命令清空
                g_RemoteControlState.lastReceiveOrder = IDLE_ORDER; //情况上一次执行的命令
                OffLock();  //解锁
            }
            ClrWdt();
            
            RESET_CURRENT_A();
            RESET_CURRENT_B();
            RESET_CURRENT_C();
            ClrWdt(); 
            RESET_CURRENT_A();
            RESET_CURRENT_B();
            RESET_CURRENT_C();

            //注意更新计数函数的位置，防止在执行远方操作时将通信中断全部关闭
            
            g_SetSwitchState[0].Order = IDLE_ORDER; //清零
            g_SetSwitchState[1].Order = IDLE_ORDER;
            g_SetSwitchState[2].Order = IDLE_ORDER;
            g_SetSwitchState[0].State = IDLE_ORDER;
            g_SetSwitchState[1].State = IDLE_ORDER;
            g_SetSwitchState[2].State = IDLE_ORDER;
            
               
            //判断远方与就地的延时消抖
                ClrWdt();   
                //远方控制时允许通信
                //暂时不要485通信，所以需要屏蔽
//                /***************************************************************************
//                //空闲状态下，处理通讯程序
//                result = ReciveBufferDataDealing(&sendFrame, &recvFrame);//返回剩余长度
//                ClrWdt();
//                if (recvFrame.completeFlag == TRUE)
//                {
//                    ExecuteFunctioncode(&recvFrame);
//                }
//                ClrWdt();
//                if (result) //若缓存中还有数据先处理帧数据。
//                {
//                    continue;
//                }
//                ***************************************************************************/
            
            //检测是否欠电压， 并更新显示
            CheckVoltage();       
            
            if(g_SysTimeStamp.TickTime - g_SysTimeStamp.ScanTime >= SCAN_TIME) //大约每2ms扫描一次
            {
                SwitchScan();   //执行按键扫描程序
                ClrWdt();
                g_SysTimeStamp.ScanTime = g_SysTimeStamp.TickTime;
            }
            if (CheckIOState()) //收到合分闸指令，退出后立即进行循环
            {
                lastOrder = g_Order;
                g_Order = IDLE_ORDER;    //将命令清零
                continue;
            }
            
            if((g_SysTimeStamp.TickTime - g_SysTimeStamp.SendDataTime >= SEND_TIME) || (g_SuddenState.SuddenFlag == TRUE))
            {
                ClrWdt();
                UpdataState();  //更新状态                
                DsplaySwitchState();    //更新机构的状态显示
                g_SuddenState.SuddenFlag = FALSE;  //Clear
                OverflowDetection(SEND_TIME);   //增加溢出判断
                g_SysTimeStamp.SendDataTime = g_SysTimeStamp.TickTime;
            }
            ClrWdt();
            //运行指示灯
            if(g_SysTimeStamp.TickTime - g_SysTimeStamp.ChangeLedTime >= g_changeLedTime)
            {
                UpdateLEDIndicateState(RUN_LED,state);
                state = ~state;
                if(g_changeLedTime == 1500)   //CAN总线关闭错误处理
                {
                    cn++;
                    if(cn >= 10) //1500*4ms
                    {
                        InitStandardCAN(0, 0);      //初始化CAN模块
                        g_changeLedTime = 500;   //运行指示灯闪烁间隔为500ms
                    }
                }
                g_SysTimeStamp.ChangeLedTime = g_SysTimeStamp.TickTime;
            }
            
            //获取温度数据
            if(g_SysTimeStamp.TickTime - g_SysTimeStamp.GetTempTime >= GET_TEMP_TIME)
            {
                g_SysTimeStamp.GetTempTime = g_SysTimeStamp.TickTime;
                ClrWdt();
                g_SystemVoltageParameter.temp = DS18B20GetTemperature();    //获取温度值
            }
            
            //超时检测复位
            if((g_RemoteControlState.overTimeFlage == TRUE) && 
               (g_RemoteControlState.ReceiveStateFlag != IDLE_ORDER) && 
               (g_RemoteControlState.orderId <= 5) && (g_RemoteControlState.orderId > 0))  //判断是否需要超时检测
            {
                if(!CheckIsOverTime())
                {
                    if((g_RemoteControlState.ReceiveStateFlag == TONGBU_HEZHA) && 
                       (g_RemoteControlState.orderId == 0x05))   //判断是否是同步合闸命令
                    {
                        ON_INT();
                        TurnOffInt2();
                        g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
                    }
                    g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER; //Clear Order
                    g_RemoteControlState.overTimeFlage = FALSE;  //Clear Flag
                    SendErrorFrame(g_RemoteControlState.orderId , OVER_TIME_ERROR);
                    g_RemoteControlState.orderId = 0;   //Clear
                    OffLock();  //解锁
                }
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
    ClrWdt();
    
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
//****************************************
//突发状态量更新
    g_SuddenState.CapState1 = 0;
    g_SuddenState.CapState2 = 0;
    g_SuddenState.CapState3 = 0;
    g_SuddenState.ExecuteOrder1 = 0;
    g_SuddenState.ExecuteOrder2 = 0;
    g_SuddenState.ExecuteOrder3 = 0;
    g_SuddenState.SuddenFlag = TRUE;
    g_SuddenState.SwitchState1 = 0;
    g_SuddenState.SwitchState2 = 0;
    g_SuddenState.SwitchState3 = 0;
    ClrWdt();
//****************************************
    
    //默认状态下的同步合闸顺序
    index = 0;
    g_Index[index].indexLoop = 0;
    g_Index[index].GetTime = GetSwitchOnTime;
    g_Index[index].GetTime(g_Index + index);
    index++;
    
    g_Index[index].indexLoop = 1;
    g_Index[index].GetTime = GetSwitchOnTime;
    g_Index[index].GetTime(g_Index + index);
    index++;

    g_Index[index].indexLoop = 2;
    g_Index[index].GetTime = GetSwitchOnTime;
    g_Index[index].GetTime(g_Index + index);
    index++;
    
    ClrWdt();
    InitSetSwitchState();
    
    g_SysTimeStamp.ChangeLedTime = g_MsTicks;   //对于时间状态量的初始化
    g_SysTimeStamp.GetTempTime = g_MsTicks;     //对于时间状态量的初始化
    g_SysTimeStamp.ScanTime = g_MsTicks;        //对于时间状态量的初始化
    g_SysTimeStamp.SendDataTime = g_MsTicks;    //对于时间状态量的初始化
    
    g_changeLedTime = 500;    //初始化值为500ms
    
    //远方控制标识位初始化
    g_RemoteControlState.ReceiveStateFlag = IDLE_ORDER;
    g_RemoteControlState.lastReceiveOrder = IDLE_ORDER;
    g_RemoteControlState.overTimeFlage = FALSE;
    
    OffLock();  //解锁，可以检测
}  

/**
 * 
 * <p>Function name: [InitSetSwitchState]</p>
 * <p>Discription: [机构状态的初始化]</p>
 */
void InitSetSwitchState(void)
{
	uint8_t index = 0;

	g_SetSwitchState[index].State = IDLE_ORDER;	//默认为空闲状态
	g_SetSwitchState[index].Order = IDLE_ORDER; //默认未执行
    g_SetSwitchState[index].LastOrder = IDLE_ORDER; //默认上一次未执行任何指令
	g_SetSwitchState[index].SwitchOnTime = HEZHA_TIME;  //默认合闸时间50ms
	g_SetSwitchState[index].SwitchOffTime = FENZHA_TIME;    //默认合闸时间50ms
	g_SetSwitchState[index].OffestTime = 0; //默认无偏移时间
	g_SetSwitchState[index].SysTime = 0;    //默认系统时间为零
	g_SetSwitchState[index].SwitchOn = SwitchOnFirstPhase;
	g_SetSwitchState[index].SwitchOff = SwitchOffFirstPhase;
	index++;
    ClrWdt();

	g_SetSwitchState[index].State = IDLE_ORDER;	//默认为空闲状态
	g_SetSwitchState[index].Order = IDLE_ORDER; //默认未执行
    g_SetSwitchState[index].LastOrder = IDLE_ORDER; //默认上一次未执行任何指令
	g_SetSwitchState[index].SwitchOnTime = HEZHA_TIME;  //默认合闸时间50ms
	g_SetSwitchState[index].SwitchOffTime = FENZHA_TIME;    //默认分闸时间30ms
	g_SetSwitchState[index].OffestTime = 0; //默认无偏移时间
	g_SetSwitchState[index].SysTime = 0;    //默认系统时间为零
	g_SetSwitchState[index].SwitchOn = SwitchOnSecondPhase;
	g_SetSwitchState[index].SwitchOff = SwitchOffSecondPhase;
	index++;
    ClrWdt();

	g_SetSwitchState[index].State = IDLE_ORDER;	//默认为空闲状态
	g_SetSwitchState[index].Order = IDLE_ORDER; //默认未执行
    g_SetSwitchState[index].LastOrder = IDLE_ORDER; //默认上一次未执行任何指令
	g_SetSwitchState[index].SwitchOnTime = HEZHA_TIME;  //默认合闸时间50ms
	g_SetSwitchState[index].SwitchOffTime = FENZHA_TIME;    //默认分闸时间30ms
	g_SetSwitchState[index].OffestTime = 0; //默认无偏移时间
	g_SetSwitchState[index].SysTime = 0;    //默认系统时间为零
	g_SetSwitchState[index].SwitchOn = SwitchOnThirdPhase;
	g_SetSwitchState[index].SwitchOff = SwitchOffThirdPhase;
	index++;
    ClrWdt();
    
}


//各个机构的合闸函数
//*************************************

/**
 * 
 * <p>Function name: [SwitchOnFirstPhase]</p>
 * <p>Discription: [第一相合闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOnFirstPhase(SwitchConfig* pConfig)
{
	if((pConfig->Order == HE_ORDER) && (pConfig->State == RUN_STATE))	//首先判断是否是合闸命令,且是否是可以执行状态
	{
		HEZHA_A();
		HEZHA_A();
	}
	if(IsOverTime(pConfig->SysTime,pConfig->SwitchOnTime))	//超时时间到则复位
	{
		RESET_CURRENT_A();
		RESET_CURRENT_A();
		pConfig->Order = IDLE_ORDER;
		pConfig->State = IDLE_ORDER;
        ClrWdt();
        pConfig->LastOrder = HE_ORDER; //刚执行完合闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchOnSecondPhase]</p>
 * <p>Discription: [第二相合闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOnSecondPhase(SwitchConfig* pConfig)
{
	if((pConfig->Order == HE_ORDER) && (pConfig->State == RUN_STATE))	//首先判断是否是合闸命令,且是否是可以执行状态
	{
		HEZHA_B();
		HEZHA_B();
	}
	if(IsOverTime(pConfig->SysTime,pConfig->SwitchOnTime))	//超时时间到则复位
	{
		RESET_CURRENT_B();
		RESET_CURRENT_B();
		pConfig->Order = IDLE_ORDER;
		pConfig->State = IDLE_ORDER;
        ClrWdt();
        pConfig->LastOrder = HE_ORDER; //刚执行完合闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchOnThirdPhase]</p>
 * <p>Discription: [第三相合闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOnThirdPhase(SwitchConfig* pConfig)
{
	if((pConfig->Order == HE_ORDER) && (pConfig->State == RUN_STATE))	//首先判断是否是合闸命令,且是否是可以执行状态
	{
		HEZHA_C();
		HEZHA_C();
	}
	if(IsOverTime(pConfig->SysTime,pConfig->SwitchOnTime))	//超时时间到则复位
	{
		RESET_CURRENT_C();
		RESET_CURRENT_C();
		pConfig->Order = IDLE_ORDER;
		pConfig->State = IDLE_ORDER;
        ClrWdt();
        pConfig->LastOrder = HE_ORDER; //刚执行完合闸指令
	}
}


//*************************************

//各个机构的分闸函数
//*************************************
/**
 * 
 * <p>Function name: [SwitchOffFirstPhase]</p>
 * <p>Discription: [第一相分闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOffFirstPhase(SwitchConfig* pConfig)
{
	if((pConfig->Order == FEN_ORDER) && (pConfig->State == RUN_STATE))	//首先判断是否是分闸命令
	{
		FENZHA_A();
		FENZHA_A();
	}
	if(IsOverTime(pConfig->SysTime,pConfig->SwitchOffTime))	//超时时间到则复位
	{
		RESET_CURRENT_A();
		RESET_CURRENT_A();
		pConfig->Order = IDLE_ORDER;
		pConfig->State = IDLE_ORDER;
        ClrWdt();
        pConfig->LastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchOffSecondPhase]</p>
 * <p>Discription: [第二相分闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOffSecondPhase(SwitchConfig* pConfig)
{
	if((pConfig->Order == FEN_ORDER) && (pConfig->State == RUN_STATE))	//首先判断是否是分闸命令
	{
		FENZHA_B();
		FENZHA_B();
	}
	if(IsOverTime(pConfig->SysTime,pConfig->SwitchOffTime))	//超时时间到则复位
	{
		RESET_CURRENT_B();
		RESET_CURRENT_B();
		pConfig->Order = IDLE_ORDER;
		pConfig->State = IDLE_ORDER;
        ClrWdt();
        pConfig->LastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

/**
 * 
 * <p>Function name: [SwitchOffThirdPhase]</p>
 * <p>Discription: [第三相分闸]</p>
 * @param pConfig 执行该函数功能的指针
 */
void SwitchOffThirdPhase(SwitchConfig* pConfig)
{
    if((pConfig->Order == FEN_ORDER) && (pConfig->State == RUN_STATE))	//首先判断是否是分闸命令
	{
		FENZHA_C();
		FENZHA_C();
	}
	if(IsOverTime(pConfig->SysTime,pConfig->SwitchOffTime))	//超时时间到则复位
	{
		RESET_CURRENT_C();
		RESET_CURRENT_C();
		pConfig->Order = IDLE_ORDER;
		pConfig->State = IDLE_ORDER;
        ClrWdt();
        pConfig->LastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

//*************************************

/**
 * 
 * <p>Function name: [GetSwitchOnTime]</p>
 * <p>Discription: [获取机构合闸时间]</p>
 * @param pIndex 
 */
void GetSwitchOnTime(IndexConfig* pIndex)
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
 */
void GetOffestTime(struct DefFrameData* pReciveFrame , struct DefFrameData* pSendFrame)
{
    uint16_t highTime = 0;
    uint16_t lowTime = 0;

	uint8_t len = pSendFrame->len;

	switch (len)
	{
		case 5:	//有两个回路需要同步控制
		case 8:	//有三个回路需要同步控制
        {
			g_Index[0].indexLoop = pSendFrame->pBuffer[1];
			g_Index[1].indexLoop = pSendFrame->pBuffer[2];

            ClrWdt();
            lowTime = pReciveFrame->pBuffer[2];
            highTime = pReciveFrame->pBuffer[3];
            g_Index[1].offestTime = (highTime << 8) | lowTime;
            g_Index[1].offestTime = g_Index[1].offestTime / 2;  //单片机计时时间为2us
            
            ClrWdt();
			g_Index[0].GetTime(g_Index);
			g_Index[1].GetTime(g_Index + 1);
            
            if(CAP3_STATE)
            {
                g_Index[2].indexLoop = pSendFrame->pBuffer[3];
                ClrWdt();
                lowTime = pReciveFrame->pBuffer[4];
                highTime = pReciveFrame->pBuffer[5];
                g_Index[2].offestTime = (highTime << 8) | lowTime;
                g_Index[2].offestTime = g_Index[2].offestTime / 2;  //单片机计时时间为2us
                
                ClrWdt();
                g_Index[2].GetTime(g_Index + 2);
            }
            break;	
        }
        default:
        {
            ClrWdt();
            break;
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
    if (g_SetSwitchState[0].LastOrder == HE_ORDER) //机构1合闸
    {
        ClrWdt();
        WriteFenzhaCount(JG1_HE_COUNT_ADDRESS , &g_ActionCount.hezhaCount1);
    }
    else if (g_SetSwitchState[0].LastOrder == FEN_ORDER)  //机构1分闸
    {
        ClrWdt();
        WriteFenzhaCount(JG1_FEN_COUNT_ADDRESS , &g_ActionCount.fenzhaCount1);
    }
    
    if (g_SetSwitchState[1].LastOrder == HE_ORDER)   //机构2合闸
    {
        ClrWdt();
        WriteFenzhaCount(JG2_HE_COUNT_ADDRESS , &g_ActionCount.hezhaCount2);
    }
    else if (g_SetSwitchState[1].LastOrder == FEN_ORDER)  //机构2分闸
    {
        ClrWdt();
        WriteFenzhaCount(JG2_FEN_COUNT_ADDRESS , &g_ActionCount.fenzhaCount2);
    }
    
    if(CAP3_STATE)
    {
        if (g_SetSwitchState[2].LastOrder == HE_ORDER)   //机构3合闸
        {
            ClrWdt();
            WriteFenzhaCount(JG3_HE_COUNT_ADDRESS , &g_ActionCount.hezhaCount3);
        }
        else if (g_SetSwitchState[2].LastOrder == FEN_ORDER)  //机构3分闸
        {
            ClrWdt();
            WriteFenzhaCount(JG3_FEN_COUNT_ADDRESS , &g_ActionCount.fenzhaCount3);
        }
    }
    g_SetSwitchState[0].LastOrder = IDLE_ORDER;    //清零防止重复写入
    g_SetSwitchState[1].LastOrder = IDLE_ORDER;    //清零防止重复写入
    g_SetSwitchState[2].LastOrder = IDLE_ORDER;    //清零防止重复写入
    ClrWdt();
    
}



