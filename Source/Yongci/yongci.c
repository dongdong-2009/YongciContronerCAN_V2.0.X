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

#define ON_INT() {ON_UART_INT(); ON_CAN_INT();}
#define OFF_INT() {OFF_UART_INT(); OFF_CAN_INT();}


//静态变量--综合编译器与单片机特点，看门狗复位应能保持变量原值
//A路运行指示，表示IGBT正在工作中 状态
static uint16 RunStateFlag;

//对于合闸指令应先上锁，合闸动作完毕后，立即解锁。合闸动作前应先确认指令，再确认是否已经上锁。
static uint16 HeActionLock = OFF_LOCK;
static uint16 FenActionLock = OFF_LOCK;

extern  frameRtu sendFrame, recvFrame;

volatile uint16 HezhaTimeA = HEZHA_TIME_A;
volatile uint16 FenzhaTimeA = FENZHA_TIME_A;

//永磁等待合闸命令
volatile uint16 WaitIntOrder = NULL_ORDER;

uint16 LastOrderState = IDLE_ORDER;//记录最后一次动作命令



void __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void)
{
    ClrWdt();
    IFS0bits.T1IF = 0;
    YongciStopCurrentA();
    
    return;  
}
/**************************************************
 *函数名：SetRunStateFlagA()
 *功能:设置新的状态
 *形参：  void
 *返回值：void
****************************************************/
inline void SetRunStateFlagA(uint16 newFlag)
{
    ClrWdt();
    RunStateFlag = newFlag;
    RunStateFlag = newFlag;
}
/**************************************************
 *函数名：GetRunStateFlagA()
 *功能:设置新的状态
 *形参：  void
 *返回值：void
****************************************************/
inline uint16 GetRunStateFlagA(void)
{
    ClrWdt();
    return RunStateFlag;
}
/**************************************************
 *函数名： HeOnLock()
 *功能:设置合闸锁--允许合闸指令
 *形参：  void
 *返回值：void
****************************************************/
inline void HeOnLock(void)
{
    HeActionLock =  ON_HE_LOCK;
    HeActionLock =  ON_HE_LOCK;
}
/**************************************************
 *函数名： FenOnLock()
 *功能:设置分闸锁--允许分闸指令
 *形参：  void
 *返回值：void
****************************************************/
inline void FenOnLock(void)
{
    FenActionLock =  ON_FEN_LOCK;
    FenActionLock =  ON_FEN_LOCK;
}
/**************************************************
 *函数名： OffLockk()
 *功能:关锁指令，不允许分闸和合闸命令执行
 *形参：  void
 *返回值：void
****************************************************/
inline void OffLock(void)
{
    HeActionLock =  OFF_LOCK;
    FenActionLock = OFF_LOCK;
    HeActionLock =  OFF_LOCK;
    FenActionLock = OFF_LOCK;
}

