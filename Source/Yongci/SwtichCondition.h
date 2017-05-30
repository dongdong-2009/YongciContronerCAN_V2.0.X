/** 
 * <p>application name： SwtichCondition.h</p> 
 * <p>application describing： SwtichCondition宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */

#ifndef SWTICHCONDITION_H
#define	SWTICHCONDITION_H
#include "../Driver/tydef.h"


#ifdef	__cplusplus
extern "C" {
#endif

    
void SwitchScan(void);
uint8 CheckIOState(void);
void DsplaySwitchState(void);   //显示相应的状态



#ifdef	__cplusplus
}
#endif

#endif	/* SWTICHCONDITION_H */

