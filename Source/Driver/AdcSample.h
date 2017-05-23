/** 
 * <p>application name： AdcSample.h</p> 
 * <p>application describing： AdcSample宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */

#ifndef ADCSAMPLE_H
#define	ADCSAMPLE_H

#include "tydef.h"

#ifdef	__cplusplus
extern "C" {
#endif

#define ADC_MODULUS 0.001220703125f

void AdcInit(void);
void SoftSampleOnce(void);
#define  NUM_CHS2SCAN 3 //扫描几路ADC就相应的赋值即可



#ifdef	__cplusplus
}
#endif

#endif	/* ADCSAMPLE_H */

