/** 
 * <p>application name： SwtichCondition.c</p> 
 * <p>application describing： 开入量值获取</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 

 * 摘要:
 * 2015/8/10:完善远本转换。
 * 2015/11/17: 改变IO检测模式，由纯延时形式，变成采样点样式。
 * 将远方检测IO与本地合并。

 * @updata:[2017-05-20] [张东旭][更改IO读取方式，采用的是并口转串口，165采样。扫描改为定时器扫描]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */
#include "SwtichCondition.h"
#include "../Header.h"
#include "DeviceParameter.h"

#define COIL1_HEZHA()   (HZHA1_INPUT | FENWEI1_INPUT)   //机构1合闸条件
#define COIL1_FENZHA()  (FZHA1_INPUT | HEWEI1_INPUT)    //机构1分闸条件

#define COIL2_HEZHA()   (HZHA2_INPUT | FENWEI2_INPUT)   //机构2合闸条件
#define COIL2_FENZHA()  (FZHA2_INPUT | HEWEI2_INPUT)    //机构2分闸条件

#define COIL3_HEZHA()   (HZHA3_INPUT | FENWEI3_INPUT)   //机构3合闸条件
#define COIL3_FENZHA()  (FZHA3_INPUT | HEWEI3_INPUT)    //机构3分闸条件

//远方\本地控制检测
#define YUAN_BEN_CONDITION()    ((DigitalInputState & YUAN_INPUT) == YUAN_INPUT)

//工作\调试模式检测
#define WORK_DEBUG_CONDITION()  ((DigitalInputState & WORK_INPUT) == WORK_INPUT)

//带电指示
#define CHARGED_CONDITION()  ((DigitalInputState & DIANXIAN_INPUT) == DIANXIAN_INPUT)

#if(CAP3_STATE)
//本地的总合闸条件：总分位 && 总合闸信号 && 本地控制 && 工作模式 && 电压满足 && 合闸信号未输入
#define Z_HEZHA_CONDITION()     ((DigitalInputState)== (Z_HEZHA_INPUT | FENWEI1_INPUT | FENWEI2_INPUT | FENWEI3_INPUT | WORK_INPUT))

//本地的总分闸条件：总合位 && 总分闸信号 && 本地控制 && 工作模式 && 不带电 && 电压满足 && 分闸信号未输入
#define Z_FENZHA_CONDITION()    \
(DigitalInputState == (Z_FENZHA_INPUT | HEWEI1_INPUT | HEWEI2_INPUT | HEWEI3_INPUT | WORK_INPUT) && (DigitalInputState & DIANXIAN_INPUT) == 0)

#else
//本地的总合闸条件：总分位 && 总合闸信号 && 本地控制 && 工作模式 && 电压满足 && 合闸信号未输入
#define Z_HEZHA_CONDITION()     (DigitalInputState == (Z_HEZHA_INPUT | FENWEI1_INPUT | FENWEI2_INPUT | WORK_INPUT))

//本地的总分闸条件：总合位 && 总分闸信号 && 本地控制 && 工作模式 && 不带电 && 电压满足 && 分闸信号未输入
#define Z_FENZHA_CONDITION()    \
    (DigitalInputState == (Z_FENZHA_INPUT | HEWEI1_INPUT | HEWEI2_INPUT | WORK_INPUT) && (DigitalInputState & DIANXIAN_INPUT) == 0)
#endif


//本地的机构1合闸条件：分位1 && 合闸1信号 && 电压1满足 && 本地控制 && 调试模式 && 合闸信号未输入
#define HEZHA1_CONDITION()      ((DigitalInputState & (COIL1_HEZHA() | WORK_INPUT)) == COIL1_HEZHA() &&  \
                                 (g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down))
//本地的机构1分闸条件：合位1 && 分闸1信号 && 电压1满足 && 本地控制 && 调试模式 && 不带电 && 分闸信号未输入
#define FENZHA1_CONDITION()    \
((DigitalInputState & (COIL1_FENZHA() | WORK_INPUT)) == COIL1_FENZHA() && (DigitalInputState & DIANXIAN_INPUT) == 0 &&    \
 (g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down))

//本地的机构2合闸条件：分位2 && 合闸2信号 && 电压2满足 && 本地控制 && 调试模式 && 合闸信号未输入
#define HEZHA2_CONDITION()      ((DigitalInputState & (COIL2_HEZHA() | WORK_INPUT)) == COIL2_HEZHA() &&  \
                                 (g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down))
