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

uint8_t g_LastcapState[LOOP_QUANTITY] = {0};   //获取上一次电容电压状态

void GetcapState(uint8_t loop, float min, float max, float capVale);
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
 * <p>Function name: [GetCapVolatageState]</p>
 * <p>Discription: [获取电压状态]</p>
 * @return 电压状态，大于最小值为0xAAAA
 */
uint16_t GetCapVolatageState(void)
{
    GetCapVoltage();
    ClrWdt();
    if ((g_SystemVoltageParameter.voltageCap1  >= g_SystemLimit.capVoltage1.down + THRESHOLD_VALUE) && 
        (g_SystemVoltageParameter.voltageCap2  >= g_SystemLimit.capVoltage2.down + THRESHOLD_VALUE) && 
        CHECK_VOLTAGE_CAP3())
    {
        return 0xAAAA;
    }
    else
    {
        return 0;
    }
}

/**
 * 
 * <p>Function name: [UpdataCapVoltageState]</p>
 * <p>Discription: [更新电容指示灯和继电器]</p>
 */
void UpdataCapVoltageState(void)
{
    GetcapState(LOOP_A, g_SystemLimit.capVoltage1.down, g_SystemLimit.capVoltage1.upper, g_SystemVoltageParameter.voltageCap1);
    GetcapState(LOOP_B, g_SystemLimit.capVoltage2.down, g_SystemLimit.capVoltage2.upper, g_SystemVoltageParameter.voltageCap2);
    if(g_LastcapState[LOOP_A] != g_SuddenState.capState[LOOP_A])
    {
        g_SuddenState.suddenFlag = TRUE;    //发送突发状态
        g_SuddenState.capSuddentFlag = TRUE;
    }
    if(g_LastcapState[LOOP_B] != g_SuddenState.capState[LOOP_B])
    {
        g_SuddenState.suddenFlag = TRUE;    //发送突发状态
        g_SuddenState.capSuddentFlag = TRUE;
    }
#if(CAP3_STATE)
    GetcapState(LOOP_C, g_SystemLimit.capVoltage3.down, g_SystemLimit.capVoltage3.upper, g_SystemVoltageParameter.voltageCap3);
    if(g_LastcapState[LOOP_C] != g_SuddenState.capState[LOOP_C])
    {
        g_SuddenState.suddenFlag = TRUE;    //发送突发状态
        g_SuddenState.capSuddentFlag = TRUE;
    }
#endif
}
/**
 * 
 * <p>Function name: [GetcapState]</p>
 * <p>Discription: [获取电容状态]</p>
 * @param loop  回路数
 * @param min   电容电压最小值
 * @param max   电容电压最大值
 * @param capVale   电容电压值
 */
void GetcapState(uint8_t loop, float min, float max, float capVale)
{
    float minValue = min + THRESHOLD_VALUE;
    
    if (capVale  >= max)    //超过上限
    {
        g_SuddenState.capState[loop] = CAP_UPPER_STATE; //超压状态
    }
    else if((capVale < max) && (capVale >= minValue))
    {
        g_SuddenState.capState[loop] = CAP_NORMAL_STATE; //正常状态
    }
    else if(capVale <= min)
    {
        g_SuddenState.capState[loop] = CAP_LOW_STATE; //欠压状态
    }
    else
    {
        g_SuddenState.capState[loop] = 0;   //错误状态
    }
}
/**
 * 
 * <p>Function name: [ReadCapDropVoltage]</p>
 * <p>Discription: [读取在执行合闸或者分闸后电容电压的变化]</p>
 */
void ReadCapDropVoltage(void)
{
    SoftSampleOnce();
    ClrWdt();
    g_SystemVoltageParameter.capDropVoltage1 = ADCBUF1 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient1;
    g_SystemVoltageParameter.capDropVoltage2 = ADCBUF2 * LOCAL_CAP_MODULUS * g_SystemCalibrationCoefficient.capVoltageCoefficient2;
    CAP3_DROP_VOLTAGE();
}
