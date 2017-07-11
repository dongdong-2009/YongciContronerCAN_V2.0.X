#ifndef _TYDEF_H_
#define _TYDEF_H_

#include <stdint.h> 

typedef float           float32_t;
typedef long double     float64_t;

#ifndef TRUE
#define TRUE   0xff

#endif

#ifndef FALSE
#define FALSE  0

typedef struct TagPointUint8
{
    uint8_t* pData; //数据指针
    uint8_t len; //指向数据长度
}PointUint8;

typedef union TagPointFloat
{
    float32_t fdata;
    uint8_t uData[4];
}PointFloat;


#endif

#endif
