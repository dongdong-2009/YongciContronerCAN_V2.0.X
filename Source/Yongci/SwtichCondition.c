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

#define COIL1_HEZHA()   (HZHA1_INPUT | FENWEI1_INPUT | WORK_INPUT)   //机构1合闸条件
#define COIL1_FENZHA()  (FZHA1_INPUT | HEWEI1_INPUT | WORK_INPUT)    //机构1分闸条件

#define COIL2_HEZHA()   (HZHA2_INPUT | FENWEI2_INPUT | WORK_INPUT)   //机构2合闸条件
#define COIL2_FENZHA()  (FZHA2_INPUT | HEWEI2_INPUT | WORK_INPUT)    //机构2分闸条件

#if(CAP3_STATE)
#define COIL3_HEZHA()   (HZHA3_INPUT | FENWEI3_INPUT | WORK_INPUT)   //机构3合闸条件
#define COIL3_FENZHA()  (FZHA3_INPUT | HEWEI3_INPUT | WORK_INPUT)    //机构3分闸条件
#define ALL_HEZHA()     (Z_HEZHA_INPUT | FENWEI1_INPUT | FENWEI2_INPUT | FENWEI3_INPUT)
#define ALL_FENZHA()    (Z_FENZHA_INPUT | HEWEI1_INPUT | HEWEI2_INPUT | HEWEI3_INPUT)

#else
#define ALL_HEZHA()     (Z_HEZHA_INPUT | FENWEI1_INPUT | FENWEI2_INPUT)
#define ALL_FENZHA()    (Z_FENZHA_INPUT | HEWEI1_INPUT | HEWEI2_INPUT)
#endif

//远方\本地控制检测
#define YUAN_BEN_CONDITION()    ((DigitalInputState & YUAN_INPUT) == YUAN_INPUT)

//工作\调试模式检测
#define WORK_DEBUG_CONDITION()  ((DigitalInputState & WORK_INPUT) == WORK_INPUT)

//带电指示
#define CHARGED_CONDITION()  ((DigitalInputState & DIANXIAN_INPUT) == DIANXIAN_INPUT)

//本地的总合闸条件：总分位 && 总合闸信号 && 本地控制 && 工作模式 && 电压满足 && 合闸信号未输入
#define Z_HEZHA_CONDITION()     ((DigitalInputState & (ALL_HEZHA() | WORK_INPUT)) == ALL_HEZHA())

//本地的总分闸条件：总合位 && 总分闸信号 && 本地控制 && 工作模式 && 不带电 && 电压满足 && 分闸信号未输入
#define Z_FENZHA_CONDITION()    ((DigitalInputState & (ALL_FENZHA() | DIANXIAN_INPUT | WORK_INPUT)) == ALL_FENZHA())

//本地的机构1合闸条件：分位1 && 合闸1信号 && 电压1满足 && 本地控制 && 调试模式 && 合闸信号未输入
#define HEZHA1_CONDITION()      ((DigitalInputState & COIL1_HEZHA()) == COIL1_HEZHA())
//本地的机构1分闸条件：合位1 && 分闸1信号 && 电压1满足 && 本地控制 && 调试模式 && 不带电 && 分闸信号未输入
#define FENZHA1_CONDITION()    \
((DigitalInputState & (COIL1_FENZHA() | DIANXIAN_INPUT)) == COIL1_FENZHA())

//本地的机构2合闸条件：分位2 && 合闸2信号 && 电压2满足 && 本地控制 && 调试模式 && 合闸信号未输入
#define HEZHA2_CONDITION()      ((DigitalInputState & COIL2_HEZHA()) == COIL2_HEZHA())
//本地的机构2分闸条件：合位2 && 分闸2信号 && 电压2满足 && 本地控制 && 调试模式 && 不带电 && 分闸信号未输入
#define FENZHA2_CONDITION()    \
((DigitalInputState & (COIL2_FENZHA() | DIANXIAN_INPUT)) == COIL2_FENZHA())

