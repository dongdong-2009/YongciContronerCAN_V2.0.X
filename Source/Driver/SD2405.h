/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_SD_H
#define	XC_SD_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include "tydef.h"
// TODO Insert appropriate #include <>

// TODO Insert C++ class definitions if appropriate

// TODO Insert declarations

// Comment a function and leverage automatic documentation with slash star star
/**
    <p><b>Function prototype:</b></p>
  
    <p><b>Summary:</b></p>

    <p><b>Description:</b></p>

    <p><b>Precondition:</b></p>

    <p><b>Parameters:</b></p>

    <p><b>Returns:</b></p>

    <p><b>Example:</b></p>
    <code>
 
    </code>

    <p><b>Remarks:</b></p>
 */
// TODO Insert declarations or function prototypes (right here) to leverage 
// live documentation

#ifdef	__cplusplus
extern "C" {
#endif /* __cplusplus */
    
    
//*********************************************************
/******SD2405配置位地址*******/
#define SD2405_ADDR 0x64    //从机地址0110 0100
#define WRTC1_ADDR  0x10    //中断允许地址
#define WRTC2_ADDR  0x0F    //写允许位地址
#define ARST_ADDR   0x11    //自动复位地址
#define FLAG_ADDR   0x14    //标志位地址
//*********************************************************

//*********************************************************
/******SD2405时间位地址*******/
#define SEC_ADDR        0x00
#define MIN_ADDR        0x01
#define HOUR_ADDR       0x02
#define WEEKDAY_ADDR    0x03
#define DAY_ADDR        0x04
#define MOUTH_ADDR      0x05
#define YEAR_ADDR       0x06
//*********************************************************
    
    
//*********************************************************
//标志位
    
#define FLAG    0x88
#define TWELVE_SYSTEM_ON    0x00
#define TWELVE_SYSTEM_OFF   0x80
//*********************************************************
    
typedef struct TagCalibrationTime
{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t day;
    uint8_t weekday;
    uint8_t mouth;
    uint16_t year;
}CheckTime;

void SD2405_Init(void);
void GetTime(void);
void SetTime(CheckTime* time , uint8_t twelveOn);
void I2CMasterRead(uint8_t* Rxdata , uint8_t readlen);

    // TODO If C++ is being used, regular C code needs function names to have C 
    // linkage so the functions can be used by the c code. 
extern CheckTime g_CheckTime;  //校验用时间

#ifdef	__cplusplus
}
#endif /* __cplusplus */

#endif	/* XC_HEADER_TEMPLATE_H */

