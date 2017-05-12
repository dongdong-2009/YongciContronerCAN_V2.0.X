#ifndef _DEVICEIO_H_
#define _DEVICEIO_H_
#include "tydef.h"

#define ENABLE  1
#define DISABLE 0

#define TURN_ON  0x00
#define TURN_OFF 0xFF

/**
 * <p>Discription:[IGBT 控制---A]</p>
 */
//合闸桥臂--双IGBT
#define DRIVER_A_1   LATGbits.LATG3
#define DRIVER_A_1_DIR TRISGbits.TRISG3
#define DRIVER_D_1   LATBbits.LATB10
#define DRIVER_D_1_DIR  TRISBbits.TRISB10

//分闸桥臂--单IGBT
#define DRIVER_B_1   LATBbits.LATB13
#define DRIVER_B_1_DIR TRISBbits.TRISB13
#define DRIVER_C_1   LATFbits.LATF4
#define DRIVER_C_1_DIR TRISFbits.TRISF4

/**
 * <p>Discription:[A同步输出]</p>
 */
#define DECTA   LATGbits.LATG8
#define DECTA_DIR TRISGbits.TRISG8

#define HEZHA_A() {DRIVER_B_1  = DISABLE; DRIVER_C_1 = ENABLE;DRIVER_A_1 = ENABLE;DRIVER_D_1 = DISABLE;DECTA = ENABLE;}
#define FENZHA_A() {DRIVER_A_1 = DISABLE; DRIVER_D_1  = ENABLE; DRIVER_B_1 = ENABLE; DRIVER_C_1 = DISABLE;DECTA = ENABLE;}
#define RESET_CURRENT_A() {DRIVER_A_1 = DISABLE;DRIVER_D_1 = ENABLE; DRIVER_B_1 = DISABLE; DRIVER_C_1 = ENABLE;DECTA = DISABLE;}


/**
 * <p>Discription:[IGBT 控制---B]</p>
 */
//合闸桥臂--双IGBT
#define DRIVER_A_2   LATFbits.LATF6
#define DRIVER_A_2_DIR TRISFbits.TRISF6
#define DRIVER_D_2   LATBbits.LATB9
#define DRIVER_D_2_DIR  TRISBbits.TRISB9

//分闸桥臂--单IGBT
#define DRIVER_B_2   LATBbits.LATB12
#define DRIVER_B_2_DIR TRISBbits.TRISB12
#define DRIVER_C_2   LATBbits.LATB15
#define DRIVER_C_2_DIR TRISBbits.TRISB15

/**
 * <p>Discription:[B同步输出]</p>
 */
#define DECTB   LATBbits.LATB5
#define DECTB_DIR TRISBbits.TRISB5

#define HEZHA_B() {DRIVER_B_2  = DISABLE; DRIVER_C_2 = ENABLE;DRIVER_A_2 = ENABLE;DRIVER_D_2 = DISABLE;DECTB = ENABLE;}
#define FENZHA_B() {DRIVER_A_2 = DISABLE; DRIVER_D_2  = ENABLE; DRIVER_B_2 = ENABLE; DRIVER_C_2 = DISABLE;DECTB = ENABLE;}
#define RESET_CURRENT_B() {DRIVER_A_2 = DISABLE;DRIVER_D_2 = ENABLE; DRIVER_B_2 = DISABLE; DRIVER_C_2 = ENABLE;DECTB = DISABLE;}


/**
 * <p>Discription:[IGBT 控制---C]</p>
 */
//合闸桥臂--双IGBT
#define DRIVER_A_3   LATFbits.LATF5
#define DRIVER_A_3_DIR TRISFbits.TRISF5
#define DRIVER_D_3   LATBbits.LATB8
#define DRIVER_D_3_DIR  TRISBbits.TRISB8

//分闸桥臂--单IGBT
#define DRIVER_B_3   LATBbits.LATB11
#define DRIVER_B_3_DIR TRISBbits.TRISB11
#define DRIVER_C_3   LATBbits.LATB14
#define DRIVER_C_3_DIR TRISBbits.TRISB14

/**
 * <p>Discription:[C同步输出]</p>
 */
#define DECTC   LATBbits.LATB4
#define DECTC_DIR TRISBbits.TRISB4

#define HEZHA_C() {DRIVER_B_3  = DISABLE; DRIVER_C_3 = ENABLE;DRIVER_A_3 = ENABLE;DRIVER_D_3 = DISABLE;DECTC = ENABLE;}
#define FENZHA_C() {DRIVER_A_3 = DISABLE; DRIVER_D_3  = ENABLE; DRIVER_B_3 = ENABLE; DRIVER_C_3 = DISABLE;DECTC = ENABLE;}
#define RESET_CURRENT_C() {DRIVER_A_3 = DISABLE;DRIVER_D_3 = ENABLE; DRIVER_B_3 = DISABLE; DRIVER_C_3 = ENABLE;DECTC = DISABLE;}

