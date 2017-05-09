/*****************88
 * 摘要:
 * 2015/8/10:完善远本转换。
 * 2015/11/17: 改变IO检测模式，由纯延时形式，变成采样点样式。
 * 将远方检测IO与本地合并。
***/
#include "SwtichCondition.h"
#include "../Header.h"
#include "DeviceParameter.h"

#define DELAY_MS    20

#define STATE_HE_ZHA_Z  0x5505  //同时合闸命令
#define STATE_FEN_ZHA_Z 0x550A  //同时分闸命令

#define STATE_HE_ZHA_1  0x5515  //机构1合闸命令
#define STATE_FEN_ZHA_1 0x551A  //机构1分闸命令

#define STATE_HE_ZHA_2  0x5525  //机构2合闸命令
#define STATE_FEN_ZHA_2 0x552A  //机构2分闸命令

#define STATE_HE_ZHA_3  0x5535  //机构3合闸命令
#define STATE_FEN_ZHA_3 0x553A  //机构3分闸命令

/**
 * <p>Discription: [检测机构出现错误的命令]</p>
 */
#define STATE_ERROR_1   0xAB00  //机构1错误
#define STATE_ERROR_2   0xAB11  //机构2错误
#define STATE_ERROR_3   0xAB22  //机构3错误

#define STATE_ERROR_4   0xAB33  //机构1错误，同时输入了合闸、分闸信号
#define STATE_ERROR_5   0xAB44  //机构2错误，同时输入了合闸、分闸信号
#define STATE_ERROR_6   0xAB55  //机构3错误，同时输入了合闸、分闸信号

#define STATE_ERROR_7   0xAB66  //同时输入了总的合闸、分闸信号

#define STATE_FEN_CONTINOUS        0xAA55

#define COIL1_HEZHA()   (HZHA1_INPUT | FENWEI1_INPUT)   //机构1合闸条件
#define COIL1_FENZHA()  (FZHA1_INPUT | HEWEI1_INPUT)    //机构1分闸条件

#define COIL2_HEZHA()   (HZHA2_INPUT | FENWEI2_INPUT)   //机构2合闸条件
#define COIL2_FENZHA()  (FZHA2_INPUT | HEWEI2_INPUT)    //机构2分闸条件
    
uint32 g_kairuValue = 0;    //165返回值

//本地的总合闸条件：总分位 && 总合闸信号 && 电压满足 && 本地控制 && 调试模式 && 不带电
#define Z_HEZHA_CONDITION()    \
    ((g_kairuValue == (Z_HEZHA_INPUT | FENWEI1_INPUT | FENWEI2_INPUT)) &&     \
    (g_SystemVoltageParameter.voltageCap1 >= g_SystemLimit.capVoltage1.down) &&     \
    (g_SystemVoltageParameter.voltageCap1 >= g_SystemLimit.capVoltage2.down))

//本地的总分闸条件：总合位 && 总分闸信号 && 电压满足 && 本地控制 && 调试模式 && 不带电
#define Z_FENZHA_CONDITION()    \
    ((g_kairuValue == (Z_FENZHA_INPUT | HEWEI1_INPUT | HEWEI2_INPUT)) &&     \
    (g_SystemVoltageParameter.voltageCap1 >= g_SystemLimit.capVoltage1.down) &&     \
    (g_SystemVoltageParameter.voltageCap1 >= g_SystemLimit.capVoltage2.down))

//本地的机构1合闸条件：分位1 && 合闸1信号 && 电压1满足 && 本地控制 && 调试模式 && 不带电
#define HEZHA1_CONDITION()      \
    (((g_kairuValue & (COIL1_HEZHA() | YUAN_AND_WORK())) == COIL1_HEZHA()) &&     \
    (g_SystemVoltageParameter.voltageCap1 >= g_SystemLimit.capVoltage1.down))
//本地的机构1分闸条件：合位1 && 分闸1信号 && 电压1满足 && 本地控制 && 调试模式 && 不带电
#define FENZHA1_CONDITION()      \
    (((g_kairuValue & (COIL1_FENZHA() | YUAN_AND_WORK())) == COIL1_FENZHA()) &&     \
    (g_SystemVoltageParameter.voltageCap1 >= g_SystemLimit.capVoltage1.down))

