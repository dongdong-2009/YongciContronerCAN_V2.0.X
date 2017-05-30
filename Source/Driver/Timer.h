/** 
 * <p>application name： Timer.h</p> 
 * <p>application describing： Timer宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */
#ifndef XC_TIMER_H
#define	XC_TIMER_H

#ifdef	__cplusplus
extern "C" {
#endif

void SetTimer2(unsigned  int ms);
void StartTimer2(void);
void StopTimer2(void);

void Init_Timer3(void);
void StartTimer3(unsigned int us);
void ResetTimer3(void);

#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */

