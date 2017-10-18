/**
 * @file InitTemp.c
 * @brief 操作DS18B20温度传感器
 * copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.
 * company： SOJO
 * @date 2017.05.20
 *
 * @author Zhangxiaomou
 * @version ver 1.0
 */

#include "../Header.h"


uint8_t DS18B20ReadBit(void);    //读一位
void DS18B20Reset(void);
uint8_t DS18B20ReadByte(void);
void DS18B20WriteByte(uint8_t dat);
void DS18B20StartTranfer(void);

uint8_t Flag = 0; //DS18B20 复位成功标志
uint16_t TemperateValue = 0;

/***************************************** 
*时序：初始化时序、读时序、写时序。
*所有时序都是将主机(单片机)作为主设备，单总线器件作为从设备。而每一次命令和数据的传输
*都是从主机主动启动写时序开始，如果要求单总线器件回送数据，在进行写命令后，主机需启动读
*时序完成数据接收。数据和命令的传输都是低位在先。  
*初始化时序：复位脉冲 存在脉冲
*        读；1 或 0时序
*        写；1 或 0时序
*只有存在脉冲信号是从18b20（从机）发出的，其它信号都是由主机发出的。
*存在脉冲：让主机（总线）知道从机（18b20）已经做好了准备。
******************************************/


/***********************************************************************
 * @fn DS18B20Reset()
 * @param uint8 ch 转换通道
 * 返回值;void
 * @brief 初始化：检测总线控制器发出的复位脉冲和ds18b20的任何通讯都要从初始化开始初始化
 * 序列包括一个由总线控制器发出的复位脉冲和跟在其后由从机发出的存在脉冲。
 * 初始化：复位脉冲+存在脉冲
 * 具体操作：
 *  总线控制器发出（TX）一个复位脉冲 （一个最少保持480μs 的低电平信号），然后释放总线，
 * 进入接收状态（RX）。单线总线由5K 上拉电阻拉到高电平。探测到I/O 引脚上的上升沿后
 * DS1820 等待15~60μs，然后发出存在脉冲（一个60~240μs 的低电平信号）。
 **************************************************************************/
void DS18B20Reset(void)
{
    ClrWdt();
    TERM0_DIR = 0;
    TERM0_OUT = 1;
    __delay_us(2);
    TERM0_OUT = 0;
    //当总线停留在低电平超过480us，总线上所以器件都将被复位，这里//延时约530us总线停留在低电平超过480μs，总线上的所有器件都将被复位。
    __delay_us(550);
    //产生复位脉冲后，微处理器释放总线,让总线处于空闲状态
    TERM0_OUT = 1;
    TERM0_DIR = 1;
    //释放总线后，以便从机18b20通过拉低总线来指示其是否在线,
    //等待时间15~60us， 所以延时44us
    __delay_us(45);
    TERM0_DIR = 1;
    if(TERM0_IN != 0) 
    {
       Flag=0;
    }
    else 
    {
       Flag=1; //复位成功标志
    }
    ClrWdt();
    __delay_us(2);
    //再次拉高总线，让总线处于空闲状态
    TERM0_OUT = 1;
    TERM0_DIR = 1;
}

/**************************************************
 * @fn DS18B20ReadBit()
 * @param uint8 ch 转换通道
 * @return 返回读取bit
 * @brief 读/写时间隙:DS1820 的数据读写是通过时间隙处理位和命令字来确认信息交换。
 ************************************************/
uint8_t DS18B20ReadBit(void)    //读一位
{
    uint8_t dat;
    //单片机（微处理器）将总线拉低
    TERM0_DIR = 0;
    TERM0_OUT = 0;
    //读时隙起始于微处理器将总线拉低至少1us
    __delay_us(2);

    ClrWdt();
    //拉低总线后接着释放总线，让从机18b20能够接管总线，输出有效数据
    TERM0_OUT = 1;
    TERM0_DIR = 1;

    //小延时一下，读取18b20上的数据 ,因为从ds18b20上输出的数据在读"时间隙"下降沿出现15us内有效
    __delay_us(3);

    //主机读从机18b20输出的数据，这些数据在读时隙的下降沿出现//15us内有效
    dat = TERM0_IN;
    //所有读"时间隙"必须60~120us，这里80us
    ClrWdt();
    __delay_us(80);
    return(dat);       //返回有效数据
}


/**************************************************
 * @fn DS18B20ReadByte()
 * @param  uint8 ch 转换通道
 * @return 返回读取字节
 * @brief   读取一个字节
 ****************************************************/