//本地的机构2分闸条件：合位2 && 分闸2信号 && 电压2满足 && 本地控制 && 调试模式 && 不带电 && 分闸信号未输入
#define FENZHA2_CONDITION()    \
((DigitalInputState & (COIL2_FENZHA() | WORK_INPUT)) == COIL2_FENZHA() && (DigitalInputState & DIANXIAN_INPUT) == 0 &&    \
 (g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down))

//本地的机构3合闸条件：分位2 && 合闸3信号 && 电压2满足 && 本地控制 && 调试模式 && 合闸信号未输入
#define HEZHA3_CONDITION()      ((DigitalInputState & (COIL3_HEZHA() | WORK_INPUT)) == COIL3_HEZHA() &&  \
                                 (g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down))
//本地的机构3分闸条件：合位2 && 分闸3信号 && 电压2满足 && 本地控制 && 调试模式 && 不带电 && 分闸信号未输入
#define FENZHA3_CONDITION()    \
((DigitalInputState & (COIL3_FENZHA() | WORK_INPUT)) == COIL3_FENZHA() && (DigitalInputState & DIANXIAN_INPUT) == 0 &&    \
 (g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down))


/**
 * 
 * <p>Discription: [对机构的合分位检测]</p>
 */
//******************************************************************************
//机构1的合位检测
#define HEWEI1_CONDITION()  ((DigitalInputState & HEWEI1_INPUT) == HEWEI1_INPUT)
//机构1的分位检测
#define FENWEI1_CONDITION()  ((DigitalInputState & FENWEI1_INPUT) == FENWEI1_INPUT)

//机构2的合位检测
#define HEWEI2_CONDITION()  ((DigitalInputState & HEWEI2_INPUT) == HEWEI2_INPUT)
//机构2的分位检测
#define FENWEI2_CONDITION()  ((DigitalInputState & FENWEI2_INPUT) == FENWEI2_INPUT)

//机构3的合位检测
#define HEWEI3_CONDITION()  ((DigitalInputState & HEWEI3_INPUT) == HEWEI3_INPUT)
//机构3的分位检测
#define FENWEI3_CONDITION()  ((DigitalInputState & FENWEI3_INPUT) == FENWEI3_INPUT)
//******************************************************************************


/**
 * <p>Discription: [一下错误均为不可屏蔽掉的错误，且错误严重]</p>
 */
//***************************************************************************************************
//机构1的本地错误条件: 合位和分位同时成立 或 同时不成立
#define ERROR1_CONDITION()      \
        (((DigitalInputState & (FENWEI1_INPUT | HEWEI1_INPUT)) == (FENWEI1_INPUT | HEWEI1_INPUT)) ||   \
        ((DigitalInputState & (FENWEI1_INPUT | HEWEI1_INPUT)) == 0))

//机构2的本地错误条件: 合位和分位同时成立 或 同时不成立
#define ERROR2_CONDITION()      \
        (((DigitalInputState & (FENWEI2_INPUT | HEWEI2_INPUT)) == (FENWEI2_INPUT | HEWEI2_INPUT)) ||   \
        ((DigitalInputState & (FENWEI2_INPUT | HEWEI2_INPUT)) == 0))

//机构3的本地错误条件: 合位和分位同时成立 或 同时不成立
#define ERROR3_CONDITION()      \
        ((DigitalInputState & (FENWEI3_INPUT | HEWEI3_INPUT)) == (FENWEI3_INPUT | HEWEI3_INPUT) ||   \
        ((DigitalInputState & (FENWEI3_INPUT | HEWEI3_INPUT)) == 0))
//***************************************************************************************************


#define INPUT_FILT_TIME 20   // * 2ms
#define MIN_EFFECTIVE_TIME  18  //最小的有效时间是（INPUT_FILT_TIME * 90%）
static uint8_t g_timeCount[17] = {0};
static uint8_t ScanCount = 0;    //扫描计数
static uint32_t volatile DigitalInputState = 0;    //165返回值
static uint8_t LastSwitchState[LOOP_QUANTITY] = {0};   //获取上一次开关分合位状态

//**********************************************
//显示缓冲，主要用于存放继电器和LED灯的数据
static uint16_t OpenDisplayBuffer[6] = {0};
static uint16_t CloseDisplayBuffer[6] = {0};
static uint16_t CapDisplayBuffer[6] = {0};
static uint16_t ErrorDisplayBuffer[6] = {0};
//**********************************************

void UpdataSwitchState(void);

/**
 * 
 * <p>Function name: [DisplayBufferInit]</p>
 * <p>Discription: [初始化显示缓冲数组，数组指向继电器和指示灯]</p>
 */
