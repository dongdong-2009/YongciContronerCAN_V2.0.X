/** 
 * <p>application name： RefParameter.c</p> 
 * <p>application describing： 用来存放计算使用的各种数据</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[2017-04-07] [张东旭][更改其内容为适应永磁控制器程序]
 * @author ZhangXiaomou 
 * @version ver 1.0
 */
#include "../Header.h"
#include "RefParameter.h"
#include "../DeviceNet/DeviceNet.h"

/**
 * EEPROM地址
 */
#define EEPROM_STAR_ADDRESS 0x7FF100    //EEPROM起始地址
#define EEPROM_END_ADDRESS  0x7FFFFE    //EEPROM结束地址
#define EEPROM_OFFSET_ADDRESS 0x04      //EEPROM偏移地址
#define ACCUMULATE_SUM_ADDRESS 0x7FFFFC   //累加和EEPROM地址

/*
 * 局部函数定义
 */
void SetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig);
void GetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig);
void GetValueFloatInt32(PointUint8* pPoint, ConfigData* pConfig);
void SetValueFloatInt32(PointUint8* pPoint, ConfigData* pConfig);
void GetValueUint16(PointUint8* pPoint, ConfigData* pConfig);
void SetValueUint16(PointUint8* pPoint, ConfigData* pConfig);
void SetValueUint8(PointUint8* pPoint, ConfigData* pConfig);
void GetValueUint8(PointUint8* pPoint, ConfigData* pConfig);

void ParameterWriteByID(uint8_t id,PointUint8* pPoint);
void ParameterReadByID(uint8_t id,PointUint8* pPoint);


/**
 * 分合闸延时时间
 */
WaitTime g_DelayTime;
/**
 * 系统电容电压有关参数
 */
SystemVoltageParameter g_SystemVoltageParameter;
/**
 * 系统参数校准
 */
SystemCalibrationCoefficient g_SystemCalibrationCoefficient;

/**
 *系统参数上下限
 */
LimitValue g_SystemLimit;

/**
 * 合分闸计数
 */
ActionCount g_ActionCount;

/**
 * 系统固有量
 */
SystemState g_SystemState;
/**
 *同步预制等待时间,单位ms
 */
uint16_t _PERSISTENT g_SyncReadyWaitTime;
/**
 * 遥控预制等待时间
 */
uint16_t _PERSISTENT g_RemoteWaitTime;   
/**
 * 本地系统时钟
 */
uint8_t g_LocalMac;

/**
 * 默认同步命令
 */
SyncCommand g_SyncCommand;

/**
 * 内部所需的宏定义，主要包括ID号初始值、参数列表长度
 */
#define PARAMETER_LEN 27  //设置参数列表
#define READONLY_PARAMETER_LEN 21  //只读参数列表
#define SET_START_ID 0x01   //设置参数ID号开始值
#define READONLY_START_ID 0x41  //只读参数ID号开始值
/**
 *系统配置参数合集
 */
ConfigData g_SetParameterCollect[PARAMETER_LEN]; //配置参数列表--可读可写
ConfigData g_ReadOnlyParameterCollect[READONLY_PARAMETER_LEN]; //参数合集--只读列表
/**
 * 
 */
PointUint8 BufferData;
/**
 * 设置参数
 * @param id      配置号
 * @param pPoint  指向保存数据的指针
 *
 * @return 错误代码 0-没有错误 非0-有错误
 */
uint8_t SetParamValue(uint8_t id,PointUint8* pPoint)
{    
	for(uint8_t i = 0; i < PARAMETER_LEN; i++)
	{
		ClrWdt();
		if(g_SetParameterCollect[i].ID == id)
		{
			//TODO :添加错误处理——每一个Set/Get函数添加相应的处理内容。
			g_SetParameterCollect[i].fSetValue(pPoint, g_SetParameterCollect + i);
			if (pPoint->len == 0)
			{
				return 0xF1;
			}
            
//            BufferData.len = pPoint->len;
//            ParameterWriteByID(id , pPoint);   //写EEPROM           
//            ClrWdt();
//            ParameterReadByID(id , &BufferData);      //读取EEPROM
            //添加判断读取出来的数据是否与变换之后的一致性
            ClrWdt();
            g_SetParameterCollect[i].fGetValue(pPoint, g_SetParameterCollect + i);
            
            ClrWdt();
			if (pPoint->len == 0)
			{
				return 0xF2;
			}
            
			return 0;
		}

	}
	return 0xFF;
}
/**
 * 读取参数
 * @param id      配置号
 * @param pPoint  指向保存数据的指针
 *
 * @return 错误代码 0-没有错误 非0-有错误
 */
