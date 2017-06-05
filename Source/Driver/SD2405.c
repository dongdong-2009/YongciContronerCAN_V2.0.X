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
  
    if(IIC_MasterReadByte(WRTC1_ADDR) != wrtc1)
    {
        IIC_WriteByte(WRTC1_ADDR , wrtc1); //WRTC1 置1，写允许，报警模式中断全部禁止
    }
    if(IIC_MasterReadByte(WRTC2_ADDR) != wrtc2)
    {
        IIC_WriteByte(WRTC2_ADDR , wrtc2);
    }
    if(IIC_MasterReadByte(ARST_ADDR) != arst)
    {
        IIC_WriteByte(ARST_ADDR , wrtc1); //WRTC1 置1，写允许，报警模式中断全部禁止
    }
    if(IIC_MasterReadByte(FLAG_ADDR) != flag)
    {
        IIC_WriteByte(FLAG_ADDR , flag); //写标志位
    }
    ClrWdt();
}

/**
 * 
 * <p>Function name: [turnTime]</p>
 * <p>Discription: [获取SD2405的真实时间]</p>
 * @param time 输入的时间
 * @return 换算后的真实时间
 */
uint8_t turnTime(uint8_t time)
{
    uint8_t ReadFigH = 0;
    uint8_t ReadFigL = 0;
    uint8_t retime = 0;
   
    ReadFigL = time & 0x0F;
    ReadFigH = (time >> 4) & 0x07;
    
    retime = ReadFigH *10 + ReadFigL;
   
   return(retime);
}

/**
 * 
 * <p>Function name: [GetTime]</p>
 * <p>Discription: [获取时间]</p>
 */
void GetTime(void)
{
    uint8_t Second_1,Minute_1,Hour_1,Week_1,Day_1,Month_1,Year_1;
    uint8_t fig = IIC_MasterReadByte(0x14);
    ClrWdt();
    if(fig == 0xAA)
    {
        ClrWdt();
        ClrWdt();
        ClrWdt();
    }
          
    g_CheckTime.sec = IIC_MasterReadByte(0x00);
    g_CheckTime.min = turnTime(IIC_MasterReadByte(0x01));
    g_CheckTime.hour = turnTime(IIC_MasterReadByte(0x02));
    g_CheckTime.weekday = turnTime(IIC_MasterReadByte(0x03));
    g_CheckTime.day = turnTime(IIC_MasterReadByte(0x04));
    g_CheckTime.mouth = turnTime(IIC_MasterReadByte(0x05));
	g_CheckTime.year = turnTime(IIC_MasterReadByte(0x06));
    ClrWdt();
    ClrWdt();
    ClrWdt();
}

         
         
         