void DisplayBufferInit(void)
{
    uint8_t index = 0;
    CloseDisplayBuffer[index] = HEWEI1_RELAY;
    index++;
    CloseDisplayBuffer[index] = HEWEI2_RELAY;
    index++;
    CloseDisplayBuffer[index] = HEWEI3_RELAY;
    index++;
    CloseDisplayBuffer[index] = HEWEI1_LED;
    index++;
    CloseDisplayBuffer[index] = HEWEI2_LED;
    index++;
    CloseDisplayBuffer[index] = HEWEI3_LED;
    index = 0;
    
    OpenDisplayBuffer[index] = FENWEI1_RELAY;
    index++;
    OpenDisplayBuffer[index] = FENWEI2_RELAY;
    index++;
    OpenDisplayBuffer[index] = FENWEI3_RELAY;
    index++;
    OpenDisplayBuffer[index] = FENWEI1_LED;
    index++;
    OpenDisplayBuffer[index] = FENWEI2_LED;
    index++;
    OpenDisplayBuffer[index] = FENWEI3_LED;
    index = 0;
    
    ErrorDisplayBuffer[index] = ERROR1_RELAY;
    index++;
    ErrorDisplayBuffer[index] = ERROR2_RELAY;
    index++;
    ErrorDisplayBuffer[index] = ERROR3_RELAY;
    index++;
    ErrorDisplayBuffer[index] = ERROR1_LED;
    index++;
    ErrorDisplayBuffer[index] = ERROR2_LED;
    index++;
    ErrorDisplayBuffer[index] = ERROR3_LED;
    index = 0;
    
    CapDisplayBuffer[index] = CAP1_RELAY;
    index++;
    CapDisplayBuffer[index] = CAP2_RELAY;
    index++;
    CapDisplayBuffer[index] = CAP3_RELAY;
    index++;
    CapDisplayBuffer[index] = CAP1_LED;
    index++;
    CapDisplayBuffer[index] = CAP2_LED;
    index++;
    CapDisplayBuffer[index] = CAP3_LED;
    index = 0;
}
/**
 * 
 * <p>Function name: [CheckIOState]</p>
 * <p>Discription: [检测IO状态，并更新状态显示]</p>
 * @return 接收到分合闸命令返回0xFF,否则返回0
 */