uint8_t ReadParamValue(uint8_t id,PointUint8* pPoint)
{
	if (id < READONLY_START_ID) //小于只读ID，可为配置参数类型
	{
		for(uint8_t i = 0; i < PARAMETER_LEN; i++)
		{
			ClrWdt();
			if(g_SetParameterCollect[i].ID == id)
			{
				ClrWdt();
				//TODO :添加错误处理——每一个Get函数添加相应的处理内容。

				g_SetParameterCollect[i].fGetValue(pPoint, g_SetParameterCollect + i);
				if (pPoint->len == 0)
				{
					return 0xF1;
				}
				return 0;
			}

		}
		return 0xF2;
	}
	else
	{
		for(uint8_t i = 0; i < READONLY_PARAMETER_LEN; i++)
		{
			ClrWdt();
			if(g_ReadOnlyParameterCollect[i].ID == id)
			{
				//TODO :添加错误处理——每一个et函数添加相应的处理内容。
                ClrWdt();
				g_ReadOnlyParameterCollect[i].fGetValue(pPoint, g_ReadOnlyParameterCollect + i);
				if (pPoint->len == 0)
				{
					return 0xF3;
				}
				return 0;
			}
		}
		return 0xF4;
	}
	return 0xFF;
}

/**
 * 初始化系统参数合集
 */
void InitSetParameterCollect(void)
{
	uint8_t index = 0, id = SET_START_ID;
    //uint16 -- 电容电压、工作电压上下限
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.capVoltage1.upper;  //电容电压1上限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
    ClrWdt();
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.capVoltage1.down;   //电容电压1下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.hezhaVoltage1.down; //合闸电压1下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.fenzhaVoltage1.down;//分闸电压1下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.capVoltage2.upper;
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.capVoltage2.down;
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.hezhaVoltage2.down; //合闸电压2下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.fenzhaVoltage2.down; //分闸电压2下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.capVoltage3.upper;
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.capVoltage3.down;
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.hezhaVoltage3.down; //合闸电压3下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.fenzhaVoltage3.down; //分闸电压3下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.workVoltage.upper;  //工作电压上限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemLimit.workVoltage.down;   //工作电压下限
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	//Uint16 -- 电容电压校准系数
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.capVoltageCoefficient1;
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index ++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.capVoltageCoefficient2;
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemCalibrationCoefficient.capVoltageCoefficient3;
	g_SetParameterCollect[index].type = 0x22;
	g_SetParameterCollect[index].fSetValue = SetValueFloatUint16;
	g_SetParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	ClrWdt();
	//Uint16 --[0-65535]，单位ms
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_RemoteWaitTime;  //遥控预制等待时间
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SyncReadyWaitTime;  //同步预制等待时间
	g_SetParameterCollect[index].type = 0x20;
	g_SetParameterCollect[index].fSetValue = SetValueUint16;
	g_SetParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	ClrWdt();
	//Uint8 --[0-255]，单位ms    
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_DelayTime.hezhaTime1;  //合闸时间1
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_DelayTime.fenzhaTime1;  //分闸时间1
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_DelayTime.hezhaTime2;  //合闸时间2
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_DelayTime.fenzhaTime2;  //分闸时间2
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_DelayTime.hezhaTime3;  //合闸时间3
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_DelayTime.fenzhaTime3;  //分闸时间3
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	ClrWdt();
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_SystemState.MacID;  //MAC地址
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_SetParameterCollect[index].ID = id++;
	g_SetParameterCollect[index].pData = &g_LocalMac;   //系统时钟
	g_SetParameterCollect[index].type = 0x10;
	g_SetParameterCollect[index].fSetValue = SetValueUint8;
	g_SetParameterCollect[index].fGetValue = GetValueUint8;
	index++;
    
}

