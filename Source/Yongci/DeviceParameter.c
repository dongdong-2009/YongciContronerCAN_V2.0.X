/** 
 * <p>application name： DeviceParameter.c</p> 
 * <p>application describing： 电容电压读取</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */
#include "../Header.h"
#include "../Driver/AdcSample.h"
#include "DeviceParameter.h"
#include "../SerialPort/RefParameter.h"

#define MARGIN_VALUE    2

typedef struct TagVoltageColect
{
    float32_t valueA;
    float32_t valueB;
    float32_t valueC;
    float32_t valueW;
}VoltageColect;

typedef struct TagAdcFilter
{
    VoltageColect sum;
    VoltageColect max;
    VoltageColect min;
    VoltageColect value;
}AdcFilter;

/**
 * 
 * <p>Function name: [GetCapVoltage]</p>
 * <p>Discription: [软件启动转换，获取ADC值.]</p>
 */
void GetCapVoltage(void)
{
    uint8_t i = 0;
    uint8_t count = 5;
    AdcFilter adcFilter;
    /* Clear */
    adcFilter.sum.valueA = 0;
    adcFilter.sum.valueB = 0;
    adcFilter.sum.valueC = 0;
    adcFilter.sum.valueW = 0;
    
    SoftSampleOnce();
    adcFilter.value.valueW = ADCBUF0 * ADC_MODULUS;
    adcFilter.max.valueW = adcFilter.value.valueW ;
    adcFilter.min.valueW = adcFilter.value.valueW;
    adcFilter.sum.valueW += adcFilter.value.valueW;

    adcFilter.value.valueA = ADCBUF1 * LOCAL_CAP_MODULUS;
    adcFilter.max.valueA = adcFilter.value.valueA;
    adcFilter.min.valueA = adcFilter.value.valueA;
    adcFilter.sum.valueA += adcFilter.value.valueA;

    adcFilter.value.valueB = ADCBUF2 * LOCAL_CAP_MODULUS;
    adcFilter.max.valueB = adcFilter.value.valueB;
    adcFilter.min.valueB = adcFilter.value.valueB;
    adcFilter.sum.valueB += adcFilter.value.valueB;
#if(CAP3_STATE)
    adcFilter.value.valueC = ADCBUF3 * LOCAL_CAP_MODULUS;
    adcFilter.max.valueC = adcFilter.value.valueC;
    adcFilter.min.valueC = adcFilter.value.valueC;
    adcFilter.sum.valueC += adcFilter.value.valueC;
#endif
    for(i = 0; i < count; i++)
    {
        SoftSampleOnce();
        adcFilter.value.valueW = ADCBUF0 * ADC_MODULUS;
        adcFilter.sum.valueW += adcFilter.value.valueW;

        adcFilter.value.valueA = ADCBUF1 * LOCAL_CAP_MODULUS;
        adcFilter.sum.valueA += adcFilter.value.valueA;

        adcFilter.value.valueB = ADCBUF2 * LOCAL_CAP_MODULUS;
        adcFilter.sum.valueB += adcFilter.value.valueB;
        ClrWdt();
        
        /* get workvoltage min value*/
        if(adcFilter.min.valueW > adcFilter.value.valueW)
        {
            adcFilter.min.valueW = adcFilter.value.valueW;
        }
        /* get capAvoltage min value*/
        if(adcFilter.min.valueA > adcFilter.value.valueA)
        {
            adcFilter.min.valueA = adcFilter.value.valueA;
        }
        /* get capBvoltage min value*/
        if(adcFilter.min.valueW > adcFilter.value.valueB)
        {
            adcFilter.min.valueW = adcFilter.value.valueB;
        }
        ClrWdt();
        
        /* get workvoltage max value*/
        if(adcFilter.max.valueW < adcFilter.value.valueW)
        {
            adcFilter.max.valueW = adcFilter.value.valueW;
        }
        /* get capAvoltage max value*/
        if(adcFilter.max.valueA < adcFilter.value.valueA)
        {
            adcFilter.max.valueA = adcFilter.value.valueA;
        }
        /* get capBvoltage max value*/
        if(adcFilter.max.valueB < adcFilter.value.valueB)
        {
            adcFilter.max.valueB = adcFilter.value.valueB;
        }
#if(CAP3_STATE)
        adcFilter.value.valueC = ADCBUF3 * LOCAL_CAP_MODULUS;
        adcFilter.sum.valueC += adcFilter.value.valueC;
        /* get capCvoltage min value*/
        if(adcFilter.min.valueC > adcFilter.value.valueC)
        {
            adcFilter.min.valueC = adcFilter.value.valueC;
        }
        /* get capCvoltage max value*/
        if(adcFilter.max.valueC < adcFilter.value.valueC)
        {
            adcFilter.max.valueC = adcFilter.value.valueC;
        }
#endif
        ClrWdt();
    }
    g_SystemVoltageParameter.workVoltage = (adcFilter.sum.valueW - adcFilter.max.valueW - adcFilter.min.valueW) / (count - 1);
    g_SystemVoltageParameter.voltageCap1 = (adcFilter.sum.valueA - adcFilter.max.valueA - adcFilter.min.valueA) / (count - 1);
    g_SystemVoltageParameter.voltageCap2 = (adcFilter.sum.valueB - adcFilter.max.valueB - adcFilter.min.valueB) / (count - 1);    
    #if(CAP3_STATE)
    {
        g_SystemVoltageParameter.voltageCap3 = (adcFilter.sum.valueC - adcFilter.max.valueC - adcFilter.min.valueC) / (count - 1);
    }
    #else
    {
        g_SystemVoltageParameter.voltageCap3 = 225;
    }
    #endif
    ClrWdt();
}

