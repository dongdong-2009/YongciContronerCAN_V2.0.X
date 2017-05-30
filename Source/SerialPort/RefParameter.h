/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/** 
 * <p>application name： RefParameter.h</p> 
 * <p>application describing： RefParameter宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef PROJECTHEADER_REFPARAMETER_H_
#define	PROJECTHEADER_REFPARAMETER_H_

#include <xc.h> // include processor files - each processor file is guarded.  
#include "../Driver/tydef.h"
#include "../DeviceNet/DeviceNet.h"
// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 
    
typedef struct TagPointUint8
{
    uint8* pData; //数据指针
    uint8 len; //指向数据长度
}PointUint8;

/**
 * 过程延时时间定义
 */
typedef struct FenHeZhaDelayTime
{
	uint8 hezhaTime1;   //合闸时间1 (ms) 
	uint8 hezhaTime2;   //合闸时间2 (ms) 
	uint8 hezhaTime3;   //合闸时间3 (ms) 
    uint8 fenzhaTime1;  //分闸时间1 (ms)
    uint8 fenzhaTime2;  //分闸时间2 (ms)
    uint8 fenzhaTime3;  //分闸时间3 (ms)
}WaitTime;

/**
 * 校准系数
 */
typedef struct TagSystemCalibrationCoefficient
{
	float capVoltageCoefficient1;   //默认用于电容1电压校准系数
	float capVoltageCoefficient2;   //默认用于电容2电压校准系数
	float capVoltageCoefficient3;   //默认用于电容3电压校准系数

}SystemCalibrationCoefficient;

/**
 * 电容电压参数
 */
typedef struct TagSystemVoltageParameter
{
	float voltageCap1;  //用于电容电压1
	float voltageCap2;  //用于电容电压2
	float voltageCap3;  //用于电容电压3

	float workVoltage; //工作电压
    
    float temp; //温度

}SystemVoltageParameter;

/**
 * 上下限值
 */
typedef struct TagUpDownValue
{
	float upper;//上限
	float down;//下限

}UpDownValue;

/**
 * 系统限定参数值
 */
typedef struct TagLimitValue
{
	UpDownValue capVoltage1;    //电容1电压
	UpDownValue capVoltage2;    //电容2电压
	UpDownValue capVoltage3;    //电容3电压
	UpDownValue hezhaVoltage1;  //合闸1电压（只有下限）
	UpDownValue fenzhaVoltage1; //分闸1电压（只有下限）
	UpDownValue hezhaVoltage2;  //合闸2电压（只有下限）
	UpDownValue fenzhaVoltage2; //分闸2电压（只有下限）
	UpDownValue hezhaVoltage3;  //合闸3电压（只有下限）
	UpDownValue fenzhaVoltage3; //分闸3电压（只有下限）
	UpDownValue workVoltage;    //工作电压
    
}LimitValue;

/**
 *同步命令
 */
typedef struct TagSyncCommand
{
	uint8 commandByte[8]; //命令字节数组
	uint8 configbyte; //配置字
	uint16 actionRadA; //A   设定弧度归一化值r = M/65536 *2*PI
	uint16 actionRadB; //B
	uint16 actionRadC; //C
}SyncCommand;


/**
 * 配置数据
 */
typedef struct TagConfigData
{
    uint8 ID;       //ID号
    void* pData;    //指向数据
    uint8 type;     //类型
    void (*fSetValue)(PointUint8*, struct TagConfigData* );
    void (*fGetValue)(PointUint8*, struct TagConfigData* );
}ConfigData;

/**
 * 分合闸次数
 */
typedef struct TagActionCount
{
    uint16 hezhaCount1;     //合闸次数1
    uint16 fenzhaCount1;    //分闸次数1
    uint16 hezhaCount2;     //合闸次数2
    uint16 fenzhaCount2;    //分闸次数2
    uint16 hezhaCount3;     //合闸次数3
    uint16 fenzhaCount3;    //分闸次数3
}ActionCount;

/**
 * 系统状态量
 */
typedef struct TagSystemState
{
    uint8 heFenState1;   //处于合位还是分为
    uint8 heFenState2;   //处于合位还是分为
    uint8 heFenState3;   //处于合位还是分为
    uint8 yuanBenState; //远方或者就地
    uint8 workMode;     //工作模式
    uint8 warning;      //告警
    uint8 MacID;        //MAC地址
    
}SystemState;


/**
 * 供外部调用的全局变量，且均为读取变量
 */
extern SystemVoltageParameter g_SystemVoltageParameter; //系统电压
extern LimitValue g_SystemLimit;
extern ActionCount g_ActionCount;   //分合闸计数
extern SystemState g_SystemState;   //系统状态量
extern WaitTime g_DelayTime;        //分合闸时间
extern SystemCalibrationCoefficient g_SystemCalibrationCoefficient; //系统参数校准

extern uint16 _PERSISTENT g_SyncReadyWaitTime;  //同步预制等待时间
extern uint16 _PERSISTENT g_RemoteWaitTime;     //遥控预制等待时间

uint8 SetParamValue(uint8 id,PointUint8* pPoint);
uint8 ReadParamValue(uint8 id,PointUint8* pPoint);
void RefParameterInit(void);
void WriteAccumulateSum(void);
uint8 AccumulateSumVerify(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