/**
 * 初始化只读系统参数合集
 */
void InitReadonlyParameterCollect(void)
{
	uint8_t index = 0, id = READONLY_START_ID;
	ClrWdt();
    //uint16 -- 电容电压、工作电压
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.voltageCap1;
	g_ReadOnlyParameterCollect[index].type = 0x22;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.voltageCap2;
	g_ReadOnlyParameterCollect[index].type = 0x22;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	ClrWdt();
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.voltageCap3;
	g_ReadOnlyParameterCollect[index].type = 0x22;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.workVoltage;
	g_ReadOnlyParameterCollect[index].type = 0x22;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.temp;   //温度
	g_ReadOnlyParameterCollect[index].type = 0x22;  
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatInt32;
	index++;
	ClrWdt();
    //uint16 -- 合分闸计数
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ActionCount.hezhaCount1;   //合闸次数1
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ActionCount.fenzhaCount1;   //分闸次数1
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	ClrWdt();
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ActionCount.hezhaCount2;   //合闸次数2
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ActionCount.fenzhaCount2;   //分闸次数2
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	ClrWdt();
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ActionCount.hezhaCount3;   //合闸次数3
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_ActionCount.fenzhaCount3;   //分闸次数3
	g_ReadOnlyParameterCollect[index].type = 0x20;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint16;
	index++;
	ClrWdt();
    //uint8 -- 系统状态位    
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemState.heFenState1;   //合位分位1
	g_ReadOnlyParameterCollect[index].type = 0x10;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemState.heFenState2;   //合位分位2
	g_ReadOnlyParameterCollect[index].type = 0x10;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	ClrWdt();
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemState.heFenState3;   //合位分位3
	g_ReadOnlyParameterCollect[index].type = 0x10;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemState.yuanBenState;   //远方就地
	g_ReadOnlyParameterCollect[index].type = 0x10;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	ClrWdt();
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemState.workMode;   //工作模式
	g_ReadOnlyParameterCollect[index].type = 0x10;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemState.warning;   //告警
	g_ReadOnlyParameterCollect[index].type = 0x10;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	ClrWdt();
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemState.MacID;   //MAC
	g_ReadOnlyParameterCollect[index].type = 0x10;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueUint8;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.capDropVoltage1;    //电容1跌落电压
	g_ReadOnlyParameterCollect[index].type = 0x22;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.capDropVoltage2;    //电容2跌落电压
	g_ReadOnlyParameterCollect[index].type = 0x22;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
	g_ReadOnlyParameterCollect[index].ID = id++;
	g_ReadOnlyParameterCollect[index].pData = &g_SystemVoltageParameter.capDropVoltage3;    //电容3跌落电压
	g_ReadOnlyParameterCollect[index].type = 0x22;
	g_ReadOnlyParameterCollect[index].fSetValue = 0;
	g_ReadOnlyParameterCollect[index].fGetValue = GetValueFloatUint16;
	index++;
    
}

/**
 * 初始化全局变量参数
 */