/**
 * 
 * <p>Function name: [GetCapVolatageState]</p>
 * <p>Discription: [获取电压状态]</p>
 * @return 电压状态，大于最小值为0xAAAA
 */
uint16_t GetCapVolatageState(void)
{
    GetCapVoltage();
    ClrWdt();
    if ((g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down) && 
        (g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down) && 
        CHECK_VOLTAGE_CAP3())
    {
        ClrWdt();        
        ClrWdt();
        return 0xAAAA;
    }
    else
    {
        return 0;
    }
}

/**
 * 
 * <p>Function name: [CheckVoltage]</p>
 * <p>Discription: [检测电压的状态,并更新指示灯和继电器]</p>
 */
void UpdataVoltageState(void)
{
    ClrWdt();
    //机构1电容状态更新
    if ((g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.upper) && 
        (g_SuddenState.CapState1 != 0x03)) //电压超过上限
    {
        if(g_SuddenState.SuddenFlag == FALSE)
        {
            g_SuddenState.Cap1Error = CAP1_ERROR;
            g_SuddenState.CapState1 = 0x03;    //0b11
            g_SuddenState.SuddenFlag = TRUE;   //该状态属于突发状态
            return ;
        }
    }
    else if((g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down) && 
            (g_SystemVoltageParameter.voltageCap1  < g_SystemLimit.capVoltage1.upper) && 
            (g_SuddenState.CapState1 != 0x02)) //正常电压
    {
        ClrWdt();
        g_SuddenState.Cap1Error = NO_ERROR;
        g_SuddenState.SuddenFlag = TRUE;
        UpdateIndicateState(CAP1_RELAY , CAP1_LED ,TURN_ON);     
        g_SuddenState.CapState1 = 0x02;    //0b10
    }
    else if((g_SystemVoltageParameter.voltageCap1  < g_SystemLimit.capVoltage1.down - MARGIN_VALUE) && 
            (g_SuddenState.CapState1 != 0x01)) //低电压
    {
        ClrWdt();
        UpdateIndicateState(CAP1_RELAY , CAP1_LED ,TURN_OFF); 
        g_SuddenState.CapState1 = 0x01;    //0b01
    }
    
    //机构2电容状态更新
    if ((g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.upper) && 
        (g_SuddenState.CapState2 != 0x0C)) //电压超过上限
    {
        if(g_SuddenState.SuddenFlag == FALSE)
        {
            g_SuddenState.Cap2Error = CAP2_ERROR;
            g_SuddenState.CapState2 = 0x0C;    //0b11
            g_SuddenState.SuddenFlag = TRUE;   //该状态属于突发状态
            return ;
        }
    }
    else if((g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down) && 
            (g_SystemVoltageParameter.voltageCap2  < g_SystemLimit.capVoltage2.upper) && 
            (g_SuddenState.CapState2 != 0x08)) //正常电压
    {
        ClrWdt();
        g_SuddenState.Cap2Error = NO_ERROR;
        g_SuddenState.SuddenFlag = TRUE;
        UpdateIndicateState(CAP2_RELAY , CAP2_LED ,TURN_ON);      
        g_SuddenState.CapState2 = 0x08;    //0b10 
    }
    else if((g_SystemVoltageParameter.voltageCap2  < g_SystemLimit.capVoltage2.down - MARGIN_VALUE) && 
            (g_SuddenState.CapState2 != 0x04)) //低电压
    {
        ClrWdt();
        UpdateIndicateState(CAP2_RELAY , CAP2_LED ,TURN_OFF);      
        g_SuddenState.CapState2 = 0x04;    //0b01 
    }
    
    //机构3电容状态更新
    #if(CAP3_STATE)
    {
    if ((g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.upper) && 
        (g_SuddenState.CapState3 != 0x30)) //电压超过上限
    {
        if(g_SuddenState.SuddenFlag == FALSE)
        {
            g_SuddenState.Cap3Error = CAP3_ERROR;
            g_SuddenState.CapState3 = 0x30;    //0b11
            g_SuddenState.SuddenFlag = TRUE;   //该状态属于突发状态
            return ;
        }
    }
    else if((g_SystemVoltageParameter.voltageCap3  >= g_SystemLimit.capVoltage3.down) && 
            (g_SystemVoltageParameter.voltageCap3  < g_SystemLimit.capVoltage3.upper) && 
            (g_SuddenState.CapState3 != 0x20)) //正常电压
    {
        ClrWdt();
        g_SuddenState.Cap3Error = NO_ERROR;
        g_SuddenState.SuddenFlag = TRUE;
        UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_ON);   
        g_SuddenState.CapState3 = 0x20;    //0b10      
    }
    else if((g_SystemVoltageParameter.voltageCap3  < g_SystemLimit.capVoltage3.down - MARGIN_VALUE) && 
            (g_SuddenState.CapState3 != 0x10)) //低电压
    {
        ClrWdt();
        UpdateIndicateState(CAP3_RELAY , CAP3_LED ,TURN_OFF);  
        g_SuddenState.CapState3 = 0x10;    //0b01 
    }
    }
    #endif
}

