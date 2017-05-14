/* 
 * File:   SwtichCondition.h
 * Author: LiDehai
 *
 * Created on 2015年7月13日, 下午7:45
 */

#ifndef SWTICHCONDITION_H
#define	SWTICHCONDITION_H
#include "../Driver/tydef.h"


#ifdef	__cplusplus
extern "C" {
#endif

#define SWITCH_ONE  0   //第一个动作的机构
#define SWITCH_TWO  1   //第二个动作的机构
#define SWITCH_THREE  2 //第三个动作的机构
    
void SwitchScan(void);
uint8 CheckIOState(void);
void DsplaySwitchState(void);   //显示相应的状态




#ifdef	__cplusplus
}
#endif

#endif	/* SWTICHCONDITION_H */