//温度传感器
#define TERM0_IN    PORTGbits.RG9
#define TERM0_OUT   LATGbits.LATG9
#define TERM0_DIR   TRISGbits.TRISG9

#define ACTIVE_STATE  1 //激活

//合分位状态检测
//按钮输入标志位
#define HE_STATE    0x3E
#define FEN_STATE   0x3D
#define HE_WEI_STATE    0x3B
#define FEN_WEI_STATE   0x37
#define HE_YUAN_STATE   0x2F
#define FEN_YUAN_STATE  0x1F

//*********************************************
#define RELAY1_OPEN     0x4000 
#define RELAY2_OPEN     0x8000 
#define RELAY3_OPEN     0x2000
#define RELAY4_OPEN     0x0800
#define RELAY5_OPEN     0x1000  
#define RELAY6_OPEN     0x0400 
#define RELAY7_OPEN     0x0100
#define RELAY8_OPEN     0x0200
 

#define RELAY9_OPEN     0x0080
#define RELAY10_OPEN    0x0020 
#define RELAY11_OPEN    0x0040 
#define RELAY12_OPEN    0x0010
#define RELAY13_OPEN    0x0004 
#define RELAY14_OPEN    0x0008 
#define RELAY15_OPEN    0x0002
#define RELAY16_OPEN    0x0001   


#define RELAY1_CLOSE    0xBFFF
#define RELAY2_CLOSE    0x7FFF
#define RELAY3_CLOSE    0xDFFF
#define RELAY4_CLOSE    0xF7FF

#define RELAY5_CLOSE    0xEFFF  
#define RELAY6_CLOSE    0xFBFF 
#define RELAY7_CLOSE    0xFEFF
#define RELAY8_CLOSE    0xFDFF  

#define RELAY9_CLOSE    0xFF7F
#define RELAY10_CLOSE   0xFFDF 
#define RELAY11_CLOSE   0xFFBF 
#define RELAY12_CLOSE   0xFFEF

#define RELAY13_CLOSE   0xFFFB 
#define RELAY14_CLOSE   0xFFF7 
#define RELAY15_CLOSE   0xFFFD
#define RELAY16_CLOSE   0xFFFE  

/**
 * <p>Discription:[LED灯定义]</p>
 */
#define CAP1_RELAY   RELAY1_OPEN    //电容1储能指示继电器
#define CAP2_RELAY   RELAY2_OPEN    //电容2储能指示继电器
#define CAP3_RELAY   RELAY3_OPEN    //电容3储能指示继电器

#define ERROR1_RELAY    RELAY4_OPEN //故障指示继电器1
#define ERROR2_RELAY    RELAY5_OPEN //故障指示继电器2
#define ERROR3_RELAY    RELAY6_OPEN //故障指示继电器3

#define RUN_RELAY   RELAY7_OPEN     //运行指示继电器

#define Z_HEWEI_RELAY    RELAY8_OPEN    //总合位指示继电器
#define Z_FENWEI_RELAY   RELAY9_OPEN    //总分位指示继电器
    
#define HEWEI1_RELAY    RELAY10_OPEN    //机构1合位指示继电器
#define HEWEI3_RELAY    RELAY11_OPEN    //机构2合位指示继电器
#define HEWEI2_RELAY    RELAY12_OPEN    //机构3合位指示继电器

#define FENWEI1_RELAY   RELAY13_OPEN    //机构1分位指示继电器
#define FENWEI2_RELAY   RELAY14_OPEN    //机构2分位指示继电器
#define FENWEI3_RELAY   RELAY15_OPEN    //机构3分位指示继电器

//*********************************************
//set 0-led on
//set 1-led off
//*********************************************
#define LED1_ON 0xFFFE
#define LED2_ON 0xFFFD
#define LED3_ON 0xFFFB 
#define LED4_ON 0xFFF7
#define LED5_ON 0xFFEF  
#define LED6_ON 0xFFDF 
#define LED7_ON 0xFFBF
#define LED8_ON 0xFF7F  
 
#define LED9_ON  0xFDFF
#define LED10_ON 0xFBFF 
#define LED11_ON 0xF7FF 
#define LED12_ON 0xEFFF
#define LED13_ON 0xDFFF 
#define LED14_ON 0xBFFF 
#define LED15_ON 0x7FFF
 
#define LED1_OFF 0x0001
#define LED2_OFF 0x0002  
#define LED3_OFF 0x0004 
#define LED4_OFF 0x0008
#define LED5_OFF 0x0010  
#define LED6_OFF 0x0020 
#define LED7_OFF 0x0040
#define LED8_OFF 0x0080  
 
#define LED9_OFF  0x0200
#define LED10_OFF 0x0400 
#define LED11_OFF 0x0800 
#define LED12_OFF 0x1000
#define LED13_OFF 0x2000 
#define LED14_OFF 0x4000 
#define LED15_OFF 0x8000