void RefParameterInit(void)
{
    uint8_t error = 0;
    uint8_t buffer[8] = {0,0,0,0,0,0,0,0};
    
    BufferData.pData = buffer;
    BufferData.len = 8;
    
    //合分闸次数初始化
	ClrWdt();
    ReadActionCount(JG1_HE_COUNT_ADDRESS, &g_ActionCount.hezhaCount1);
    ReadActionCount(JG1_FEN_COUNT_ADDRESS, &g_ActionCount.fenzhaCount1);
    ReadActionCount(JG2_HE_COUNT_ADDRESS, &g_ActionCount.hezhaCount2);
    ReadActionCount(JG2_FEN_COUNT_ADDRESS, &g_ActionCount.fenzhaCount2);
	
#if(CAP3_STATE)
    ReadActionCount(JG3_HE_COUNT_ADDRESS , &g_ActionCount.hezhaCount3);
    ReadActionCount(JG3_FEN_COUNT_ADDRESS , &g_ActionCount.fenzhaCount3);
#else
    g_ActionCount.hezhaCount3 = 0;  //实际需要读取EEPROM
    g_ActionCount.fenzhaCount3 = 0;  //实际需要读取EEPROM
#endif  
    
    //系统电容电压初始化
    g_SystemVoltageParameter.workVoltage = 0;   //实际需要读取ADC值
    g_SystemVoltageParameter.temp = DS18B20GetTemperature();
	ClrWdt();
    
    //系统状态值初始化
    g_SystemState.heFenState1 = 0x00;
    g_SystemState.heFenState2 = 0x00;
    g_SystemState.heFenState3 = 0x00;
	ClrWdt();
    
    g_SystemState.yuanBenState = 0x00;  //远方本地初始化
    g_SystemState.workMode = 0x00;      //工作模式，默认值
    g_SystemState.warning = 0x00;       //默认无警告
    g_SystemState.charged = 0;          //默认不带电
    
    InitSetParameterCollect();
    InitReadonlyParameterCollect();    
	ClrWdt();
    
    error = AccumulateSumVerify();  //累加和校验    
    if(error)
    {
        g_TimeStampCollect.changeLedTime.delayTime = 100;
        //系统参数上下限
        g_SystemLimit.workVoltage.upper = 6.0f;
        g_SystemLimit.workVoltage.down =  2.7f;
        ClrWdt();

        g_SystemLimit.capVoltage1.upper = 240.0f;
        g_SystemLimit.capVoltage2.upper = 241.0f;
        g_SystemLimit.capVoltage2.upper = 242.0f;

        g_SystemLimit.capVoltage1.down = 180.0f;
        g_SystemLimit.capVoltage2.down = 181.1f;
        g_SystemLimit.capVoltage2.down = 182.0f;
        
        g_SystemCalibrationCoefficient.capVoltageCoefficient1 = 1;
        g_SystemCalibrationCoefficient.capVoltageCoefficient2 = 1;
        g_SystemCalibrationCoefficient.capVoltageCoefficient3 = 1;
        
        g_DelayTime.hezhaTime1 = 50;
        g_DelayTime.hezhaTime2 = 50;
        g_DelayTime.hezhaTime3 = 50;
        
        g_DelayTime.fenzhaTime1 = 30;
        g_DelayTime.fenzhaTime2 = 30;
        g_DelayTime.fenzhaTime3 = 30;
        
        //同步预制等待时间
        g_SyncReadyWaitTime = 3000;
        g_RemoteWaitTime = 3000;
        g_LocalMac = 0x0A;
    }    
}

/**
 * 设置参数[0,655.35]，默认保留2位小数
 * 适用于电压频率,工作电压，
 * <p>
 * 以float存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
void SetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
		//Todo: 更具ID选择保留有效位数
		float32_t ration = 0.01f;
		uint16_t data = pPoint->pData[1];
		data =  (data << 8) | pPoint->pData[0];
		ClrWdt();
		float32_t result = (float32_t)data * ration;
		*(float32_t*)pConfig->pData = result;
		ClrWdt();
		pPoint->len = 2;
	}
	else
	{
		ClrWdt();
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 获取参数[0,655.35]，默认保留2位小数
 * 适用于电压频率,工作电压，
 * <p>
 * 以float存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
void GetValueFloatUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
        float32_t ration = 100.0f;
        uint16_t result = (uint16_t)(*(float*)pConfig->pData * ration);
		pPoint->pData[0] = (uint8_t)(result & 0x00FF);
		ClrWdt();
		pPoint->pData[1] = (uint8_t)(result >> 8);
		pPoint->len = 2;
	}
	else
	{
		ClrWdt();
		pPoint->len = 0; //置为0，以示意错误
	}
}


/**
 * 
 * <p>Function name: [GetValueFloatInt16]</p>
 * <p>Discription: [获取参数，带符号]</p>
 * @param pPoint    指向数据数组
 * @param pConfig   指向当前配置数据
 */