uint8_t CheckIOState(void)
{
    ClrWdt();     
    if(g_LockUp)    //上锁状态下不能进行操作
    {
        g_Order = IDLE_ORDER;    //将命令清零
        return 0;
    }
    switch (g_Order)
    {
        //检测按键状态
        case CHECK_Z_HE_ORDER: //收到合闸命令 需要判断一下电容电压能否达到要求
        {
            ClrWdt();
            if((g_SystemState.workMode == WORK_STATE) && (GetCapVolatageState()))
            {
               // TongBuHeZha(); TODO:
                ClrWdt();
                SingleCloseOperation(LOOP_A , g_DelayTime.hezhaTime1);
                SingleCloseOperation(LOOP_B , g_DelayTime.hezhaTime2);
                #if(CAP3_STATE)  //判断第三块驱动是否存在
                {
                    SingleCloseOperation(LOOP_C , g_DelayTime.hezhaTime3);
                }
                #endif
                g_RemoteControlState.orderId = CloseAction;    //拒动错误ID号
                ClrWdt();
                g_SuddenState.ExecuteOrder[LOOP_A] = CLOSE_STATE;
                g_SuddenState.ExecuteOrder[LOOP_B] = CLOSE_STATE;
                g_SuddenState.ExecuteOrder[LOOP_C] = CLOSE_STATE;
                return 0xFF;
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
        }
        case CHECK_Z_FEN_ORDER: //收到分闸命令
        {
            ClrWdt();
            if(g_SystemState.charged == TRUE)   //带电不能执行分闸操作
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0;
            }
            if((g_SystemState.workMode == WORK_STATE) && (GetCapVolatageState())) //多加入一重验证
            {
                ClrWdt();
                SingleOpenOperation(LOOP_A , g_DelayTime.fenzhaTime1);
                SingleOpenOperation(LOOP_B , g_DelayTime.fenzhaTime2);
                #if(CAP3_STATE)  //判断第三块驱动是否存在
                {
                    SingleOpenOperation(LOOP_C , g_DelayTime.fenzhaTime3);
                }
                #endif
                g_RemoteControlState.orderId = OpenAction;    //拒动错误ID号
                ClrWdt();
                g_SuddenState.ExecuteOrder[LOOP_A] = OPEN_STATE;
                g_SuddenState.ExecuteOrder[LOOP_B] = OPEN_STATE;
                g_SuddenState.ExecuteOrder[LOOP_C] = OPEN_STATE;
                return 0xFF;
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
        }
        
        case CHECK_1_HE_ORDER: //收到机构1合闸命令
        {
            ClrWdt();
            if(g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down)
            {
                SingleCloseOperation(LOOP_A , g_DelayTime.hezhaTime1);
                g_SuddenState.ExecuteOrder[LOOP_A] = CLOSE_STATE;
                g_RemoteControlState.orderId = CloseAction;    //拒动错误ID号
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
            return 0xFF;
        }
        case CHECK_1_FEN_ORDER: //收到机构1分闸命令
        {
            ClrWdt();
            if(g_SystemState.charged == TRUE)   //带电不能执行分闸操作
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0;
            }
            if(g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down)
            {
                SingleOpenOperation(LOOP_A , g_DelayTime.fenzhaTime1);
                g_SuddenState.ExecuteOrder[LOOP_A] = OPEN_STATE;
                g_RemoteControlState.orderId = OpenAction;    //拒动错误ID号
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
            return 0xFF;
        }
        
        case CHECK_2_HE_ORDER: //收到机构2合闸命令
        {
            ClrWdt();        
            if(g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down)
            {
                SingleCloseOperation(LOOP_B , g_DelayTime.hezhaTime2);
                g_SuddenState.ExecuteOrder[LOOP_B] = CLOSE_STATE;
                g_RemoteControlState.orderId = CloseAction;    //拒动错误ID号
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
            return 0xFF;
        }
        case CHECK_2_FEN_ORDER: //收到机构2分闸命令
        {
            ClrWdt();
            if(g_SystemState.charged == TRUE)   //带电不能执行分闸操作
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0;
            }
            if(g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down)
            {
                SingleOpenOperation(LOOP_B , g_DelayTime.fenzhaTime2);
                g_SuddenState.ExecuteOrder[LOOP_B] = OPEN_STATE;
                g_RemoteControlState.orderId = OpenAction;    //拒动错误ID号
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
            return 0xFF;
        }
        
        case CHECK_3_HE_ORDER: //收到机构3合闸命令
        {
            ClrWdt();        
            if(CAP3_STATE && (g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down))  //判断第三块驱动是否存在
            {
                SingleCloseOperation(LOOP_C , g_DelayTime.hezhaTime3);
                g_SuddenState.ExecuteOrder[LOOP_C] = CLOSE_STATE;
                g_RemoteControlState.orderId = CloseAction;    //拒动错误ID号
            }       
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }     
            return 0xFF;
        }
        case CHECK_3_FEN_ORDER: //收到机构3分闸命令
        {
            ClrWdt();
            if(g_SystemState.charged == TRUE)   //带电不能执行分闸操作
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0;
            }
            if(CAP3_STATE && (g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down))  //判断第三块驱动是否存在
            {
                SingleOpenOperation(LOOP_C , g_DelayTime.fenzhaTime3);
                g_SuddenState.ExecuteOrder[LOOP_C] = OPEN_STATE;
                g_RemoteControlState.orderId = OpenAction;    //拒动错误ID号
            }       
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }     
            return 0xFF;
        }
        default:
        {
            ClrWdt();
        }
    }
    return 0;
}
/**
 * 
 * <p>Function name: [CheckSwitchState]</p>
 * <p>Discription: [执行相应的指示]</p>
 */