//本地的机构2合闸条件：分位2 && 合闸2信号 && 电压2满足 && 本地控制 && 调试模式 && 不带电
#define HEZHA2_CONDITION()      \
    (((g_kairuValue & (COIL2_HEZHA() | YUAN_AND_WORK())) == COIL2_HEZHA()) &&     \
    (g_SystemVoltageParameter.voltageCap2 >= g_SystemLimit.capVoltage2.down))
//本地的机构2分闸条件：合位2 && 分闸2信号 && 电压2满足 && 本地控制 && 调试模式 && 不带电
#define FENZHA2_CONDITION()      \
    (((g_kairuValue & (COIL2_FENZHA() | YUAN_AND_WORK())) == COIL2_FENZHA()) &&    \
    (g_SystemVoltageParameter.voltageCap2 >= g_SystemLimit.capVoltage2.down))

/**
 * <p>Discription: [一下错误均为不可屏蔽掉的错误，且错误严重]</p>
 */
//***************************************************************************************************
//本地错误条件: 合位和分位同时成立 或 同时不成立
#define ERROR1_CONDITION()      \
        (((g_kairuValue & (FENWEI1_INPUT | HEWEI1_INPUT)) == (FENWEI1_INPUT | HEWEI1_INPUT)) ||   \
        ((g_kairuValue & (FENWEI1_INPUT | HEWEI1_INPUT)) == 0))

//本地错误条件: 合位和分位同时成立 或 同时不成立
#define ERROR2_CONDITION()      \
        (((g_kairuValue & (FENWEI2_INPUT | HEWEI2_INPUT)) == (FENWEI2_INPUT | HEWEI2_INPUT)) ||   \
        ((g_kairuValue & (FENWEI2_INPUT | HEWEI2_INPUT)) == 0))

//本地错误条件: 合位和分位同时成立 或 同时不成立
#define ERROR3_CONDITION()      \
        ((g_kairuValue & (FENWEI3_INPUT | HEWEI3_INPUT)) == (FENWEI3_INPUT | HEWEI3_INPUT) ||   \
        ((g_kairuValue & (FENWEI3_INPUT | HEWEI3_INPUT)) == 0))

//本地错误条件: 机构1同时输入了合分闸信号
#define ERROR4_CONDITION()  ((g_kairuValue & (HZHA1_INPUT | FZHA1_INPUT)) == (HZHA1_INPUT | FZHA1_INPUT))

//本地错误条件: 机构2同时输入了合分闸信号
#define ERROR5_CONDITION()  ((g_kairuValue & (HZHA2_INPUT | FZHA2_INPUT)) == (HZHA2_INPUT | FZHA2_INPUT))

//本地错误条件: 机构3同时输入了合分闸信号
#define ERROR6_CONDITION()  ((g_kairuValue & (HZHA3_INPUT | FZHA3_INPUT)) == (HZHA3_INPUT | FZHA3_INPUT))

//本地错误条件: 同时输入了合分闸信号
#define ERROR7_CONDITION()  ((g_kairuValue & (Z_HEZHA_INPUT | Z_FENZHA_INPUT)) == (Z_HEZHA_INPUT | Z_FENZHA_INPUT))
//***************************************************************************************************

#define FENZHA_CONDITION_CONTINOUS() (g_kairuValue == 0)

/**
 * 
 * <p>Function name: [GetSwitchState]</p>
 * <p>Discription: [获取开关状态]</p>
 * @param state 需要检测的状态
 * @return 开关状态
 */
uint16 GetSwitchState(uint16 state)
{
    ClrWdt();
    g_kairuValue = ReHC74165(); //获取按键值
    switch (state)
    {
        //整体合闸
        case STATE_HE_ZHA_Z:
        {
            return  Z_HEZHA_CONDITION();
            break;
        }
        //整体分闸
        case STATE_FEN_ZHA_Z:
        {
            return  Z_FENZHA_CONDITION();
            break;
        }
        //机构1合闸
        case STATE_HE_ZHA_1:
        {
            return HEZHA1_CONDITION();
            break;
        }
        //机构1分闸
        case STATE_FEN_ZHA_1:
        {
            return FENZHA1_CONDITION();
            break;
        }
        //机构2合闸
        case STATE_HE_ZHA_2:
        {
            return HEZHA2_CONDITION();
            break;
        }
        //机构2分闸
        case STATE_FEN_ZHA_2:
        {
            return FENZHA2_CONDITION();
            break;
        }
        
        case STATE_ERROR_1:
        {
            return ERROR1_CONDITION();
        }
        
        case STATE_ERROR_2:
        {
            return ERROR2_CONDITION();
        }
        
        case STATE_ERROR_4:
        {
            return ERROR4_CONDITION();
        }
        
        case STATE_ERROR_5:
        {
            return ERROR5_CONDITION();
        }
        
        case STATE_ERROR_7:
        {
            return ERROR7_CONDITION();
        }
        
        case STATE_FEN_CONTINOUS:
        {
            return FENZHA_CONDITION_CONTINOUS();
        }
        default : //命令错误
        {
            Reset(); //执行复位
        }
    }
    return 0;
    
}

