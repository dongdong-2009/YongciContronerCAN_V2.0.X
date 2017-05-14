/****************************
 * 
 * 2015/10/13:
 * 关于继电器状态说明：
 * 1.储能电压达到限定值后，储能继电器线圈得电，常开闭合。否则失电断开。
 * 2.检测到故障后，储能继电器线圈得电，常开闭合。未检测到问题时，断开。
 * 
 * 
 * 2015/6/18:更改中断同步方式，改软件等待与硬件触发相结合的方式
 * 改地址为0xA2
 * **************************/


#include "../Header.h"
#include "yongci.h"



extern  frameRtu sendFrame, recvFrame;
extern uint8 _PERSISTENT g_Order;  //需要执行的命令,在单片机发生复位的情况下该值依然可以保存

SwitchConfig g_SetSwitchState[4];	//配置机构状态


void InitSetSwitchState(void);
void UpdateCount(void);
void SwitchOffFirstPhase(SwitchConfig* pConfig);
void SwitchOffSecondPhase(SwitchConfig* pConfig);
void SwitchOffThirdPhase(SwitchConfig* pConfig);
void SwitchOnFirstPhase(SwitchConfig* pConfig);
void SwitchOnSecondPhase(SwitchConfig* pConfig);
void SwitchOnThirdPhase(SwitchConfig* pConfig);


void __attribute__((interrupt, no_auto_psv)) _T3Interrupt(void)
{
    IFS0bits.T3IF = 0;
	if((g_SetSwitchState[1].State == REDAY_STATE) && (g_SetSwitchState[1].Order == IDLE_ORDER))
	{
		g_SetSwitchState[1].State = RUN_STATE;
        g_SetSwitchState[1].Order = HE_ORDER;
		g_SetSwitchState[1].SysTime = g_MsTicks;
        g_SetSwitchState[1].SwitchOn(g_SetSwitchState + 1);
        ResetTimer3();
        if(CAP3_STATE)
        {
            StartTimer3(g_SetSwitchState[2].OffestTime);//偏移时间
        }
		
	}
    else if((g_SetSwitchState[2].State == REDAY_STATE) && (g_SetSwitchState[2].Order == IDLE_ORDER))
	{
		g_SetSwitchState[2].State = RUN_STATE;
        g_SetSwitchState[2].Order = HE_ORDER;
		g_SetSwitchState[2].SysTime = g_MsTicks;
        g_SetSwitchState[2].SwitchOn(g_SetSwitchState + 2);
        ResetTimer3();
	}
    
}

/**
 * 
 * <p>Function name: [TongBuHeZha]</p>
 * <p>Discription: [对机构执行同步合闸操作]</p>
 */
void TongBuHeZha(uint16 offsetTime1,uint16 offsetTime2)
{
	g_SetSwitchState[1].State = REDAY_STATE;
    g_SetSwitchState[1].Order = IDLE_ORDER;
	g_SetSwitchState[1].SwitchOnTime = HEZHA_TIME + 3;
	g_SetSwitchState[1].OffestTime = offsetTime1;

    if(CAP3_STATE)
    {
        g_SetSwitchState[2].State = REDAY_STATE;
        g_SetSwitchState[2].Order = IDLE_ORDER;
        g_SetSwitchState[2].SwitchOnTime = HEZHA_TIME + 3;
        g_SetSwitchState[2].OffestTime = offsetTime2;
    }

	g_SetSwitchState[0].State = RUN_STATE;
    g_SetSwitchState[0].Order = HE_ORDER;
	g_SetSwitchState[0].SwitchOnTime = HEZHA_TIME + 3;
	g_SetSwitchState[0].SysTime = g_MsTicks;
	g_SetSwitchState[0].SwitchOn(g_SetSwitchState);

    StartTimer3(g_SetSwitchState[1].OffestTime);    //120us,实际使用示波器为144us
}
/**
 * 
 * <p>Function name: [HEZHA_Action]</p>
 * <p>Discription: [对机构执行合闸操作，对外提供接口，方便引用]</p>
 * @param index 执行机构的索引号
 * @param time IGBT导通时间
 */