#if(CAP3_STATE)
//本地的机构3合闸条件：分位3 && 合闸3信号 && 电压2满足 && 本地控制 && 调试模式 && 合闸信号未输入
#define HEZHA3_CONDITION()      ((DigitalInputState & COIL3_HEZHA()) == COIL3_HEZHA())
//本地的机构3分闸条件：合位3 && 分闸3信号 && 电压2满足 && 本地控制 && 调试模式 && 不带电 && 分闸信号未输入
#define FENZHA3_CONDITION()    \
((DigitalInputState & (COIL3_FENZHA() | DIANXIAN_INPUT)) == COIL3_FENZHA())
#endif


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

#if(CAP3_STATE)
//机构3的合位检测
#define HEWEI3_CONDITION()  ((DigitalInputState & HEWEI3_INPUT) == HEWEI3_INPUT)
//机构3的分位检测
#define FENWEI3_CONDITION()  ((DigitalInputState & FENWEI3_INPUT) == FENWEI3_INPUT)
#endif
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

#if(CAP3_STATE)
//机构3的本地错误条件: 合位和分位同时成立 或 同时不成立
#define ERROR3_CONDITION()      \
        ((DigitalInputState & (FENWEI3_INPUT | HEWEI3_INPUT)) == (FENWEI3_INPUT | HEWEI3_INPUT) ||   \
        ((DigitalInputState & (FENWEI3_INPUT | HEWEI3_INPUT)) == 0))
#endif
//***************************************************************************************************


#define INPUT_FILT_TIME 30   // * 2ms
#define MIN_EFFECTIVE_TIME  18  //最小的有效时间是（INPUT_FILT_TIME * 90%）


#define DIGITAL_INPUT_EFFECTIVE(index)  (DigitalInputValidCount[index] >= MIN_EFFECTIVE_TIME)
#define DIGITAL_INPUT_INVALID(index)    (DigitalInputValidCount[index] < MIN_EFFECTIVE_TIME)

#define CLEAR_VALID_COUNT(index)    {(DigitalInputValidCount[index] = 0);}
#define ADD_VALID_COUNT(index)    {DigitalInputValidCount[index]++;}

/**
 * 数字量输入有效计数
 */
static uint8_t DigitalInputValidCount[17] = {0};    

static uint8_t ScanCount = 0;    //扫描计数
static uint32_t volatile DigitalInputState = 0;    //165返回值

uint8_t g_LastSwitchState[LOOP_COUNT] = {0};   //获取上一次开关分合位状态

//**********************************************
//显示字，主要用于存放继电器和LED灯的数据 
const uint16_t CloseShowWord[6] = {HEWEI1_RELAY, HEWEI2_RELAY, HEWEI3_RELAY, HEWEI1_LED, HEWEI2_LED, HEWEI3_LED};
const uint16_t OpenShowWord[6] = {FENWEI1_RELAY, FENWEI2_RELAY, FENWEI3_RELAY, FENWEI1_LED, FENWEI2_LED, FENWEI3_LED};
const uint16_t CapShowWord[6] = {CAP1_RELAY, CAP2_RELAY, CAP3_RELAY, CAP1_LED, CAP2_LED, CAP3_LED};
const uint16_t ErrorShowWord[6] = {ERROR1_RELAY, ERROR2_RELAY, ERROR3_RELAY, ERROR1_LED, ERROR2_LED, ERROR3_LED};
//**********************************************

void UpdateSwitchState(void);
void CheckAllLoopSwitchState(void);
uint8_t CheckSwitchOrder(void);

/**
 * 
 * <p>Function name: [CheckIOState]</p>
 * <p>Discription: [检测IO状态，并更新状态显示]</p>
 * @return 接收到分合闸命令返回0xFF,否则返回0
 */