/**************************************************
 *函数名：YongciStopCurrentA
 *功能:结束IGBT导通状态。结束合闸或者分闸电流通过状态。
 *形参：  void
 *返回值：void
****************************************************/
void YongciStopCurrentA(void)
{
    ClrWdt();
    ResetTimer1(); //复位定时器，为下一轮做准备
    LastOrderState = RunStateFlag;
    RunStateFlag = IDLE_ORDER;
    RunStateFlag = IDLE_ORDER;
    ON_INT();
}
/**************************************************
 *函数名：YongciMainTask()
 *功能: 永磁控制主循环 
 *形参：  void
 *返回值：void
****************************************************/
void YongciMainTask(void)
{
    uint8 result = 0;
    uint32 kairuValue = 0;
    while(0xFFFF) //主循环
    {
        switch (RunStateFlag)
        {
            
            case HE_ORDER: //正在执行合闸指令
            {
                ClrWdt();
//                HEZHA_A(); //始终刷新合闸状态
//                HEZHA_B(); //始终刷新合闸状态
                break;
            }
            case FEN_ORDER: //正在执行分闸指令
            {
                ClrWdt();
//                FENZHA_A();//始终刷新分闸状态
//                FENZHA_B();//始终刷新分闸状态
                break;
            }
            
            case HE_ORDER_A: //正在执行合闸指令
            {
                ClrWdt();
//                HEZHA_A(); //始终刷新合闸状态
                break;
            }
            case FEN_ORDER_A: //正在执行分闸指令
            {
                ClrWdt();
//                FENZHA_A();//始终刷新分闸状态
                break;
            }
            
            case HE_ORDER_B: //正在执行合闸指令
            {
                ClrWdt();
//                HEZHA_B(); //始终刷新合闸状态
                break;
            }
            case FEN_ORDER_B: //正在执行分闸指令
            {
                ClrWdt();
//                FENZHA_B();//始终刷新分闸状态
                break;
            }
            
            case IDLE_ORDER: //空闲状态
            {
                //若此处进入中断，开启分合闸控制就会出现问题。
                ClrWdt();
                RESET_CURRENT_A();//始终刷新空闲状态
                RESET_CURRENT_A();
                RESET_CURRENT_B();
                RESET_CURRENT_B();
                OffLock();//解除上锁状态
                UpdateCount(LastOrderState);//更新计数
                
                kairuValue = ReHC74165();
                if (kairuValue == YUAN_AND_WORK())//远控
                {
                    g_SystemState.yuanBenState = YUAN_STATE;   //远方
                }
                else
                {
                    g_SystemState.yuanBenState = BEN_STATE;   //就地
                }
                //紧接着处理后任务,意味着刚从合分闸动作中跳出.
                if ( IDLE_ORDER != LastOrderState )
                {
                    if(g_SystemState.yuanBenState == BEN_STATE)
                    {
                        result =  ContinuousCheck( &LastOrderState );
                    }
                    ClrWdt();
                    if (CHECK_1_HE_ORDER  ==  result)
                    {
                        FenZhaActionA();
                        continue; //继续下次循环
                    }
                    ClrWdt();
                    //判断完后检测更新状态
                    result = CheckIOState();
                    if (result) //收到合分闸指令，退出后立即进行循环
                    {
                        continue;
                    }
                    LastOrderState = IDLE_ORDER;
                }
                LastOrderState = IDLE_ORDER;
                
                if(g_SystemState.yuanBenState == YUAN_STATE)
                {
                    ON_INT();//此时开启中断以方便接收
                    //空闲状态下，处理通讯程序
                    result = ReciveBufferDataDealing(&sendFrame, &recvFrame);//返回剩余长度
                    ClrWdt();
                    if (recvFrame.completeFlag == TRUE)
                    {
                        ExecuteFunctioncode(&recvFrame);
                    }
                    ClrWdt();
                    if (result) //若缓存中还有数据先处理帧数据。
                    {
                        continue;
                    }
                }
                result = CheckIOState();
                if (result) //收到合分闸指令，退出后立即进行循环
                {
                    continue;
                }
                ClrWdt(); 
                //检测是否欠电压， 并更新显示
                CheckVoltage();
                CheckSwitchState();
                ClrWdt();
                //此处应不断刷新一些IO状态，如RD。
                break;
            }
            default:
            {
                ClrWdt();
                RunStateFlag = IDLE_ORDER; //某种错误状态
                RESET_CURRENT_A();          //始终刷新空闲状态
                break;
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
    ClrWdt();
    //IGBT引脚
    //A
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

    HezhaTimeA = HEZHA_TIME_A;
    FenzhaTimeA = FENZHA_TIME_A;
    
    WaitIntOrder =  NULL_ORDER;
     
    HeActionLock = OFF_LOCK;
    FenActionLock = OFF_LOCK;
}  

/**************************************************
 *函数名：HeZhaActionA()
 *功能: 合闸动作
 *形参：  void
 *返回值：void
****************************************************/
inline void HeZhaActionA(void)
{
    //关中断 配对使用
    ClrWdt();
    OFF_INT();
     
    Init_Timer1(HezhaTimeA);
    RunStateFlag = HE_ORDER_A;
    RunStateFlag = HE_ORDER_A;
    HEZHA_A();
    HEZHA_A();
    LastOrderState = HE_ORDER_A;
    //此处添加上锁判断
    if (ON_HE_LOCK != HeActionLock) //若不为上锁态，说明指令下达有问题或者无故跳转到此处。
    {
        ClrWdt();
        RESET_CURRENT_A();//始终刷新空闲状态
        RESET_CURRENT_A();
        RunStateFlag =IDLE_ORDER;
        RunStateFlag =IDLE_ORDER;
        LastOrderState = IDLE_ORDER;
        ON_INT();
        return;
    }
                
    StartTimer1();
}
/**************************************************
 *函数名：FenZhaActionA()
 *功能: 分闸动作
 *形参：  void
 *返回值：void
****************************************************/
inline  void FenZhaActionA(void)
{
    //关中断 配对使用
    ClrWdt();
    OFF_INT();
    
    Init_Timer1(FenzhaTimeA);
    RunStateFlag = FEN_ORDER_A;
    RunStateFlag = FEN_ORDER_A;
    FENZHA_A();
    FENZHA_A();
    LastOrderState = FEN_ORDER_A;
    
    //此处添加上锁判断
     if (ON_FEN_LOCK != FenActionLock) //若不为上锁态，说明指令下达有问题或者无故跳转到此处。
    {
        ClrWdt();
        RESET_CURRENT_A();//始终刷新空闲状态
        RESET_CURRENT_A();
        RunStateFlag =IDLE_ORDER;
        RunStateFlag =IDLE_ORDER;
        LastOrderState = IDLE_ORDER;
        ON_INT();
        return;
    }
    StartTimer1();
}

/**************************************************
 *函数名： UpdateCount()
 *功能: 根据合闸与分闸更新EEPROM存在的计数
 *形参：  uint16 state--合闸或分闸命令
 *返回值：void
****************************************************/
void UpdateCount(uint16 state)
{
    //应禁止中断
    _prog_addressT addr; 
    if (state == HE_ORDER_A) //机构1合闸
    {
        addr = (_prog_addressT)JG1_HE_COUNT_ADDRESS;
        OFF_INT();  //不允许CAN中断
        WriteFenzhaCount(addr);
    }
    else if (state == FEN_ORDER_A)  //机构1分闸
    {
        addr = (_prog_addressT)JG1_FEN_COUNT_ADDRESS;
        OFF_INT();  //不允许CAN中断
        WriteFenzhaCount(addr);
    }
    else if (state == HE_ORDER_B)   //机构1合闸
    {
        addr = (_prog_addressT)JG2_HE_COUNT_ADDRESS;
        OFF_INT();  //不允许CAN中断
        WriteFenzhaCount(addr);
    }
    else if (state == FEN_ORDER_B)  //机构1分闸
    {
        addr = (_prog_addressT)JG2_FEN_COUNT_ADDRESS;
        OFF_INT();  //不允许CAN中断
        WriteFenzhaCount(addr);
    }
    else if (state == HE_ORDER)     //两个机构同时合闸
    {
        OFF_INT();  //不允许CAN中断
        addr = (_prog_addressT)JG1_HE_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
        addr = (_prog_addressT)JG2_HE_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
    }
    else if (state == FEN_ORDER)    //两个机构同时分闸
    {
        OFF_INT();  //不允许CAN中断
        addr = (_prog_addressT)JG1_FEN_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
        addr = (_prog_addressT)JG2_FEN_COUNT_ADDRESS;
        WriteFenzhaCount(addr);
    }
    else
    {
        return;
    }
    
    ON_INT();  //允许CAN中断
}