void HEZHA_Action(uint8 index,uint16 time)
{
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
void FENZHA_Action(uint8 index,uint16 time)
{
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
//    uint8 result = 0;
    while(0xFFFF) //主循环
    {
        //机构1合闸、分闸刷新
        if((g_SetSwitchState[0].Order == HE_ORDER) && (g_SetSwitchState[0].State == RUN_STATE))
        {
            g_SetSwitchState[0].SwitchOn(g_SetSwitchState);
        }
        else if((g_SetSwitchState[0].Order == FEN_ORDER) && (g_SetSwitchState[0].State == RUN_STATE))
        {
            g_SetSwitchState[0].SwitchOff(g_SetSwitchState);
        }
        
        //机构2合闸、分闸刷新
        if((g_SetSwitchState[1].Order == HE_ORDER) && (g_SetSwitchState[1].State == RUN_STATE))
        {
            g_SetSwitchState[1].SwitchOn(g_SetSwitchState + 1);
        }
        else if((g_SetSwitchState[1].Order == FEN_ORDER) && (g_SetSwitchState[1].State == RUN_STATE))
        {
            g_SetSwitchState[1].SwitchOff(g_SetSwitchState + 1);
        }
        
        if(CAP3_STATE)
        {
            //机构3合闸、分闸刷新
            if((g_SetSwitchState[2].Order == HE_ORDER) && (g_SetSwitchState[2].State == RUN_STATE))
            {
                g_SetSwitchState[2].SwitchOn(g_SetSwitchState + 2);
            }
            else if((g_SetSwitchState[2].Order == FEN_ORDER) && (g_SetSwitchState[2].State == RUN_STATE))
            {
                g_SetSwitchState[2].SwitchOff(g_SetSwitchState + 2);
            }
        }
        
        //机构空闲状态刷新
        if((g_SetSwitchState[0].Order == IDLE_ORDER) && (g_SetSwitchState[1].Order == IDLE_ORDER) && 
           (g_SetSwitchState[2].Order == IDLE_ORDER))
        {
            if((g_SetSwitchState[0].State == IDLE_ORDER) && (g_SetSwitchState[1].State == IDLE_ORDER) && 
               (g_SetSwitchState[2].State == IDLE_ORDER))
            {
                RESET_CURRENT_A();
                RESET_CURRENT_B();
                RESET_CURRENT_C();
                RESET_CURRENT_A();
                RESET_CURRENT_B();
                RESET_CURRENT_C();
            }
            UpdateCount();//更新计数
            //注意更新计数函数的位置，防止在执行远方操作时将通信中断全部关闭
            
            g_SetSwitchState[0].Order = IDLE_ORDER; //清零
            g_SetSwitchState[1].Order = IDLE_ORDER;
            g_SetSwitchState[2].Order = IDLE_ORDER;
            g_SetSwitchState[0].State = IDLE_ORDER;
            g_SetSwitchState[1].State = IDLE_ORDER;
            g_SetSwitchState[2].State = IDLE_ORDER;
            
            //判断远方与就地的延时消抖
            if ((g_SystemState.yuanBenState == YUAN_STATE) && (g_SystemState.workMode == WORK_STATE))//远控
            {
                ON_INT();   
                //远方控制时允许通信
                //暂时不要485通信，所以需要屏蔽
//                /***************************************************************************
//                if(g_SystemState.yuanBenState == YUAN_STATE)
//                {
//                    ON_INT();//此时开启中断以方便接收
//                    //空闲状态下，处理通讯程序
//                    result = ReciveBufferDataDealing(&sendFrame, &recvFrame);//返回剩余长度
//                    ClrWdt();
//                    if (recvFrame.completeFlag == TRUE)
//                    {
//                        ExecuteFunctioncode(&recvFrame);
//                    }
//                    ClrWdt();
//                    if (result) //若缓存中还有数据先处理帧数据。
//                    {
//                        continue;
//                    }
//                }
//                ***************************************************************************/
            }
            else if (g_SystemState.yuanBenState == BEN_STATE)
            {
                OFF_INT();  //就地控制时不允许通信
            }
            
            if(g_ScanTime >= 1) //大约每2ms扫描一次
            {
                SwitchScan();   //执行按键扫描程序
                g_ScanTime = 0;
            }
            if (CheckIOState()) //收到合分闸指令，退出后立即进行循环
            {
                g_Order = IDLE_ORDER;    //将命令清零
                continue;
            }
            ClrWdt(); 
            //检测是否欠电压， 并更新显示
            CheckVoltage();
            //更新机构的状态显示
            DsplaySwitchState();
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
    ClrWdt();
    
    //IGBT引脚
    //A
    DRIVER_A_1_DIR = 0;
    DRIVER_B_1_DIR = 0;
    DRIVER_C_1_DIR = 0;
    DRIVER_D_1_DIR = 0;
    RESET_CURRENT_A();
    
    //IGBT引脚
    //B
    DRIVER_A_2_DIR = 0;
    DRIVER_B_2_DIR = 0;
    DRIVER_C_2_DIR = 0;
    DRIVER_D_2_DIR = 0;
    RESET_CURRENT_B();
    
    //IGBT引脚
    //B
    DRIVER_A_3_DIR = 0;
    DRIVER_B_3_DIR = 0;
    DRIVER_C_3_DIR = 0;
    DRIVER_D_3_DIR = 0;
    RESET_CURRENT_C();
    
    g_Order = IDLE_ORDER;   //初始化
    ReceiveStateFlag = IDLE_ORDER;  //初始化通信所需的命令状态
    
    InitSetSwitchState();
}  

/**
 * 
 * <p>Function name: [InitSetSwitchState]</p>
 * <p>Discription: [机构状态的初始化]</p>
 */
void InitSetSwitchState(void)
{
	uint8 index = 0;

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
    
}


//各个机构的合闸函数
//*************************************
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
        pConfig->LastOrder = HE_ORDER; //刚执行完合闸指令
	}
}

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
        pConfig->LastOrder = HE_ORDER; //刚执行完合闸指令
	}
}


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
        pConfig->LastOrder = HE_ORDER; //刚执行完合闸指令
	}
}