uint8_t CheckIOState(void)
{
    ClrWdt();     
    switch (g_Order)
    {
        //检测按键状态
        case HE_ORDER: //收到合闸命令 需要判断一下电容电压能否达到要求
        {
            ClrWdt();
            if((g_SystemState.workMode == WORK_STATE) && (!CheckLoopCapVoltage(LOOP_ID_ALL)))
            {
                OnLock();   //上锁
                ClrWdt();
                SingleCloseOperation(DEVICE_I , g_DelayTime.hezhaTime1);
                SingleCloseOperation(DEVICE_II , g_DelayTime.hezhaTime2);
#if(CAP3_STATE)  //判断第三块驱动是否存在
                SingleCloseOperation(DEVICE_III , g_DelayTime.hezhaTime3);
#endif
                g_RemoteControlState.orderId = CloseAction;    //拒动错误ID号
                ClrWdt();
                g_SuddenState.executeOrder[DEVICE_I] = CLOSE_STATE;
                g_SuddenState.executeOrder[DEVICE_II] = CLOSE_STATE;
#if(CAP3_STATE)  //判断第三块驱动是否存在
                g_SuddenState.executeOrder[DEVICE_III] = CLOSE_STATE;
#endif
                return 0xFF;
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
        }
        case FEN_ORDER: //收到分闸命令
        {
            ClrWdt();
            if(g_SystemState.charged == TRUE)   //带电不能执行分闸操作
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0;
            }
            if((g_SystemState.workMode == WORK_STATE) && (!CheckLoopCapVoltage(LOOP_ID_ALL))) //多加入一重验证
            {
                OnLock();   //上锁
                ClrWdt();
                SingleOpenOperation(DEVICE_I , g_DelayTime.fenzhaTime1);
                SingleOpenOperation(DEVICE_II , g_DelayTime.fenzhaTime2);
#if(CAP3_STATE)  //判断第三块驱动是否存在
                SingleOpenOperation(DEVICE_III , g_DelayTime.fenzhaTime3);
#endif
                g_RemoteControlState.orderId = OpenAction;    //拒动错误ID号
                ClrWdt();
                g_SuddenState.executeOrder[DEVICE_I] = OPEN_STATE;
                g_SuddenState.executeOrder[DEVICE_II] = OPEN_STATE;
#if(CAP3_STATE)  //判断第三块驱动是否存在
                g_SuddenState.executeOrder[DEVICE_III] = OPEN_STATE;
#endif
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
            if(!CheckLoopCapVoltage(LOOP_ID_I))
            {
                OnLock();   //上锁
                SingleCloseOperation(DEVICE_I , g_DelayTime.hezhaTime1);
                g_SuddenState.executeOrder[DEVICE_I] = CLOSE_STATE;
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
            if(!CheckLoopCapVoltage(LOOP_ID_I))
            {
                OnLock();   //上锁
                SingleOpenOperation(DEVICE_I , g_DelayTime.fenzhaTime1);
                g_SuddenState.executeOrder[DEVICE_I] = OPEN_STATE;
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
            if(!CheckLoopCapVoltage(LOOP_ID_II))
            {
                OnLock();   //上锁
                SingleCloseOperation(DEVICE_II , g_DelayTime.hezhaTime2);
                g_SuddenState.executeOrder[DEVICE_II] = CLOSE_STATE;
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
            if(!CheckLoopCapVoltage(LOOP_ID_II))
            {
                OnLock();   //上锁
                SingleOpenOperation(DEVICE_II , g_DelayTime.fenzhaTime2);
                g_SuddenState.executeOrder[DEVICE_II] = OPEN_STATE;
                g_RemoteControlState.orderId = OpenAction;    //拒动错误ID号
            }
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }
            return 0xFF;
        }
#if(CAP3_STATE)
        case CHECK_3_HE_ORDER: //收到机构3合闸命令
        {
            ClrWdt();        
            if(!CheckLoopCapVoltage(LOOP_ID_III))  //检测电容电压
            {
                OnLock();   //上锁
                SingleCloseOperation(DEVICE_III , g_DelayTime.hezhaTime3);
                g_SuddenState.executeOrder[DEVICE_III] = CLOSE_STATE;
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
            if(!CheckLoopCapVoltage(LOOP_ID_III))  //检测电容电压
            {
                OnLock();   //上锁
                SingleOpenOperation(DEVICE_III , g_DelayTime.fenzhaTime3);
                g_SuddenState.executeOrder[DEVICE_III] = OPEN_STATE;
                g_RemoteControlState.orderId = OpenAction;    //拒动错误ID号
            }       
            else
            {
                g_Order = IDLE_ORDER;    //将命令清零
                return 0xFF;
            }     
            return 0xFF;
        }
#endif
        default:
        {
            OffLock();   //解锁
            ClrWdt();
        }
    }
    return 0;
}
/**
 * 
 * <p>Function name: [CheckswitchState]</p>
 * <p>Discription: [执行相应的指示]</p>
 */
void DsplaySwitchState(void)
{    
    uint8_t waringbuffer[LOOP_COUNT];
    uint8_t offestIndex = 3;
    
    UpdataCapVoltageState();   //更新电容电压显示状态
    
    waringbuffer[DEVICE_I] = g_SystemState.heFenState1; //机构1警告
    waringbuffer[DEVICE_II] = g_SystemState.heFenState2; //机构2警告
#if(CAP3_STATE)
    waringbuffer[DEVICE_III] = g_SystemState.heFenState3; //机构3警告
#else
    g_SystemState.heFenState3 = g_SystemState.heFenState2;  //主要应用在判断总分合位
#endif
    
    for(uint8_t index = 0; index < LOOP_COUNT; index++)
    {
        ClrWdt();
        switch (waringbuffer[index])
        {
            case ERROR_STATE:
            {
                UpdateIndicateState(CloseShowWord[index], CloseShowWord[index + offestIndex], TURN_OFF);  //关闭合位指示灯
                UpdateIndicateState(OpenShowWord[index], OpenShowWord[index + offestIndex], TURN_OFF);    //关闭分位指示灯
                UpdateIndicateState(ErrorShowWord[index], ErrorShowWord[index + offestIndex], TURN_ON);   //开启错误指示灯
                g_SystemState.warning = waringbuffer[index];
                break;
            }
            case OPEN_STATE:
            {
                UpdateIndicateState(CloseShowWord[index], CloseShowWord[index + offestIndex], TURN_OFF);  //关闭合位指示灯
                UpdateIndicateState(OpenShowWord[index], OpenShowWord[index + offestIndex], TURN_ON);     //开启分位指示灯
                UpdateIndicateState(ErrorShowWord[index], ErrorShowWord[index + offestIndex], TURN_OFF);  //关闭错误指示灯
                g_SystemState.warning = waringbuffer[index];
                if(g_SuddenState.RefuseAction == FEN_ERROR)
                {
                    g_TimeStampCollect.changeLedTime.delayTime = 500;  //正常状态下指示灯闪烁的间隔
                    g_SuddenState.RefuseAction = FALSE;
                    OffLock();
                }
                break;
            }
            case CLOSE_STATE:
            {
                UpdateIndicateState(CloseShowWord[index], CloseShowWord[index + offestIndex], TURN_ON);   //开启合位指示灯
                UpdateIndicateState(OpenShowWord[index], OpenShowWord[index + offestIndex], TURN_OFF);    //关闭分位指示灯
                UpdateIndicateState(ErrorShowWord[index], ErrorShowWord[index + offestIndex], TURN_OFF);  //关闭错误指示灯
                g_SystemState.warning = waringbuffer[index];
                if(g_SuddenState.RefuseAction == HE_ERROR)
                {
                    g_TimeStampCollect.changeLedTime.delayTime = 500;  //正常状态下指示灯闪烁的间隔
                    g_SuddenState.RefuseAction = FALSE;
                    OffLock();
                }
                break;
            }
        }
        switch(g_SuddenState.capState[index])
        {
            case CAP_UPPER_STATE:
            {
                UpdateIndicateState(ErrorShowWord[index], ErrorShowWord[index + offestIndex], TURN_ON);   //开启错误指示灯
                break;
            }
            case CAP_NORMAL_STATE:
            {
                UpdateIndicateState(CapShowWord[index], CapShowWord[index + offestIndex], TURN_ON);   //开启电容指示灯
                if(g_SystemState.warning != ERROR_STATE)
                {
                    UpdateIndicateState(ErrorShowWord[index], ErrorShowWord[index + offestIndex], TURN_OFF);  //关闭错误指示灯
                }
                break;
            }
            case CAP_LOW_STATE:
            {
                UpdateIndicateState(CapShowWord[index], CapShowWord[index + offestIndex], TURN_OFF);  //开启电容指示灯
                if(g_SystemState.warning != ERROR_STATE)
                {
                    UpdateIndicateState(ErrorShowWord[index], ErrorShowWord[index + offestIndex], TURN_OFF);  //关闭错误指示灯
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
        UpdateIndicateState(Z_FENWEI_RELAY, Z_FENWEI_LED, TURN_ON);
        UpdateIndicateState(Z_HEWEI_RELAY, Z_HEWEI_LED, TURN_OFF);
        ClrWdt();
    }
    else if((g_SystemState.heFenState2 == CLOSE_STATE)&&(g_SystemState.heFenState1 == CLOSE_STATE) && 
       (g_SystemState.heFenState3 == CLOSE_STATE)) //合闸状态
    {
        //分位指示灯亮 
        UpdateIndicateState(Z_HEWEI_RELAY, Z_HEWEI_LED, TURN_ON);
        UpdateIndicateState(Z_FENWEI_RELAY, Z_FENWEI_LED, TURN_OFF);
        ClrWdt();
    }
    else
    {
        UpdateIndicateState(Z_FENWEI_RELAY, Z_FENWEI_LED, TURN_OFF);
        UpdateIndicateState(Z_HEWEI_RELAY, Z_HEWEI_LED, TURN_OFF);
    }
}
/**
 * 
 * <p>Function name: [SwitchScan]</p>
 * <p>Discription: [检测开入量]</p>
 */
void SwitchScan(void)
{
    uint8_t index = 0;
    DigitalInputState = ReadHC165();
    ScanCount++;  //每次扫描均计数
    //远方\就地检测
    if(YUAN_BEN_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //工作\调试模式检测
    if(WORK_DEBUG_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //带电闭锁
    if(CHARGED_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构1的合位检测
    if(HEWEI1_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构1的分位检测
    if(FENWEI1_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构2的合位检测
    if(HEWEI2_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构2的分位检测
    if(FENWEI2_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
#if(CAP3_STATE)  //判断第三块驱动是否存在
    //机构3的合位检测
    if(HEWEI3_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构3的分位检测
    if(FENWEI3_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
#endif
    
    //*****************************
    //作用，屏蔽掉远方就地
    uint32_t negative = ~YUAN_INPUT;
    DigitalInputState &= negative;
    //*****************************
    ClrWdt();
    //同时合闸信号检测
    if(Z_HEZHA_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //同时分闸信号检测
    if(Z_FENZHA_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构1合闸信号检测
    if(HEZHA1_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构1分闸信号检测
    if(FENZHA1_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构2合闸信号检测
    if(HEZHA2_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构2分闸信号检测
    if(FENZHA2_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
#if(CAP3_STATE)  //判断第三块驱动是否存在
    //机构3合闸信号检测
    if(HEZHA3_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
    //机构3分闸信号检测
    if(FENZHA3_CONDITION())
    {
        ADD_VALID_COUNT(index);
    }
    index++;
#endif
    
//******************************************************************************
//时间到达后才对各个状态位进行检查
    if(ScanCount >= INPUT_FILT_TIME)
    {
        ScanCount = 0;    //Clear
        CheckAllLoopSwitchState();
        if(CheckSwitchOrder())    //收到命令后快速返回
        {
            return;
        }
        UpdateSwitchState();    //更新按键状态
    }
//******************************************************************************
    
}

/**
 * 
 * <p>Function name: [CheckAllLoopSwitchState]</p>
 * <p>Discription: [检测所有回路开关状态]</p>
 */
void CheckAllLoopSwitchState(void)
{
    uint8_t index = 0;
    //首先对远方就地进行检查
    if(DIGITAL_INPUT_EFFECTIVE(index))
    {
        g_SystemState.yuanBenState = YUAN_STATE;
    }
    else
    {
        g_SystemState.yuanBenState = BEN_STATE;
    }        
    CLEAR_VALID_COUNT(index);
    index++;
    //工作\调试模式判断
    if(DIGITAL_INPUT_EFFECTIVE(index))
    {
        g_SystemState.workMode = DEBUG_STATE;
    }
    else
    {
        g_SystemState.workMode = WORK_STATE;
    }
    CLEAR_VALID_COUNT(index);
    index++;
    //对开关是否带电进行检查
    if(DIGITAL_INPUT_EFFECTIVE(index))
    {
        g_SystemState.charged = TRUE;
    }
    else
    {
        g_SystemState.charged = FALSE;
    }
    CLEAR_VALID_COUNT(index);
    index++;
    //对机构1的合分位进行检查
    if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_INVALID(index + 1))
    {
        g_SystemState.heFenState1 = CLOSE_STATE;  
    }
    else if(DIGITAL_INPUT_EFFECTIVE(index + 1) && DIGITAL_INPUT_INVALID(index))
    {
        g_SystemState.heFenState1 = OPEN_STATE;  
    }
    else    //此时存在错误
    {            
        g_SystemState.heFenState1 = ERROR_STATE;     //合分位同时存在\不存在    
    }
    CLEAR_VALID_COUNT(index);
    index ++;
    CLEAR_VALID_COUNT(index);
    index ++;

    //对机构2的合分位进行检查
    if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_INVALID(index + 1))
    {
        g_SystemState.heFenState2 = CLOSE_STATE;  
    }
    else if(DIGITAL_INPUT_EFFECTIVE(index + 1) && DIGITAL_INPUT_INVALID(index))
    {
        g_SystemState.heFenState2 = OPEN_STATE;  
    }
    else    //此时存在错误
    {            
        g_SystemState.heFenState2 = ERROR_STATE;     //合分位同时存在
    }
    CLEAR_VALID_COUNT(index);
    index ++;
    CLEAR_VALID_COUNT(index);
    index ++;

#if(CAP3_STATE)  //判断第三块驱动是否存在
    //对机构3的合分位进行检查
    if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_INVALID(index + 1))
    {
        g_SystemState.heFenState3 = CLOSE_STATE;       
    }
    else if(DIGITAL_INPUT_EFFECTIVE(index + 1) && DIGITAL_INPUT_INVALID(index))
    {
        g_SystemState.heFenState3 = OPEN_STATE;     
    }
    else    //此时存在错误
    {            
        g_SystemState.heFenState3 = ERROR_STATE;     //合分位同时存在
    }
    CLEAR_VALID_COUNT(index);
    index ++;
    CLEAR_VALID_COUNT(index);
    index ++;
#else
    g_SystemState.heFenState3 = g_SystemState.heFenState2;
#endif
}

/**
 * 
 * <p>Function name: [CheckSwitchOrder]</p>
 * <p>Discription: [检测按键命令]</p>
 * @return 0 无命令
 */
uint8_t CheckSwitchOrder(void)
{
#if(CAP3_STATE)  //判断第三块驱动是否存在
    uint8_t index = 9;
#else
    uint8_t index = 7;
#endif
    if(CheckLockState())    //首先判断是否正在执行合闸或者分闸操作
    {
        //对总合总分信号检测        
        if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_EFFECTIVE(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            g_SystemState.heFenState1 = ERROR_STATE;
            g_SystemState.heFenState2 = ERROR_STATE;
#if(CAP3_STATE)  //判断第三块驱动是否存在
            g_SystemState.heFenState3 = ERROR_STATE;
#endif
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_INVALID(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if((g_SystemState.heFenState1 == OPEN_STATE) && 
               (g_SystemState.heFenState2 == OPEN_STATE) && 
               (g_SystemState.heFenState3 == OPEN_STATE))
            {
                g_Order = HE_ORDER;     //同时合闸命令
            }
            return g_Order;
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index + 1) && DIGITAL_INPUT_INVALID(index))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if((g_SystemState.heFenState1 == CLOSE_STATE) && 
               (g_SystemState.heFenState2 == CLOSE_STATE) && 
               (g_SystemState.heFenState3 == CLOSE_STATE) )
            {
                g_Order = FEN_ORDER;     //同时分闸命令
            }
            return g_Order;
        }
        index += 2;
        
        //对机构1的合分信号检测     
        if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_EFFECTIVE(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            g_SystemState.heFenState1 = ERROR_STATE;
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_INVALID(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if(g_SystemState.heFenState1 == OPEN_STATE)
            {
                g_Order = CHECK_1_HE_ORDER;     //同时合闸命令
            }
            return g_Order;
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index + 1) && DIGITAL_INPUT_INVALID(index))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if(g_SystemState.heFenState1 == CLOSE_STATE)
            {
                g_Order = CHECK_1_FEN_ORDER;     //同时分闸命令
            }
            return g_Order;
        }
        index += 2;
        
        //对机构2的合分信号检测     
        if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_EFFECTIVE(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            g_SystemState.heFenState2 = ERROR_STATE;
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_INVALID(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if(g_SystemState.heFenState2 == OPEN_STATE)
            {
                g_Order = CHECK_2_HE_ORDER;     //同时合闸命令
            }
            return g_Order;
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index + 1) && DIGITAL_INPUT_INVALID(index))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if(g_SystemState.heFenState2 == CLOSE_STATE)
            {
                g_Order = CHECK_2_FEN_ORDER;     //同时分闸命令
            }
            return g_Order;
        }
        index += 2;
        
        //对机构3的合分信号检测   
#if(CAP3_STATE)  //判断第三块驱动是否存在  
        if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_EFFECTIVE(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            g_SystemState.heFenState3 = ERROR_STATE;
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index) && DIGITAL_INPUT_INVALID(index + 1))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if(g_SystemState.heFenState3 == OPEN_STATE)
            {
                g_Order = CHECK_3_HE_ORDER;     //同时合闸命令
            }
            return g_Order;
        }
        else if(DIGITAL_INPUT_EFFECTIVE(index + 1) && DIGITAL_INPUT_INVALID(index))
        {
            CLEAR_VALID_COUNT(index);
            CLEAR_VALID_COUNT(index + 1);
            if(g_SystemState.heFenState3 == CLOSE_STATE)
            {
                g_Order = CHECK_3_FEN_ORDER;     //同时分闸命令
            }
            return g_Order;
        }
#endif
    }
    return 0;
}
/**
 * 
 * <p>Function name: [UpdateSwitchState]</p>
 * <p>Discription: [更新更新开关分合位状态]</p>
 */
void UpdateSwitchState(void)
{
    if(g_LastSwitchState[DEVICE_I] != g_SystemState.heFenState1)
    {
        g_SuddenState.switchState[DEVICE_I] = g_SystemState.heFenState1;
        g_LastSwitchState[DEVICE_I] = g_SuddenState.switchState[DEVICE_I];
        g_SuddenState.suddenFlag = TRUE;    //状态发生突变
        g_SuddenState.switchsuddenFlag = TRUE;  //开关分合位状态突变
    }
    if(g_LastSwitchState[DEVICE_II] != g_SystemState.heFenState2)
    {
        g_SuddenState.switchState[DEVICE_II] = g_SystemState.heFenState2;
        g_LastSwitchState[DEVICE_II] = g_SuddenState.switchState[DEVICE_II];
        g_SuddenState.suddenFlag = TRUE;    //状态发生突变
        g_SuddenState.switchsuddenFlag = TRUE;  //开关分合位状态突变
    }
#if(CAP3_STATE)
    if(g_LastSwitchState[DEVICE_III] != g_SystemState.heFenState3)
    {
        g_SuddenState.switchState[DEVICE_III] = g_SystemState.heFenState3;
        g_LastSwitchState[DEVICE_III] = g_SuddenState.switchState[DEVICE_III];
        g_SuddenState.suddenFlag = TRUE;    //状态发生突变
        g_SuddenState.switchsuddenFlag = TRUE;  //开关分合位状态突变
    }
#endif
}