/******************************
 * C1  G1  H1  F1  ZH   POWER *
 * C2  G2  H2  F2  ZF   LASER *
 * C3  G3  H3  F3  WORK LASER *
 *****************************/
/**
 * <p>Discription:[LED灯定义]</p>
 */
#define CAP1_LED LED3_ON    //电容1储能指示灯
#define CAP2_LED LED2_ON    //电容2储能指示灯
#define CAP3_LED LED1_ON    //电容3储能指示灯

#define ERROR1_LED  LED6_ON //故障指示灯1
#define ERROR2_LED  LED5_ON //故障指示灯1
#define ERROR3_LED  LED4_ON //故障指示灯1

#define HEWEI1_LED  LED9_ON //机构1合位指示灯
#define HEWEI2_LED  LED8_ON //机构2合位指示灯
#define HEWEI3_LED  LED7_ON //机构3合位指示灯

#define FENWEI1_LED  LED12_ON   //机构1分位指示灯
#define FENWEI2_LED  LED11_ON   //机构2分位指示灯
#define FENWEI3_LED  LED10_ON   //机构3分位指示灯

#define Z_HEWEI_LED  LED14_ON   //总的合位指示灯
#define Z_FENWEI_LED LED15_ON   //总的分位指示灯

#define RUN_LED LED13_ON    //工作指示灯

//Binary Input
//*************************
#define KR1_H  0x00000001   //Work or Debug
#define KR2_H  0x00000002   //Reserve
#define KR3_H  0x00000004   //FarAway or Local
#define KR4_H  0x00000008   //ON 1

#define KR5_H  0x00000010   //Live Shows
#define KR6_H  0x00000020   //OFF 1
#define KR7_H  0x00000040   //ON Order 1
#define KR8_H  0x00000080   //ON 2

#define KR9_H  0x00000100   //OFF Order 1
#define KR10_H 0x00000200   //OFF 2
#define KR11_H 0x00000400   //ON Order 2
#define KR12_H 0x00000800   //ON 3

#define KR13_H 0x00001000   //OFF Order 2
#define KR14_H 0x00002000   //OFF 3
#define KR15_H 0x00004000   //ON Order 3
#define KR16_H 0x00008000   //Reserve

#define KR17_H 0x00010000   //OFF Order 3
#define KR18_H 0x00020000   //Reserve
#define KR19_H 0x00040000   //ON Order
#define KR20_H 0x00080000   //Reserve

#define KR21_H 0x00100000   //OFF Order
#define KR22_H 0x00200000   //Reserve
#define KR23_H 0x00400000   //Reserve
#define KR24_H 0x00800000   //Reserve

//以上均为按位取反后的值
//***************************************
#define WORK_INPUT  KR1_H

#define YUAN_INPUT  KR3_H

#define YUAN_AND_WORK() (KR1_H | KR3_H)

#define DIANXIAN_INPUT KR5_H

//合闸、分闸1、2、3的输入
#define HZHA1_INPUT KR7_H
#define FZHA1_INPUT KR9_H

#define HZHA2_INPUT KR11_H
#define FZHA2_INPUT KR13_H

#define HZHA3_INPUT KR15_H
#define FZHA3_INPUT KR17_H

/**
 * 总的合闸、分闸输入
 */
#define Z_HEZHA_INPUT  KR19_H
#define Z_FENZHA_INPUT KR21_H

//合位、分位1、2、3的输入
#define HEWEI1_INPUT  KR4_H
#define FENWEI1_INPUT KR6_H

#define HEWEI2_INPUT  KR8_H
#define FENWEI2_INPUT KR10_H

#define HEWEI3_INPUT  KR12_H
#define FENWEI3_INPUT KR14_H

//******************************************

//光纤模块
//******************************************
#define RXD1_LASER_DIR  TRISDbits.TRISD9
#define RXD1_LASER  LATDbits.LATD9
#define TXD1_LASER_DIR  TRISGbits.TRISG12
#define TXD1_LASER  LATGbits.LATG12

#define RXD2_LASER_DIR  TRISDbits.TRISD10
#define RXD2_LASER  LATDbits.LATD10
#define TXD2_LASER_DIR  TRISGbits.TRISG13
#define TXD2_LASER  LATGbits.LATG13
//******************************************

#define HE_FEN_ERROR 0x00400001
//*****************************************

void InitDeviceIO(void);
unsigned long ReHC74165(void);

void InitInt1(void);
void TurnOnInt1(void);
void TurnOffInt1(void);
void UpdateIndicateState(uint16 relayPort,uint16 ledPort,uint8 state);

//向外部提供接口，以便在调试时可以找出错误原因，且可在调试时所有的错误都使用灯指出
void UpdateLEDIndicateState(uint16 port, uint8 state);  

#endif