void DsplaySwitchState(void)
{    
    
#if(CAP3_STATE)
    uint8_t loopCount = LOOP_QUANTITY;
#else
    uint8_t loopCount = LOOP_QUANTITY - 1;
#endif
    uint8_t waringBuffer[loopCount];
    uint8_t offestIndex = 3;
    
    UpdataCapVoltageState();   //更新电容电压显示状态
    
    if(!g_SuddenState.SuddenFlag)   //没有突发状态不执行
    {
        return;
    }
    
    waringBuffer[LOOP_A] = g_SystemState.heFenState1; //机构1警告
    waringBuffer[LOOP_B] = g_SystemState.heFenState2; //机构2警告
#if(CAP3_STATE)
    waringBuffer[LOOP_C] = g_SystemState.heFenState3; //机构3警告
#else
    g_SystemState.heFenState3 = g_SystemState.heFenState2;  //主要应用在判断总分合位
#endif
    
    for(uint8_t index = 0; index < loopCount; index++)
    {
        ClrWdt();
        switch (waringBuffer[index])
        {
            case ERROR_STATE:
            {
                UpdateIndicateState(CloseDisplayBuffer[index], CloseDisplayBuffer[index + offestIndex], TURN_OFF);  //关闭合位指示灯
                UpdateIndicateState(OpenDisplayBuffer[index], OpenDisplayBuffer[index + offestIndex], TURN_OFF);    //关闭分位指示灯
                UpdateIndicateState(ErrorDisplayBuffer[index], ErrorDisplayBuffer[index + offestIndex], TURN_ON);   //开启错误指示灯
                g_SystemState.warning = waringBuffer[index];
                break;
            }
            case OPEN_STATE:
            {
                UpdateIndicateState(CloseDisplayBuffer[index], CloseDisplayBuffer[index + offestIndex], TURN_OFF);  //关闭合位指示灯
                UpdateIndicateState(OpenDisplayBuffer[index], OpenDisplayBuffer[index + offestIndex], TURN_ON);     //开启分位指示灯
                UpdateIndicateState(ErrorDisplayBuffer[index], ErrorDisplayBuffer[index + offestIndex], TURN_OFF);  //关闭错误指示灯
                g_SystemState.warning = waringBuffer[index];
                break;
            }
            case CLOSE_STATE:
            {
                UpdateIndicateState(CloseDisplayBuffer[index], CloseDisplayBuffer[index + offestIndex], TURN_ON);   //开启合位指示灯
                UpdateIndicateState(OpenDisplayBuffer[index], OpenDisplayBuffer[index + offestIndex], TURN_OFF);    //关闭分位指示灯
                UpdateIndicateState(ErrorDisplayBuffer[index], ErrorDisplayBuffer[index + offestIndex], TURN_OFF);  //关闭错误指示灯
                g_SystemState.warning = waringBuffer[index];
                break;
            }
        }
        switch(g_SuddenState.CapState[index])
        {
            case CAP_UPPER_STATE:
            {
                UpdateIndicateState(ErrorDisplayBuffer[index], ErrorDisplayBuffer[index + offestIndex], TURN_ON);   //开启错误指示灯
                break;
            }
            case CAP_NORMAL_STATE:
            {
                UpdateIndicateState(CapDisplayBuffer[index], CapDisplayBuffer[index + offestIndex], TURN_ON);   //开启电容指示灯
                if(g_SystemState.warning != ERROR_STATE)
                {
                    UpdateIndicateState(ErrorDisplayBuffer[index], ErrorDisplayBuffer[index + offestIndex], TURN_OFF);  //关闭错误指示灯
                }
                break;
            }
            case CAP_LOW_STATE:
            {
                UpdateIndicateState(CapDisplayBuffer[index], CapDisplayBuffer[index + offestIndex], TURN_OFF);  //开启电容指示灯
                if(g_SystemState.warning != ERROR_STATE)
                {
                    UpdateIndicateState(ErrorDisplayBuffer[index], ErrorDisplayBuffer[index + offestIndex], TURN_OFF);  //关闭错误指示灯
                }
                break;
            }
        }
    }
    //总的分合位检测    
    //分闸状态
    ClrWdt();
    if((g_SystemState.heFenState2 == OPEN_STATE) && (g_SystemState.heFenState1 == OPEN_STATE) && 
       (g_SystemState.heFenState3 == OPEN_STATE))
    {
        //合位指示灯亮 
        UpdateIndicateState(Z_FENWEI_RELAY,Z_FENWEI_LED,TURN_ON);
        UpdateIndicateState(Z_HEWEI_RELAY,Z_HEWEI_LED,TURN_OFF);
        ClrWdt();
        if(g_SuddenState.RefuseAction == Z_HE_ERROR)
        {
            g_TimeStampCollect.changeLedTime.delayTime = 500;  //发生拒动错误后，指示灯闪烁间隔变短
            g_SuddenState.RefuseAction = FALSE; //Clear
        }
    }
    else if((g_SystemState.heFenState2 == CLOSE_STATE)&&(g_SystemState.heFenState1 == CLOSE_STATE) && 
       (g_SystemState.heFenState3 == CLOSE_STATE)) //合闸状态
    {
        //分位指示灯亮 
        UpdateIndicateState(Z_HEWEI_RELAY,Z_HEWEI_LED,TURN_ON);
        UpdateIndicateState(Z_FENWEI_RELAY,Z_FENWEI_LED,TURN_OFF);
        ClrWdt();
        if(g_SuddenState.RefuseAction == Z_FEN_ERROR)
        {
            g_TimeStampCollect.changeLedTime.delayTime = 500;  //发生拒动错误后，指示灯闪烁间隔变短
            g_SuddenState.RefuseAction = FALSE; //Clear
        }
    }
    else
    {
        UpdateIndicateState(Z_FENWEI_RELAY,Z_FENWEI_LED,TURN_OFF);
        UpdateIndicateState(Z_HEWEI_RELAY,Z_HEWEI_LED,TURN_OFF);
    }
}
/**
 * 
 * <p>Function name: [SwitchScan]</p>
 * <p>Discription: [检测开入量]</p>
 */
