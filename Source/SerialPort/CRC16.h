/* 
 * File:   CRC16.h
 * Author: ZFreeGo
 *
 * Created on 2014年9月17日, 上午9:56
 */

#ifndef CRC16_H
#define	CRC16_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../Driver/tydef.h"

uint16 CRC16(uint8 *puchMsg, uint8 usDataLen);

#ifdef	__cplusplus
}
#endif

#endif	/* CRC16_H */

