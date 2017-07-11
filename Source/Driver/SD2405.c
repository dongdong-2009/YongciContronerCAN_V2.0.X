/** 
 * <p>application name： SD2405.c</p> 
 * <p>application describing： 时钟芯片操作</p> 
 * <p>copyright： Copyright (c) 2017 Beijing SOJO Electric CO., LTD.</p> 
 * <p>company： SOJO</p> 
 * <p>time： 2017.06.05</p> 
 * 
 * @updata:[日期YYYY-MM-DD] [更改人姓名][变更描述]
 * @author Zhangxiaomou 
 * @version ver 1.0
 */



#include "../Header.h"
#include "ImitationIIC.h"
#include "SD2405.h"

uint8_t turnTime(uint8_t time);
        
uint16_t g_Time[5] = {0};

CheckTime g_CheckTime;  //校验用时间


uint8_t BCD_to_Decimal(uint8_t time);
uint8_t Decimal_to_BCD(uint8_t data);
uint8_t turnTime(uint8_t time);


/**
 * 
 * <p>Function name: [SD2405_Init]</p>
 * <p>Discription: [SD2405芯片初始化]</p>
 */
void SD2405_Init(void)
{    
    uint8_t wrtc1 = 0x80;   //WRTC1 置1，写允许，报警模式中断全部禁止 
    uint8_t wrtc2 = 0x84;   //WRTC2,3置1
    uint8_t arst = 0x80;    //ARST 置1
    uint8_t flag = FLAG;
    uint8_t fig = 0;
//***********************************************************
//    IIC_WriteByte(0x10 , 0xA9);//WRTC1置1,写允许，报警模式中断选择，选择为频率中断 VBAT模式选择 频率中断允许0x10101001
//    IIC_WriteByte(0x0F , 0x84);//WRTC2,3置1
//    IIC_WriteByte(0x11 , 0x8f);//ARST置1 ts3/2/1/0设置频率为1HZ
//***********************************************************
    
    IIC_WriteByte(WRTC1_ADDR , wrtc1); //WRTC1 置1，写允许，报警模式中断全部禁止
    IIC_WriteByte(WRTC2_ADDR , wrtc2); 
    IIC_WriteByte(ARST_ADDR , arst); 
    IIC_WriteByte(FLAG_ADDR , flag); //写标志位
    ClrWdt();
    
    IIC_MasterReadByte(FLAG_ADDR , &fig);
    ClrWdt();
    if(fig != FLAG)
    {
        ClrWdt();
        IIC_WriteByte(FLAG_ADDR , FLAG); //写标志位
        return;
    }
    //**************************************
    //时钟芯片初始化时间 2017-06-06 17:10:20 星期二
    g_CheckTime.year = 2017;    //2017年
    g_CheckTime.mouth = 6;      //6月
    g_CheckTime.day = 6;        //6日
    g_CheckTime.hour = 17;      //17时
    g_CheckTime.min = 10;       //10min
    g_CheckTime.sec = 20;       //20s
    g_CheckTime.weekday = 2;    //星期2
    //**************************************
    
    ClrWdt();
}

/**
 * 
 * <p>Function name: [BCD_to_Decimal]</p>
 * <p>Discription: [获取真实时间]</p>
 * @param time  传入的时间参数
 * @return  返回真实的时间
 */
uint8_t BCD_to_Decimal(uint8_t time)
{
   uint8_t a = time;
   uint8_t b = 0;
   uint8_t c = 0;
   
   if(a >= 16)
   {
       while(a >= 16)
       {
           a -= 16;
           b += 10;
           c = a + b;
           time = c;
       }
   }
   
   return(time);
}

/**
 * 
 * <p>Function name: [Decimal_to_BCD]</p>
 * <p>Discription: [将十进制数转换为BCD码]</p>
 * @param data 十进制数据
 * @return 生成的BCD码
 */
uint8_t Decimal_to_BCD(uint8_t data)
{
    uint8_t a = data;
    uint8_t b = 0;
    uint8_t c = 0;
    
    if(a >= 10)
    {
        while(a >= 10)
        {
            a -= 10;
            b += 16;
            c = b + a;
            data = c;
        }
    }
    
    return data;
}

/**
 * 
 * <p>Function name: [turnTime]</p>
 * <p>Discription: [获取真实的时间]</p>
 * @param time 读取的时间
 * @return 真正的时间
 */
uint8_t turnTime(uint8_t time)
{
   uint8_t ReadFig = 0;
   uint8_t ReadFigH = 0;
   uint8_t ReadFigL = 0;
   uint8_t retime = 0;
   
   ReadFig = time;
   ReadFigL = ReadFig & 0x0F;
   ReadFigH = (ReadFig >> 4) & 0x07;
   retime = ReadFigH * 10 + ReadFigL;
   ClrWdt();
   return(retime);
}

/**
 * 
 * <p>Function name: [GetTime]</p>
 * <p>Discription: [获取时间]</p>
 */
void GetTime(void)
{
    uint8_t year = g_CheckTime.year - 2000;
    uint8_t data[8] = {0};
    OFF_COMMUNICATION_INT();
    
    IIC_MasterReadByte(SEC_ADDR , &g_CheckTime.sec);
    IIC_MasterReadByte(MIN_ADDR , &g_CheckTime.min);
    IIC_MasterReadByte(HOUR_ADDR , &g_CheckTime.hour);
    IIC_MasterReadByte(WEEKDAY_ADDR , &g_CheckTime.weekday);
    IIC_MasterReadByte(DAY_ADDR , &g_CheckTime.day);
    IIC_MasterReadByte(MOUTH_ADDR , &g_CheckTime.mouth);
    IIC_MasterReadByte(YEAR_ADDR , &year);
    ClrWdt();
    
    g_CheckTime.sec = turnTime(g_CheckTime.sec);
    g_CheckTime.min = turnTime(g_CheckTime.min);    
    g_CheckTime.hour = turnTime(g_CheckTime.hour & 0x7F);
    ClrWdt();
    g_CheckTime.weekday = turnTime(g_CheckTime.weekday);
    g_CheckTime.day = turnTime(g_CheckTime.day);
    g_CheckTime.mouth = turnTime(g_CheckTime.mouth);
	g_CheckTime.year =  turnTime(year) + 2000;
    I2CMasterRead(data,7);
    ON_COMMUNICATION_INT();
    ClrWdt();
    ClrWdt();
    ClrWdt();
}

/**
 * 
 * <p>Function name: [SetTime]</p>
 * <p>Discription: [初始化设置时间，默认使用24小时制]</p>
 * @param time  指向时间的指针
 * @param twelveOn  选择时间格式选择是24小时格式还是12小时格式
 */
void SetTime(CheckTime* time , uint8_t twelveOn)
{
    uint8_t sec = Decimal_to_BCD(time->sec);
    uint8_t min = Decimal_to_BCD(time->min);
    uint8_t hour = Decimal_to_BCD(time->hour) | twelveOn;
    uint8_t day = Decimal_to_BCD(time->day);
    uint8_t mouth = Decimal_to_BCD(time->mouth);
    uint8_t year = Decimal_to_BCD(time->year);
    ClrWdt();
    
    IIC_WriteByte(SEC_ADDR , sec);
    IIC_WriteByte(MIN_ADDR , min);
    ClrWdt();
    IIC_WriteByte(HOUR_ADDR , hour);
    IIC_WriteByte(DAY_ADDR , day);
    IIC_WriteByte(MOUTH_ADDR , mouth);
    IIC_WriteByte(YEAR_ADDR , year);
    ClrWdt();
}
         
         
         