void SwitchScan(void)
{
    DigitalInputState = ReadHC165();
    ScanCount++;  //每次扫描均计数
    //远方\就地检测
    if(YUAN_BEN_CONDITION())
    {
        g_timeCount[0] ++;
    }
    else
    {
        if(g_timeCount[0] != 0)
        {
            g_timeCount[0] --;
        }
    }
    //*****************************
    //作用，屏蔽掉远方就地
    uint32_t i = ~YUAN_INPUT;
    DigitalInputState &= i;
    //*****************************
    if(g_LockUp == OFF_LOCK)    //首先判断是否正在执行合闸或者分闸操作
    {
        ClrWdt();
        //同时合闸信号检测
        if(Z_HEZHA_CONDITION())
        {
            g_timeCount[1] ++;
        }
        else
        {
            if(g_timeCount[1] != 0)
            {
                g_timeCount[1] --;
            }
        }
        //同时分闸信号检测
        if(Z_FENZHA_CONDITION())
        {
            g_timeCount[2] ++;
        }
        else
        {
            if(g_timeCount[2] != 0)
            {
                g_timeCount[2] --;
            }
        }
        //机构1合闸信号检测
        if(HEZHA1_CONDITION())
        {
            g_timeCount[3]++;
        }
        else
        {
            if(g_timeCount[3] != 0)
            {
                g_timeCount[3] --;
            }
        }
        //机构1分闸信号检测
        if(FENZHA1_CONDITION())
        {
            g_timeCount[4]++;
        }
        else
        {
            if(g_timeCount[4] != 0)
            {
                g_timeCount[4] --;
            }
        }
        //机构2合闸信号检测
        if(HEZHA2_CONDITION())
        {
            g_timeCount[5]++;
        }
        else
        {
            if(g_timeCount[5] != 0)
            {
                g_timeCount[5] --;
            }
        }
        //机构2分闸信号检测
        if(FENZHA2_CONDITION())
        {
            g_timeCount[6]++;
        }
        else
        {
            if(g_timeCount[6] != 0)
            {
                g_timeCount[6] --;
            }
        }        
    #if(CAP3_STATE)  //判断第三块驱动是否存在
    {
        //机构3合闸信号检测
        if(HEZHA3_CONDITION())
        {
            g_timeCount[7]++;
        }
        else
        {
            if(g_timeCount[7] != 0)
            {
                g_timeCount[7] --;
            }
        }
        //机构3分闸信号检测
        if(FENZHA3_CONDITION())
        {
            g_timeCount[8]++;
        }
        else
        {
            if(g_timeCount[8] != 0)
            {
                g_timeCount[8] --;
            }
        }
    }
    #endif
    }
    //机构1的合位检测
    if(HEWEI1_CONDITION())
    {
        g_timeCount[9]++;
    }
    else
    {
        if(g_timeCount[9] != 0)
        {
            g_timeCount[9] --;
        }
    }   
    //机构1的分位检测
    if(FENWEI1_CONDITION())
    {
        g_timeCount[10]++;
    }
    else
    {
        if(g_timeCount[10] != 0)
        {
            g_timeCount[10] --;
        }
    }      
    //机构2的合位检测
    if(HEWEI2_CONDITION())
    {
        g_timeCount[11]++;
    }
    else
    {
        if(g_timeCount[11] != 0)
        {
            g_timeCount[11] --;
        }
    }
    //机构2的分位检测
    if(FENWEI2_CONDITION())
    {
        g_timeCount[12]++;
    }
    else
    {
        if(g_timeCount[12] != 0)
        {
            g_timeCount[12] --;
        }
    }
#if(CAP3_STATE)  //判断第三块驱动是否存在
{
    //机构3的合位检测
    if(HEWEI3_CONDITION())
    {
        g_timeCount[13]++;
    }
    else
    {
        if(g_timeCount[13] != 0)
        {
            g_timeCount[13] --;
        }
    }
    //机构3的分位检测
    if(FENWEI3_CONDITION())
    {
        g_timeCount[14]++;
    }
    else
    {
        if(g_timeCount[14] != 0)
        {
            g_timeCount[14] --;
        }
    }
}
#endif
    //工作\调试模式检测
    if(WORK_DEBUG_CONDITION())
    {
        g_timeCount[15]++;
    }
    else
    {
        if(g_timeCount[15] != 0)
        {
            g_timeCount[15] --;
        }
    }
    //带电闭锁操作
    if(CHARGED_CONDITION())
    {
        g_timeCount[16] ++;
    }
    else
    {
        if(g_timeCount[16] != 0)
        {
            g_timeCount[16] --;
        }
    }
    
//******************************************************************************
    //时间到达后才对各个状态位进行检查
    if(ScanCount >= INPUT_FILT_TIME)
    {
        ScanCount = 0;    //Clear
        //首先对远方就地进行检查
        if(g_timeCount[0] >= MIN_EFFECTIVE_TIME)
        {
            g_SystemState.yuanBenState = YUAN_STATE;
            g_timeCount[0] = 0;
        }
        else
        {
            g_SystemState.yuanBenState = BEN_STATE;
            g_timeCount[0] = 0;
        }
        if(g_LockUp == OFF_LOCK)    //首先判断是否正在执行合闸或者分闸操作
        {
            //对总合总分信号检测        
            if((g_timeCount[1] >= MIN_EFFECTIVE_TIME) && (g_timeCount[2] < MIN_EFFECTIVE_TIME))
            {
                if((g_SystemState.heFenState1 == OPEN_STATE) && 
                   (g_SystemState.heFenState2 == OPEN_STATE) && 
                   (g_SystemState.heFenState3 == OPEN_STATE))
                {
                    g_Order = CHECK_Z_HE_ORDER;     //同时合闸命令
                    OnLock();   //上锁
                }
                g_timeCount[1] = 0;
                g_timeCount[2] = 0;
            }
            else if((g_timeCount[2] >= MIN_EFFECTIVE_TIME) && (g_timeCount[1] < MIN_EFFECTIVE_TIME))
            {
                if((g_SystemState.heFenState1 == CLOSE_STATE) && 
                   (g_SystemState.heFenState2 == CLOSE_STATE) && 
                   (g_SystemState.heFenState3 == CLOSE_STATE) )
                {
                    g_Order = CHECK_Z_FEN_ORDER;     //同时分闸命令
                    OnLock();   //上锁
                }
                g_timeCount[1] = 0;
                g_timeCount[2] = 0;
            }
            //对机构1的合分信号检测
            if((g_timeCount[3] >= MIN_EFFECTIVE_TIME) && (g_timeCount[4] < MIN_EFFECTIVE_TIME))
            {
                if(g_SystemState.heFenState1 == OPEN_STATE)
                {
                    g_Order = CHECK_1_HE_ORDER;     //同时合闸命令
                    OnLock();   //上锁
                }
                g_timeCount[3] = 0;
                g_timeCount[4] = 0;
            }
            else if((g_timeCount[4] >= MIN_EFFECTIVE_TIME) && (g_timeCount[3] < MIN_EFFECTIVE_TIME))
            {
                if(g_SystemState.heFenState1 == CLOSE_STATE)
                {
                    g_Order = CHECK_1_FEN_ORDER;     //同时分闸命令
                    OnLock();   //上锁
                }
                g_timeCount[3] = 0;
                g_timeCount[4] = 0;
            }
            //对机构2的合分信号检测   
            if((g_timeCount[5] >= MIN_EFFECTIVE_TIME) && (g_timeCount[6] < MIN_EFFECTIVE_TIME))
            {
                if(g_SystemState.heFenState2 == OPEN_STATE)
                {
                    g_Order = CHECK_2_HE_ORDER;     //同时合闸命令
                    OnLock();   //上锁
                }
                g_timeCount[5] = 0;
                g_timeCount[6] = 0;
            }
            else if((g_timeCount[6] >= MIN_EFFECTIVE_TIME) && (g_timeCount[5] < MIN_EFFECTIVE_TIME))
            {
                if(g_SystemState.heFenState2 == CLOSE_STATE)
                {
                    g_Order = CHECK_2_FEN_ORDER;     //同时分闸命令
                    OnLock();   //上锁
                }
                g_timeCount[5] = 0;
                g_timeCount[6] = 0;
            }
            //对机构3的合分信号检测   
#if(CAP3_STATE)  //判断第三块驱动是否存在
            if((g_timeCount[7] >= MIN_EFFECTIVE_TIME) && (g_timeCount[8] < MIN_EFFECTIVE_TIME))
            {
                if(g_SystemState.heFenState3 == OPEN_STATE)
                {
                    g_Order = CHECK_3_HE_ORDER;     //同时合闸命令
                    OnLock();   //上锁
                }
                g_timeCount[7] = 0;
                g_timeCount[8] = 0;
            }
            else if((g_timeCount[8] >= MIN_EFFECTIVE_TIME) && (g_timeCount[7] < MIN_EFFECTIVE_TIME))
            {
                if(g_SystemState.heFenState3 == CLOSE_STATE)
                {
                    g_Order = CHECK_3_FEN_ORDER;     //同时分闸命令
                    OnLock();   //上锁
                }
                g_timeCount[7] = 0;
                g_timeCount[8] = 0;
            }
#endif
        }
        
        //对机构1的合分位进行检查
        if((g_timeCount[9] >= MIN_EFFECTIVE_TIME) && (g_timeCount[10] < MIN_EFFECTIVE_TIME))
        {
            g_SystemState.heFenState1 = CLOSE_STATE;  
            g_timeCount[9] = 0;
            g_timeCount[10] = 0;
        }
        else if((g_timeCount[10] >= MIN_EFFECTIVE_TIME) && (g_timeCount[9] < MIN_EFFECTIVE_TIME))
        {
            g_SystemState.heFenState1 = OPEN_STATE;  
            g_timeCount[9] = 0;
            g_timeCount[10] = 0;
        }
        else    //此时存在错误
        {            
            g_SystemState.heFenState1 = ERROR_STATE;     //合分位同时存在\不存在          
            g_timeCount[9] = 0;
            g_timeCount[10] = 0;
        }
        
        //对机构2的合分位进行检查
        if((g_timeCount[11] >= MIN_EFFECTIVE_TIME) && (g_timeCount[12] < MIN_EFFECTIVE_TIME))
        {
            g_SystemState.heFenState2 = CLOSE_STATE;  
            g_timeCount[11] = 0;
            g_timeCount[12] = 0;
        }
        else if((g_timeCount[12] >= MIN_EFFECTIVE_TIME) && (g_timeCount[11] < MIN_EFFECTIVE_TIME))
        {
            g_SystemState.heFenState2 = OPEN_STATE;  
            g_timeCount[11] = 0;
            g_timeCount[12] = 0;
        }
        else    //此时存在错误
        {            
            g_SystemState.heFenState2 = ERROR_STATE;     //合分位同时存在
            g_timeCount[11] = 0;
            g_timeCount[12] = 0;
        }
        
#if(CAP3_STATE)  //判断第三块驱动是否存在
        //对机构3的合分位进行检查
        if((g_timeCount[13] >= MIN_EFFECTIVE_TIME) && (g_timeCount[14] < MIN_EFFECTIVE_TIME))
        {
            g_SystemState.heFenState3 = CLOSE_STATE;          
            g_timeCount[13] = 0;
            g_timeCount[14] = 0;
        }
        else if((g_timeCount[14] >= MIN_EFFECTIVE_TIME) && (g_timeCount[13] < MIN_EFFECTIVE_TIME))
        {
            g_SystemState.heFenState3 = OPEN_STATE;     
            g_timeCount[13] = 0;
            g_timeCount[14] = 0;
        }
        else    //此时存在错误
        {            
            g_SystemState.heFenState3 = ERROR_STATE;     //合分位同时存在
            g_timeCount[13] = 0;
            g_timeCount[14] = 0;
        }
#endif
        //工作\调试模式判断
        if(g_timeCount[15] >= MIN_EFFECTIVE_TIME)
        {
            g_SystemState.workMode = WORK_STATE;
            g_timeCount[15] = 0;
        }
        else
        {
            g_SystemState.workMode = DEBUG_STATE;
            g_timeCount[15] = 0;
        }
        //首先对远方就地进行检查
        if(g_timeCount[16] >= MIN_EFFECTIVE_TIME)
        {
            g_SystemState.charged = TRUE;
            g_timeCount[16] = 0;
        }
        else
        {
            g_SystemState.charged = FALSE;
            g_timeCount[16] = 0;
        }
        UpdataSwitchState();    //更新按键状态
    }
//******************************************************************************
    
}

/**
 * 
 * <p>Function name: [UpdataSwitchState]</p>
 * <p>Discription: [更新更新开关分合位状态]</p>
 */
void UpdataSwitchState(void)
{
    if(LastSwitchState[LOOP_A] != g_SystemState.heFenState1)
    {
        LastSwitchState[LOOP_A] = g_SystemState.heFenState1;
        g_SuddenState.SuddenFlag = TRUE;
    }
    if(LastSwitchState[LOOP_B] != g_SystemState.heFenState2)
    {
        LastSwitchState[LOOP_B] = g_SystemState.heFenState2;
        g_SuddenState.SuddenFlag = TRUE;
    }
#if(CAP3_STATE)
    if(LastSwitchState[LOOP_C] != g_SystemState.heFenState3)
    {
        LastSwitchState[LOOP_C] = g_SystemState.heFenState3;
        g_SuddenState.SuddenFlag = TRUE;
    }
#endif
}

