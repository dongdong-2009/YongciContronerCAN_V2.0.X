/**
 * @file DeviceParameter.c
 * @brief 电容电压读取
 * copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.
 * company： SOJO
 * @date 2017.06.5
 *
 * @author Zhangxiaomou
 * @version ver 1.0
 */
#include "../Header.h"
#include "../Driver/AdcSample.h"
#include "DeviceParameter.h"
#include "../SerialPort/RefParameter.h"

#define THRESHOLD_VALUE    3   //V

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

uint8_t g_LastcapState[LOOP_COUNT] = {0};   //获取上一次电容电压状态

uint8_t CheckSingleLoopCapVoltage(UpDownValue* value, float capVale);
/**
 * 
 * @fn GetCapVoltage
 * @brief 软件启动转换，获取ADC值.
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
    
    g_SystemVoltageParameter.voltageCap1 *= g_SystemCalibrationCoefficient.capVoltageCoefficient1;
    g_SystemVoltageParameter.voltageCap2 *= g_SystemCalibrationCoefficient.capVoltageCoefficient2;    
#if(CAP3_STATE)
    g_SystemVoltageParameter.voltageCap3 = (adcFilter.sum.valueC - adcFilter.max.valueC - adcFilter.min.valueC) / (count - 1);
    g_SystemVoltageParameter.voltageCap3 *= g_SystemCalibrationCoefficient.capVoltageCoefficient3;  
#else
    g_SystemVoltageParameter.voltageCap3 = 0;
#endif
    ClrWdt();
}

/**
 * 
 * @fn CheckLoopCapVoltage
 * @brief 获取电压状态
 * @param loop 回路数
 * @return 电压状态，正常为0
 */
uint8_t CheckLoopCapVoltage(uint8_t loop)
{
    uint8_t i = 0;
    GetCapVoltage();
    g_SuddenState.capState[DEVICE_I] = CheckSingleLoopCapVoltage(&g_SystemLimit.capVoltage1, g_SystemVoltageParameter.voltageCap1);
    g_SuddenState.capState[DEVICE_II] = CheckSingleLoopCapVoltage(&g_SystemLimit.capVoltage2, g_SystemVoltageParameter.voltageCap2);
#if(CAP3_STATE)
    g_SuddenState.capState[DEVICE_III] = CheckSingleLoopCapVoltage(&g_SystemLimit.capVoltage3, g_SystemVoltageParameter.voltageCap3);
#endif
    for(i = 0; i < LOOP_COUNT; i++)
    {
        if(loop & (1 << i))
        {
            if(g_SuddenState.capState[i] != CAP_NORMAL_STATE)
            {
                return CAPVOLTAGE_ERROR;
            }
        }
    }
    return 0;
}

/**
 * 
 * @fn UpdataCapVoltageState
 * @brief 更新电容指示灯和继电器
 */
void UpdataCapVoltageState(void)
{
    GetCapVoltage();
    g_SuddenState.capState[DEVICE_I] = CheckSingleLoopCapVoltage(&g_SystemLimit.capVoltage1, g_SystemVoltageParameter.voltageCap1);
    g_SuddenState.capState[DEVICE_II] = CheckSingleLoopCapVoltage(&g_SystemLimit.capVoltage2, g_SystemVoltageParameter.voltageCap2);
    if(g_LastcapState[DEVICE_I] != g_SuddenState.capState[DEVICE_I])
    {
        g_LastcapState[DEVICE_I] = g_SuddenState.capState[DEVICE_I];
        g_SuddenState.suddenFlag = TRUE;    //发送突发状态
        g_SuddenState.capSuddentFlag = TRUE;
    }
    if(g_LastcapState[DEVICE_II] != g_SuddenState.capState[DEVICE_II])
    {
        g_LastcapState[DEVICE_II] = g_SuddenState.capState[DEVICE_II];
        g_SuddenState.suddenFlag = TRUE;    //发送突发状态
        g_SuddenState.capSuddentFlag = TRUE;
    }
#if(CAP3_STATE)
    g_SuddenState.capState[DEVICE_III] = CheckSingleLoopCapVoltage(&g_SystemLimit.capVoltage3, g_SystemVoltageParameter.voltageCap3);
    if(g_LastcapState[DEVICE_III] != g_SuddenState.capState[DEVICE_III])
    {
        g_LastcapState[DEVICE_III] = g_SuddenState.capState[DEVICE_III];
        g_SuddenState.suddenFlag = TRUE;    //发送突发状态
        g_SuddenState.capSuddentFlag = TRUE;
    }
#endif
}

/**
 * 
 * @fn CheckSingleLoopCapVoltage
 * @brief 检测电容电压状态
 * @param value 极值
 * @param capVale 电容电压
 * @return 返回电容状态
 */
uint8_t CheckSingleLoopCapVoltage(UpDownValue* value, float capVale)
{
    float minValue = value->down + THRESHOLD_VALUE;
    
    if (capVale  > value->upper)    //超过上限
    {
        return CAP_UPPER_STATE; //超压状态
    }
    else if((capVale <= value->upper) && (capVale >= minValue))
    {
        return CAP_NORMAL_STATE; //正常状态
    }
    else if(capVale <= value->down)
    {
        return CAP_LOW_STATE; //欠压状态
    }
    else    //在死区中
    {
        return 0;   //错误状态
    }
}
/**
 * 
 * @fn ReadCapDropVoltage
 * @brief 读取在执行合闸或者分闸后电容电压的变化
 */
void ReadCapDropVoltage(void)
{
    SoftSampleOnce();
    ClrWdt();
    g_SystemVoltageParameter.capDropVoltage1 = ADCBUF1 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient1;
    g_SystemVoltageParameter.capDropVoltage2 = ADCBUF2 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient2;
    CAP3_DROP_VOLTAGE();
}