/**
 * 
 * <p>Function name: [ReadCapDropVoltage]</p>
 * <p>Discription: [读取在执行合闸或者分闸后电容电压的变化]</p>
 * @param lastOrder 上一次执行的命令
 */
void ReadCapDropVoltage(uint16_t lastOrder)
{
    //远方与就地命令的转换
    if((lastOrder == IDLE_ORDER) && (g_RemoteControlState.lastReceiveOrder != IDLE_ORDER))  //远端遥控与就地遥控
    {
        lastOrder = g_RemoteControlState.lastReceiveOrder;
    }
    switch(lastOrder)
    {
        case CHECK_Z_FEN_ORDER:
        case CHECK_Z_HE_ORDER:
        {
            SoftSampleOnce();
            ClrWdt();
            g_SystemVoltageParameter.capDropVoltage1 = ADCBUF1 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient1;
            g_SystemVoltageParameter.capDropVoltage2 = ADCBUF2 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient2;
            CAP3_DROP_VOLTAGE();
            break;
        }
        case CHECK_1_FEN_ORDER:
        case CHECK_1_HE_ORDER:
        {
            SoftSampleOnce();
            ClrWdt();
            g_SystemVoltageParameter.capDropVoltage1 = ADCBUF1 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient1;
            break;
        }
        case CHECK_2_FEN_ORDER:
        case CHECK_2_HE_ORDER:
        {
            SoftSampleOnce();
            ClrWdt();
            g_SystemVoltageParameter.capDropVoltage2 = ADCBUF2 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient2;
            break;
        }
        case CHECK_3_FEN_ORDER:
        case CHECK_3_HE_ORDER:
        {
            #if(CAP3_STATE)
            {
                SoftSampleOnce();
                ClrWdt();
                CAP3_DROP_VOLTAGE();
            }
            #endif
            break;
        }
        default:
        {
            return;
        }
    }
}
