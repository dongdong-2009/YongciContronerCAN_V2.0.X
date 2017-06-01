/** 
 * <p>application name： CRC16.h</p> 
 * <p>application describing： CRC16宏定义</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.05.20</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */
#ifndef CRC16_H
#define	CRC16_H

#ifdef	__cplusplus
extern "C" {
#endif

#include "../Driver/tydef.h"

uint16_t CRC16(uint8_t *puchMsg, uint8_t usDataLen);

#ifdef	__cplusplus
}
#endif

#endif	/* CRC16_H */