/**
 * 
 * <p>Function name: [StateCheck]</p>
 * <p>Discription: [获取开关状态]</p>
 * @param state 执行的状态
 * @return 指令有效返回0xFFFF，无效返回0
 */
uint16 StateCheck(uint16 state)
{
    uint16 delay = DELAY_MS * 10;
    uint16 trueCn = 0;
    uint16 falseCn = 0;
    uint16 i = 0;
    ClrWdt();
    if (GetSwitchState(state))
    {
        trueCn = 0;
        falseCn = 0;
        for ( i = 0 ; i < 100; i++)//100 * DELAY_MS * 10 = 2ms 实际大于2ms的循环
        {
            __delay_us(delay); 
            ClrWdt();
            if (GetSwitchState(state))
            {
                trueCn++;
            }
            else
            {
                falseCn++;
            }
            if (falseCn > 10)
            {
                break; //停止检测
            }
        } 
        if (trueCn >= 90) //大于90次则认为指令有效 90%的采样满足要求
        {
            ClrWdt();
            return 0xFFFF;
        }
    }
    return 0;
}
/**
 * 
 * <p>Function name: [CheckSwitchAction]</p>
 * <p>Discription: [可能存在如下几种情况：
 * 1.故障合位检测与分位检测同时有信号或者无信号，合闸按钮或者
 * 分闸按钮同时有信号，此时判断为故障状态。
 * 2.在合闸状态下，仅能接收分闸指令；分闸状态下，仅能接收合闸指令。]</p>
 * @return 返回各个相对应的状态量
 */
