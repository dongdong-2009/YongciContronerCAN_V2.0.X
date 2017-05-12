/* 
 * File:   Timer.h
 * Author: ZFreeGo
 *
 * Created on 2014年8月20日, 上午9:21
 */

#ifndef TIMER_H
#define	TIMER_H

#ifdef	__cplusplus
extern "C" {
#endif

#define MS_COUNT  62.5f //折算公式 1000 /(1/20 *256)    65536/79 = 800ms
#define MFOSC 16.0f //主时钟单位MHz

void SetTimer2(unsigned  int ms);
void StartTimer2(void);
void StopTimer2(void);

void Init_Timer3(void);
void StartTimer3(void);
void StopTimer3(void);


#ifdef	__cplusplus
}
#endif

#endif	/* TIMER_H */

