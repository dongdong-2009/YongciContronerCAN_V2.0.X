#ifndef _TYDEF_H_
#define _TYDEF_H_

typedef char            int8_t;
typedef int             int16_t;
typedef long            int32_t;
typedef unsigned char   uint8_t;
typedef unsigned int    uint16_t;
typedef unsigned long   uint32_t;
typedef float           float32_t;
typedef long double     float64_t;

#define UINT8_MAX       255
#define UINT16_MAX      65535
#define UINT32_MAX      4294967295u

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


#endif

#endif