uint8 CheckSwitchAction(void) 
{    
    ClrWdt();    
    //机构同时有合分闸信号输入
    while(StateCheck(STATE_ERROR_7))
    {
        g_SystemState.heFenState1 = CHECK_ERROR4_STATE;
        UpdateIndicateState(ERROR1_RELAY,ERROR1_LED,TURN_ON);
        UpdateIndicateState(ERROR2_RELAY,ERROR2_LED,TURN_ON);
        UpdateIndicateState(ERROR3_RELAY,ERROR3_LED,TURN_ON);
        __delay_ms(200);
        UpdateLEDIndicateState(ERROR1_LED,TURN_OFF);
        UpdateLEDIndicateState(ERROR2_LED,TURN_OFF);
        UpdateLEDIndicateState(ERROR3_LED,TURN_OFF);
        __delay_ms(200);
    }
    //机构1同时有合分闸信号输入
    while(StateCheck(STATE_ERROR_4))
    {
        g_SystemState.heFenState1 = CHECK_ERROR4_STATE;
        UpdateIndicateState(ERROR1_RELAY,ERROR1_LED,TURN_ON);
        __delay_ms(200);
        UpdateLEDIndicateState(ERROR1_LED,TURN_OFF);
        __delay_ms(200);
    }    
    //机构2同时有合分闸信号输入
    while(StateCheck(STATE_ERROR_5))
    {
        g_SystemState.heFenState1 = CHECK_ERROR5_STATE;
        UpdateIndicateState(ERROR2_RELAY,ERROR2_LED,TURN_ON);
        __delay_ms(200);
        UpdateLEDIndicateState(ERROR2_LED,TURN_OFF);
        __delay_ms(200);
    }
    
    //机构1合位与分位同时检测 或者 都没检测到
    if (StateCheck(STATE_ERROR_1))
    {
        g_SystemState.heFenState1 = CHECK_ERROR1_STATE;
        ClrWdt();
    }
    else
    {
        g_SystemState.heFenState1 = NO_ERROR;
    }
    
    //机构2合位与分位同时检测 或者 都没检测到
    if (StateCheck(STATE_ERROR_2))
    {
        ClrWdt();
        g_SystemState.heFenState2 = CHECK_ERROR2_STATE;
    }
    else
    {
        g_SystemState.heFenState2 = NO_ERROR;
    }
    
    if((g_SystemState.heFenState1 == CHECK_ERROR1_STATE) || (g_SystemState.heFenState2 == CHECK_ERROR2_STATE))
    {
        g_SystemState.warning = CHECK_ERROR_STATE;
        return CHECK_ERROR_STATE;
    }
    //无错误则关闭错误继电器和指示灯
    UpdateIndicateState(ERROR1_RELAY,ERROR1_LED,TURN_OFF);
    UpdateIndicateState(ERROR2_RELAY,ERROR2_LED,TURN_OFF);
    UpdateIndicateState(ERROR3_RELAY,ERROR3_LED,TURN_OFF);
    g_SystemState.warning = NO_ERROR;
    
    //在以上均不成立时 检测合分闸按钮状态
    
    //总的合分闸检测
    //在合位时检测 分闸 按钮
    ClrWdt();
    if (StateCheck(STATE_FEN_ZHA_Z))
    {
        ClrWdt();
//        FenOnLock();    //上锁
        return CHECK_Z_FEN_ORDER;//返回分闸命令
    }
    ClrWdt();
    ClrWdt();
    // 在分位时检测 合闸 按钮 且不是 欠压状态
    if (StateCheck(STATE_HE_ZHA_Z))
    {
        ClrWdt();
//        HeOnLock();//上锁
        return CHECK_Z_HE_ORDER;//返回分闸命令
    }
    ClrWdt();
    
    //机构1的合分闸检测
    //在合位时检测 分闸 按钮
    ClrWdt();
    if (StateCheck(STATE_FEN_ZHA_1))
    {
        ClrWdt();
//        FenOnLock();    //上锁
        return CHECK_1_FEN_ORDER;//返回分闸命令
    }
    ClrWdt();
    ClrWdt();
    // 在分位时检测 合闸 按钮 且不是 欠压状态
    if (StateCheck(STATE_HE_ZHA_1))
    {
        ClrWdt();
//        HeOnLock();//上锁
        return CHECK_1_HE_ORDER;//返回分闸命令
    }
    
    //机构2的合分闸检测
    //在合位时检测 分闸 按钮
    ClrWdt();
    if (StateCheck(STATE_FEN_ZHA_2))
    {
        ClrWdt();
//        FenOnLock();    //上锁
        return CHECK_2_FEN_ORDER;//返回分闸命令
    }
    ClrWdt();
    ClrWdt();
    // 在分位时检测 合闸 按钮 且不是 欠压状态
    if (StateCheck(STATE_HE_ZHA_2))
    {
        ClrWdt();
//        HeOnLock();//上锁
        return CHECK_2_HE_ORDER;//返回分闸命令
    }
    
    
    //错状态退出
    return  0;
} 
/**
 * 
 * <p>Function name: [CheckVoltage]</p>
 * <p>Discription: [检测电压的状态]</p>
 */
void CheckVoltage(void)
{
    GetCapVoltage();
    ClrWdt();
    if (g_SystemVoltageParameter.voltageCap1  >= LOW_VOLTAGE_ADC)
    {
        UpdateIndicateState(CAP1_RELAY , CAP1_LED ,TURN_ON);
    }
    else
    {
        UpdateIndicateState(CAP1_RELAY , CAP1_LED ,TURN_OFF);        
    }
    if (g_SystemVoltageParameter.voltageCap2  >= LOW_VOLTAGE_ADC)
    {
        UpdateIndicateState(CAP2_RELAY , CAP2_LED ,TURN_ON);
    }
    else
    {
        UpdateIndicateState(CAP2_RELAY , CAP2_LED ,TURN_OFF);        
    }
}
/**
 * 
 * <p>Function name: [CheckIOState]</p>
 * <p>Discription: [检测IO状态，并更新状态显示]</p>
 * @return 接收到分合闸命令返回0xFF,否则返回0
 */
uint8 CheckIOState(void)
{
    uint8 checkOrder = 0;
    
    //***********************首先检测按钮IO*******开始****************//
    //远控本地切换
    ClrWdt();   
    checkOrder = CheckSwitchAction();  
    ClrWdt();           
    switch (checkOrder)
    {
        case CHECK_ERROR_STATE:  //检测到错误
        {
            ClrWdt();      
            checkOrder = 0;
            break;
        }
        case CHECK_Z_HE_ORDER: //收到合闸命令
        {
            ClrWdt();           
            UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_ON);
//            HeZhaActionA();
//            return 0xff;
            return 0;
        }
        case CHECK_Z_FEN_ORDER: //收到分闸命令
        {
            ClrWdt();
            UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_OFF);
//            FenZhaActionA();
//            return 0xff;
            return 0;
        }
        
        case CHECK_1_HE_ORDER: //收到机构1合闸命令
        {
            ClrWdt();           
            UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_ON);
//            HeZhaActionA();
//            return 0xff;
            return 0;
        }
        case CHECK_1_FEN_ORDER: //收到机构1分闸命令
        {
            ClrWdt();
            UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_OFF);
//            FenZhaActionA();
//            return 0xff;
            return 0;
        }
        
        case CHECK_2_HE_ORDER: //收到机构2合闸命令
        {
            ClrWdt();           
            UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_ON);
//            HeZhaActionA();
//            return 0xff;
            return 0;
        }
        case CHECK_2_FEN_ORDER: //收到机构2分闸命令
        {
            ClrWdt();
            UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_OFF);
//            FenZhaActionA();
//            return 0xff;
            return 0;
        }
        default:
        {
            ClrWdt();
            checkOrder = 0;
        }
    }
    return 0;
}
/**
 * 
 * <p>Function name: [CheckSwitchState]</p>
 * <p>Discription: [检测开关状态]</p>
 */
void CheckSwitchState(void)
{
    ClrWdt();
    if(g_SystemState.heFenState1 == CHECK_ERROR1_STATE) //机构1错误
    {
        UpdateIndicateState(ERROR1_RELAY,ERROR1_LED,TURN_ON);
        UpdateIndicateState(HEWEI1_RELAY,HEWEI1_LED,TURN_OFF);
        UpdateIndicateState(FENWEI1_RELAY,FENWEI1_LED,TURN_OFF);
        ClrWdt();
    }
    else 
    {
        UpdateIndicateState(ERROR1_RELAY,ERROR1_LED,TURN_OFF);
    }
    if(g_SystemState.heFenState2 == CHECK_ERROR2_STATE) //机构2错误
    {
        UpdateIndicateState(ERROR2_RELAY,ERROR2_LED,TURN_ON);
        UpdateIndicateState(HEWEI2_RELAY,HEWEI2_LED,TURN_OFF);
        UpdateIndicateState(FENWEI2_RELAY,FENWEI2_LED,TURN_OFF);
        ClrWdt();
    }
    else
    {
        UpdateIndicateState(ERROR2_RELAY,ERROR2_LED,TURN_OFF);
    }
    
    if((g_SystemState.heFenState1 == CHECK_ERROR1_STATE) || 
       (g_SystemState.heFenState2 == CHECK_ERROR2_STATE))
    {
        return ;
    }
    g_kairuValue = ReHC74165();
    
    //以上既不是故障 也不是 合分闸命令情况下 检测合分状态，检测合分位时不考虑过多的因素
    //机构1的分合位检测    
    ClrWdt();
    if((g_kairuValue & FENWEI1_INPUT) == FENWEI1_INPUT) //分闸状态
    {
        UpdateIndicateState(FENWEI1_RELAY,FENWEI1_LED,TURN_ON);
        UpdateIndicateState(HEWEI1_RELAY,HEWEI1_LED,TURN_OFF);
        ClrWdt();
        g_SystemState.heFenState1 = CHECK_1_FEN_STATE;
    }
    else if((g_kairuValue & HEWEI1_INPUT) == HEWEI1_INPUT)  //合闸状态
    {
        UpdateIndicateState(HEWEI1_RELAY,HEWEI1_LED,TURN_ON);
        UpdateIndicateState(FENWEI1_RELAY,FENWEI1_LED,TURN_OFF);
        ClrWdt();
        g_SystemState.heFenState1 = CHECK_1_HE_STATE;
    }
    
    //机构2的分合位检测    
    ClrWdt();
    if((g_kairuValue & FENWEI2_INPUT) == FENWEI2_INPUT) //分闸状态
    {
        UpdateIndicateState(FENWEI2_RELAY,FENWEI2_LED,TURN_ON);
        UpdateIndicateState(HEWEI2_RELAY,HEWEI2_LED,TURN_OFF);
        ClrWdt();
        g_SystemState.heFenState2 = CHECK_2_FEN_STATE;
    }    
    else if((g_kairuValue & HEWEI2_INPUT) == HEWEI2_INPUT)   //合闸状态
    {
        UpdateIndicateState(HEWEI2_RELAY,HEWEI2_LED,TURN_ON);
        UpdateIndicateState(FENWEI2_RELAY,FENWEI2_LED,TURN_OFF);
        ClrWdt();
        g_SystemState.heFenState2 = CHECK_2_HE_STATE;
    }
    
    //总的分合位检测    
    //分闸状态
    ClrWdt();
    if((g_SystemState.heFenState2 == CHECK_2_FEN_STATE)&&(g_SystemState.heFenState1 == CHECK_1_FEN_STATE))
    {
        //合位指示灯亮 
        UpdateIndicateState(Z_FENWEI_RELAY,Z_FENWEI_LED,TURN_ON);
        UpdateIndicateState(Z_HEWEI_RELAY,Z_HEWEI_LED,TURN_OFF);
    }
    else if((g_SystemState.heFenState2 == CHECK_2_HE_STATE)&&(g_SystemState.heFenState1 == CHECK_1_HE_STATE)) //合闸状态
    {
        //分位指示灯亮 
        UpdateIndicateState(Z_HEWEI_RELAY,Z_HEWEI_LED,TURN_ON);
        UpdateIndicateState(Z_FENWEI_RELAY,Z_FENWEI_LED,TURN_OFF);
    }
    else
    {
        UpdateIndicateState(Z_FENWEI_RELAY,Z_FENWEI_LED,TURN_OFF);
        UpdateIndicateState(Z_HEWEI_RELAY,Z_HEWEI_LED,TURN_OFF);
    }
    
}

