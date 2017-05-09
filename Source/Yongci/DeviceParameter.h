/***********************************************
*Copyright(c) 2016,FreeGo
*保留所有权利
*文件名称:Main.c
*文件标识:
*创建日期： 2016年12月23日 
*摘要:

*当前版本:1.0
*作者: FreeGo
*取代版本:
*作者:
*完成时间:
************************************************************/
#ifndef XC_DEVICE_PARAMETER_H
#define	XC_DEVICE_PARAMETER_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "../Driver/tydef.h"

#define LOW_VOLTAGE_ADC  1.3



#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
   

//获取电压
void GetCapVoltage(void);
//获取电压状态
uint16 GetCapVolatageState(void);
#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