//*************************************

//各个机构的分闸函数
//*************************************
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
        pConfig->LastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

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
        pConfig->LastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}


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
        pConfig->LastOrder = FEN_ORDER; //刚执行完分闸指令
	}
}

//*************************************




/**************************************************
 *函数名： UpdateCount()
 *功能: 根据合闸与分闸更新EEPROM存在的计数
 *形参：  uint16 state--合闸或分闸命令
 *返回值：void
****************************************************/
void UpdateCount(void)
{
    //应禁止中断
    _prog_addressT addr; 
    OFF_INT();  //关闭通信中断，防止在写入EEPROM时被打断
    if (g_SetSwitchState[0].LastOrder == HE_ORDER) //机构1合闸
    {
        addr = (_prog_addressT)JG1_HE_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
    }
    else if (g_SetSwitchState[0].LastOrder == FEN_ORDER)  //机构1分闸
    {
        addr = (_prog_addressT)JG1_FEN_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
    }
    
    if (g_SetSwitchState[1].LastOrder == HE_ORDER)   //机构2合闸
    {
        addr = (_prog_addressT)JG2_HE_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
    }
    else if (g_SetSwitchState[1].LastOrder == FEN_ORDER)  //机构2分闸
    {
        addr = (_prog_addressT)JG2_FEN_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
    }
    
    if(CAP3_STATE)
    {
        if (g_SetSwitchState[2].LastOrder == HE_ORDER)   //机构3合闸
        {
            addr = (_prog_addressT)JG3_HE_COUNT_ADDRESS;
            WriteFenzhaCount(addr);
        }
        else if (g_SetSwitchState[2].LastOrder == FEN_ORDER)  //机构3分闸
        {
            addr = (_prog_addressT)JG3_FEN_COUNT_ADDRESS;
            WriteFenzhaCount(addr);
        }
    }
    g_SetSwitchState[0].LastOrder = IDLE_ORDER;    //清零防止重复写入
    g_SetSwitchState[1].LastOrder = IDLE_ORDER;    //清零防止重复写入
    g_SetSwitchState[2].LastOrder = IDLE_ORDER;    //清零防止重复写入
    
}