uint8_t DS18B20ReadByte(void) //读一字节
{
    ClrWdt();
	uint8_t value = 0,i = 0,j = 0;
	value=0;           
	for(i=0; i<8; i++)
    {
        ClrWdt();
        j = DS18B20ReadBit();   //先读的是低位
        value =(j<<7) | (value>>1);  		  //j只能等于0或1，把j向左移７位
    }
	return (value);        //返回一个字节的数据
}


/**************************************************
 * @fn DS18B20WriteByte()
 * @param  uint8 dat 将要写入的字节
 * @return void
 * @brief   向温度传感器写一个字节
****************************************************/
void DS18B20WriteByte(uint8_t dat) //写一个字节
{
    ClrWdt();
    uint8_t i = 0;
    uint8_t onebit = 0;
    for(i=1; i<=8; i++) 
    {
        ClrWdt();
        onebit= dat & 0x01;
        dat =dat>>1;

        if(onebit)      //写 1
        {
            ClrWdt();
            TERM0_DIR = 0;
            TERM0_OUT = 0;

           //看时序图，至少延时1us，才产生写"时间隙"  
            __delay_us(2);
            //写时间隙开始后的15μs内允许数据线拉到高电平
            TERM0_OUT = 1;
            //所有写时间隙必须最少持续60us
            __delay_us(70);
        }
        else         //写 0
        {
            TERM0_DIR = 0;
            TERM0_OUT = 0;
            ClrWdt();
            __delay_us(70);//主机要生成一个写0 时间隙，必须把数据线拉到低电平并保持至少60μs，这里70us
            TERM0_OUT = 1;
            TERM0_DIR = 1;
        }
    }
}

/*****************************************************************************
 * 主机（单片机）控制DS18B20完成温度转换要经过三个步骤：
 *每一次读写之前都要18B20进行复位操作，复位成功后发送一条ROM指令，最后发送RAM指令，
 * 这样才能对DS18b20进行预定的操作。
 *复位要求主CPU将数据线下拉500us，然后释放，当ds18B20收到信号后等待16~60us，
 * 后发出60~240us的存在低脉冲，主CPU收到此信号表示复位成功
****************************************************************************/ 

/**************************************************
 * @fn DS18B20StartTranfer()
 * @param  uint8 ch 转换通道
 * @return void
 * @brief   温度转换，先初始化，然后跳过ROM：跳过64位ROM地址，直接向ds18B20发温度转换命令。
****************************************************/
void DS18B20StartTranfer(void)		
{
    DS18B20Reset(); 		//初始化z
    
    ClrWdt();
    __delay_ms(2);              //约2ms
    ClrWdt();
    
    DS18B20WriteByte(0xcc);	 //忽略ROM指令，理由只有一个DS没必要对ROM进行操作
    DS18B20WriteByte(0x44);  //温度转化指令 
}

/**************************************************
 * @fn DS18B20GetTemperature()
 * @param uint8 ch 转换通道
 * @return void
 * @brief   获取温度值
****************************************************/
float32_t DS18B20GetTemperature(void)
{
    float32_t wendu;
    uint8_t a,b;
    DS18B20StartTranfer();
    DS18B20Reset(); 		//初始化
    if(!Flag)
    {
        DS18B20Reset(); 		//初始化
        if(!Flag)
        {
            DS18B20Reset(); 		//初始化
        }
    }
    ClrWdt();
    __delay_ms(2);              //约2ms
    ClrWdt();
   
    DS18B20WriteByte(0xcc);	 //忽略ROM指令，理由只有一个DS没必要对ROM进行操作
    DS18B20WriteByte(0x44);  //温度转化指令 
    
    __delay_ms(2);              //约2ms
    
    ClrWdt();
    DS18B20Reset();		     //复位
    ClrWdt();
    __delay_ms(2);              //约2ms
    ClrWdt();
    
    DS18B20WriteByte(0xcc);	   //跳过ROM指令
    DS18B20WriteByte(0xbe);	  //温度转化指令


    a = DS18B20ReadByte();		  //读取温度 第一个字节	这两个是连续的字节，且是bit0 与bit1,即它会在初始化前都是安顺序读取，
    b = DS18B20ReadByte();		 //读取第二个字节 高8位


    TemperateValue = b;		   
    TemperateValue <<= 8;			//temp 为unsigned int 型，它共有16位，b向右左8位，即把b放在了tempr的高八位，放在了低八位。
    TemperateValue = TemperateValue | a;		 //"|",与，有1便是“1”，temp 有十六位，高八位是b的位，这步的作用是把a赋给temp的低八位，所以现在temp 就是温度，但是没是这默认为正的温度没有负的温度
    ClrWdt();
    wendu = TemperateValue * 0.0625;     //获取温度
    
    b >>= 4;
    if(b == 0x0F)
    {
        ClrWdt();
        wendu = wendu - 4095.5;
    }
    return wendu;
}

