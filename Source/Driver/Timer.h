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
#ifndef _TIMER_H
#define	_TIMER_H

#ifdef	__cplusplus
extern "C" {
#endif

void InitTimer2(unsigned int ms);
void StartTimer2(void);
void StopTimer2(void);

void InitTimer3(void);
void StartTimer3(unsigned int us);
void ChangeTimerPeriod3(unsigned int us);
void StopTimer3(void);


void InitTimer4();
void ChangeTimerPeriod4(unsigned int ms);
void StartTimer4(void);
 void StopTimer4(void);
 uint16_t GetTimeUs(void);
#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */

