/** 
 * <p>application name： DeviceParameter.h</p> 
 * <p>application describing： DeviceParameter宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */
#ifndef XC_DEVICE_PARAMETER_H
#define	XC_DEVICE_PARAMETER_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "../Driver/tydef.h"


#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
   
#define CAP_LOW_STATE       0x01    //电容欠压状态
#define CAP_UPPER_STATE     0x03    //电容超压状态
#define CAP_NORMAL_STATE    0x02    //电容正常状态


//获取电压
void GetCapVoltage(void);
//获取电压状态
uint16_t GetCapVolatageState(void);
void UpdataCapVoltageState(void);
void ReadCapDropVoltage(void);

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