//暂时不对其做任何处理
/**
 * 
 * <p>Function name: [ContinuousCheck]</p>
 * <p>Discription: [用于合分闸动作完成后的立刻检测。]</p>
 * @param lastOrder 上一次执行的命令
 * @return  返回分合闸指令，否则返回0
 */
uint8 ContinuousCheck(uint16* lastOrder)
{
    uint16 i = 0;
    ClrWdt();
    if (g_SystemState.yuanBenState == YUAN_STATE)//远控
    {
        return 0;
    }
    if (HE_ORDER == *lastOrder)//刚执行完合闸命令
    {
        ClrWdt();
        g_kairuValue = ReHC74165();
        if ((HZHA1_INPUT & g_kairuValue) == 0)//合闸信号已经消失
        {
            ClrWdt();
            *lastOrder = IDLE_ORDER;
            return 0;
        }
        else
        {
            i = 0;
            g_kairuValue = ReHC74165();
            while((HZHA1_INPUT & g_kairuValue) == 1)    //合闸命令一直存在
            {
                g_kairuValue = ReHC74165();
                ClrWdt();
                //分闸条件 1处于合位 2电压足够 3有效的分闸信号 
                if (StateCheck(STATE_FEN_CONTINOUS))
                {
                    //返回分闸命令
                    ClrWdt();
                    //返回分闸命令
                    ClrWdt();
                    FenOnLock();//上锁
                    return CHECK_Z_FEN_ORDER;
                }
                ClrWdt();
                __delay_ms(1);
                i++;
                //持续3s认为是错误，进行错误处理
                if (i > 3000)
                {
                    
                }
            }
            ClrWdt();
            *lastOrder = IDLE_ORDER;
            return 0;
        }
    }
    ClrWdt();
    if (FEN_ORDER == *lastOrder)//刚执行完分闸命令
    {
        //等待信号结束。
        i = 0;
        g_kairuValue = ReHC74165();
        while(g_kairuValue & FZHA1_INPUT)
        {
            g_kairuValue = ReHC74165();
            ClrWdt();
            __delay_ms(1);
            ClrWdt();
            i++;
            //持续3s认为是错误，进行错误处理
            if (i > 3000)
            {
                
            }
        }
        ClrWdt();
        //若此时有合闸信号，则认为为分闸信号未结束时就有 不执行动作
        //等待信号结束。
        i = 0;
        g_kairuValue = ReHC74165();
        while(g_kairuValue & HZHA1_INPUT)
        {
            g_kairuValue = ReHC74165();
            ClrWdt();
            __delay_ms(1);
            ClrWdt();
            i++;
            //持续3s认为是错误，进行错误处理
            if (i > 3000)
            {
                
            }
        }
        *lastOrder = IDLE_ORDER;
        return 0;
    }
    *lastOrder = IDLE_ORDER;
    return 0;
    
}