void SetValueFloatInt32(PointUint8* pPoint, ConfigData* pConfig)
{
    PointFloat data;
    uint8_t i = 0;
    if (pPoint->len >= 2)
	{
        for(i = 0;i < 4;i++)
        {
            data.uData[i] = pPoint->pData[i];
        }
        
        *(float*)pConfig->pData = data.fdata;
		pPoint->len = 4;
	}
	else
	{
		ClrWdt();
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 
 * <p>Function name: [GetValueFloatInt16]</p>
 * <p>Discription: [获取参数，带符号]</p>
 * @param pPoint    指向数据数组
 * @param pConfig   指向当前配置数据
 */
void GetValueFloatInt32(PointUint8* pPoint, ConfigData* pConfig)
{
    PointFloat data;
    uint8_t i = 0;
    if (pPoint->len >= 2)
	{
        data.fdata = *(float*)pConfig->pData;
        for(i = 0;i < 4;i++)
        {
            pPoint->pData[i] = data.uData[i];
            ClrWdt();
        }
		pPoint->len = 4;
	}
	else
	{
		ClrWdt();
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 设置值，针对2字节[0,65535]
 * 适用于电压采样延时,传输延时，合闸时间，同步预制等待时间，
 * <p>
 * 以uint16_t形式存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
void SetValueUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
		pPoint->len = 2;
		uint16_t data = pPoint->pData[1];
		ClrWdt();
		data = (data<<8) | pPoint->pData[0];
		*(uint16_t*)pConfig->pData = data;
	}
	else
	{
		ClrWdt();
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 获取值，针对2字节[0,65535]
 * 适用于电压采样延时,传输延时，合闸时间，同步预制等待时间，
 * <p>
 * 以uint16_t形式存储，以2字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
void GetValueUint16(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 2)
	{
        uint16_t data = *(uint16_t*)pConfig->pData;
        pPoint->pData[0] = (uint8_t)(data & 0x00FF);
		ClrWdt();
        pPoint->pData[1] = (uint8_t)(data >> 8);
		pPoint->len = 2;
	}
	else
	{
		ClrWdt();
		pPoint->len = 0; //置为0，以示意错误
	}
}

/**
 * 设置值，针对1字节[0,255]
 *
 * <p>
 * 以uint8_t形式存储，以1字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
void SetValueUint8(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 1)
	{
		*(uint8_t*)pConfig->pData = pPoint->pData[0];
		ClrWdt();
        pPoint->len = 1;
	}
	else
	{
		pPoint->len = 0; //置为0，以示意错误
		ClrWdt();
	}
}

/**
 * 获取值，针对2字节[0,65535]
 * 适用于电压采样延时,传输延时，合闸时间，同步预制等待时间，
 * <p>
 * 以uint8_t形式存储，以1字节进行交换传输的数据
 *
 * @param   pPoint    指向数据数组
 * @param   pConfig   指向当前配置数据
 *
 * @brif 用于通讯交互数据使用
 */
void GetValueUint8(PointUint8* pPoint, ConfigData* pConfig)
{
	if (pPoint->len >= 1)
	{
        pPoint->pData[0] =  *(uint8_t*)pConfig->pData;
		ClrWdt();
        pPoint->len = 1;
	}
	else
	{
		pPoint->len = 0; //置为0，以示意错误
		ClrWdt();
	}
}
/**
 * 将累加和写入到EEPROM中
 * @param id
 * @param pPoint
 */
void WriteAccumulateSum(void)
{
    uint8_t i = 0;
    uint8_t id = 1;    
    uint16_t totalSum = 0;
    for(id = 1;id <= PARAMETER_LEN ;id++)
    {
        i = id - 1;
		ClrWdt();
        
        if(g_SetParameterCollect[i].ID == id)
        {		
            BufferData.len = g_SetParameterCollect[i].type >> 4;   //使用数据属性确定数据长度
			ClrWdt();	
            g_SetParameterCollect[i].fGetValue(&BufferData, g_SetParameterCollect + i);
            if(BufferData.len == 0)
            {
                continue;   //立即进行下一次循环
            }
            ParameterWriteByID(id , &BufferData);   //写EEPROM          
            
            ParameterReadByID(id, &BufferData);
			ClrWdt();
            for(uint8_t j = 0;j < BufferData.len;j++)
            {
				ClrWdt();
                totalSum += BufferData.pData[j];
            }
        }
    }
    WriteAccumulateSumEEPROM(&totalSum);  //将累加和写入到EEPROM中
    //此处可加校验，如果写到EEPROM中的和读取出来的不一致则报错
}
/**
 * 读取累加和并做对比
 * @param id
 * @param pPoint
 * @return 如果累加和相等则返回0，如果不相等在返回0xFF，即错误
 */
uint8_t AccumulateSumVerify(void)
{
    uint8_t i = 0;
    uint8_t id = 1;
    uint8_t len = 0;
    uint16_t readAddData = 0;
    uint16_t addData = 0;
    
    for(id = 1;id <= PARAMETER_LEN;id++)
    {
		ClrWdt();
        i = id - 1;
        if(g_SetParameterCollect[i].ID == id)   //初始化各个设置值
        {
            //为了获取数据长度
            len = g_SetParameterCollect[i].type >> 4;
            BufferData.len = len;
            ParameterReadByID(id, &BufferData);
			ClrWdt();
            
            //初始化各个变量
            g_SetParameterCollect[i].fSetValue(&BufferData, g_SetParameterCollect + i);
            for(i = 0;i < BufferData.len;i++)
            {
				ClrWdt();
                addData += BufferData.pData[i];
            }
        }
    }
	ClrWdt();
    ReadAccumulateSumEEPROM(&readAddData);
    if(readAddData == addData)
    {
		ClrWdt();
        return 0x00;
    }
    else
    {
		ClrWdt();
        //校验和不相等，需要报错，且需要重新设置所有的参数
        return 0xFF;
    }
}

/**
 * 
 * <p>Function name: [ParameterReadByID]</p>
 * <p>Discription: [读取EEPROM中的定值]</p>
 * @param id  配置号
 * @param data 指向数据的指针
 */
void ParameterReadByID(uint8_t id, PointUint8* pPoint)
{    
    uint16_t readData = 0;
    uint8_t i = 0;
    ClrWdt();
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    { 
        ClrWdt();
        ReadWord_EEPROM(address + i,&readData);
        pPoint->pData[i] = (readData & 0x00FF); //低位先读取
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            pPoint->pData[i + 1] = (readData >> 8);               
        }
    }
}

/**
 * 
 * <p>Function name: [ParameterWriteByID]</p>
 * <p>Discription: [将定值写入EEPROM]</p>
 * @param id  配置号
 * @param data 指向数据的指针
 */
void ParameterWriteByID(uint8_t id,PointUint8* pPoint)
{
    uint16_t data[2] = {0,0};
    uint8_t i = 0;
    ClrWdt();
    _prog_addressT address;
    address = (_prog_addressT)(id * EEPROM_OFFSET_ADDRESS) + EEPROM_STAR_ADDRESS;
    for(i = 0;i < pPoint->len;i += 2)
    {
        ClrWdt();
        if((pPoint->len % 2) == 0)  //对长度进行奇数校验
        {
            data[i] = pPoint->pData[i + 1];               
        }
        data[i] = data[i] << 8 | pPoint->pData[i];
        WriteWord_EEPROM(address + i,&data[i]);
    }
}
/**
 * 
 * <p>Function name: [WriteAccumulateSumEEPROM]</p>
 * <p>Discription: [写累加和到EEPROM中]</p>
 * @param writeData 所需写的数据
 */
void WriteAccumulateSumEEPROM(uint16_t* writeData)
{
    OFF_CAN_INT();  //不允许CAN中断
    ClrWdt();
    _prog_addressT address = ACCUMULATE_SUM_ADDRESS;
    WriteWord_EEPROM(address,writeData);    //写EEPROM时关闭CAN中断
    
    ON_CAN_INT();  //允许CAN中断
}

/**
 * 
 * <p>Function name: [ReadAccumulateSumEEPROM]</p>
 * <p>Discription: [读累加和]</p>
 * @param writeData 所需读取的数据
 */
void ReadAccumulateSumEEPROM(uint16_t* readData)
{
    _prog_addressT address = ACCUMULATE_SUM_ADDRESS;
    ClrWdt();
    ReadWord_EEPROM(address,readData);    
}

